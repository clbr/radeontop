#!/usr/bin/env python3
"""Extract the amdgpu pci ids from the actual kernel source code.

Too bad the amdgpu_drv.c module in 6.1 kernels or a little earlier
does not compile easily without compiling the rest of the kernel.
Even when compiled, the signature is missing.

This Python script instead parses the source code and extracts the ids.
"""
import os.path
import re
import shutil
import sys
from typing import Generator, NoReturn

EXITING_ID_PATTERN: re.Pattern = re.compile(r'CHIPSET\(0x([A-Fa-f\d]{4}), ([A-Z\d_]+|0x[A-Fa-f\d]{4}), +[A-Z\d_]+\)')
PCI_ANY_ID_PATTERN: re.Pattern = re.compile("PCI_ANY_ID")
CHIP_PATTERN: re.Pattern = re.compile("CHIP")
CHIP_IP_DISCOVERY_PATTERN: re.Pattern = re.compile("CHIP_IP_DISCOVERY")
PCI_DEVICE_PATTERN: re.Pattern = re.compile("PCI_DEVICE")
NEW_ID_PATTERN: re.Pattern = re.compile(
    r"\t\{0x1002, 0x([A-Fa-f\d]{4}), PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_([A-Z\d_]+)(\|AMD_IS_[A-Z]+)*},")


def fail_format(message: str) -> NoReturn:
    print(message)
    print("A modification to this program is necessary to parse the new format")
    exit(2)


def parse_existing_ids(text: str) -> set[int]:
    result: set[int] = set()  # This discards capitalization. This set is matched against not outputted
    for line in text.split("\n"):
        if not line:
            continue
        match: re.Match | None = EXITING_ID_PATTERN.fullmatch(line)
        if not match:
            fail_format("r600_pci_ids.h's format changed: " + line)
        result.add(int(match.group(1), 16))
    return result


def extract_struct_pciidlist(text: str) -> str:
    header: str = "static const struct pci_device_id pciidlist[] = {"
    start: int = text.find(header)
    if start < 0:
        fail_format("The pciidlist struct may have moved")
    start += len(header)
    end: int = text.find("};", start)
    if end < 0:
        fail_format("Cannot find the end of the pciidlist struct")
    return text[start:end]


def parse_amdgpu_drv_c(text: str) -> Generator[tuple[str, str], None, None]:
    for line in text.split("\n"):
        if not line:
            continue
        if not PCI_ANY_ID_PATTERN.search(line):
            if CHIP_PATTERN.search(line) and not CHIP_IP_DISCOVERY_PATTERN.search(line):
                fail_format("Failed to assume that pci ids are on one line")
            continue  # Skip ifdef, comments, final {0, 0, 0}, and CHIP_IP_DISCOVERY
        if PCI_DEVICE_PATTERN.search(line):
            continue  # Skip CHIP_IP_DISCOVERY

        match: re.Match | None = NEW_ID_PATTERN.fullmatch(line)
        if not match:
            fail_format("Could not parse pciidlist line: " + line)
        yield match.group(1), match.group(2)


def write_output(new: list[tuple[str, str]]) -> None:
    result: str = "".join(f"CHIPSET(0x{x[0]}, 0x{x[0]}, {x[1]})\n" for x in new)
    shutil.copy("include/r600_pci_ids.h", "include/r600_pci_ids.h.tmp")
    with open("include/r600_pci_ids.h.tmp", "a") as f:
        f.write(result)
    os.rename("include/r600_pci_ids.h.tmp", "include/r600_pci_ids.h")


def main() -> None:
    if len(sys.argv) != 2:
        print("Usage: ./getamdgpuids.py /path/to/kernel/src")
        exit(1)
    src_path: str = sys.argv[1]

    text: str
    with open("include/r600_pci_ids.h") as f:
        text = f.read()
    existing_ids: set[int] = parse_existing_ids(text)

    if not os.path.isdir(src_path):
        print("Not a directory: " + src_path)
        exit(1)
    # No new radeon cards, so we can just look at amdgpu
    c_path: str = os.path.join(src_path, "drivers/gpu/drm/amd/amdgpu/amdgpu_drv.c")
    try:
        with open(c_path) as f:
            text: str = f.read()
    except FileNotFoundError:
        print("File is missing: " + c_path)
        exit(1)

    text = extract_struct_pciidlist(text)
    new_ids: list[tuple[str, str]] = list(filter(lambda x: int(x[0], 16) not in existing_ids, parse_amdgpu_drv_c(text)))
    write_output(new_ids)


if __name__ == "__main__":
    main()
