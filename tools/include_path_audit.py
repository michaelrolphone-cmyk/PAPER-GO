#!/usr/bin/env python3
import re
from pathlib import Path
import sys

inc_dir = Path('include')
src_dir = Path('src')

if not inc_dir.exists() or not src_dir.exists():
    print('ERROR: expected include/ and src/ directories at repo root')
    sys.exit(2)

canonical_headers = {p.name for p in inc_dir.glob('*.h')}
missing_forwarders = []
invalid_forwarders = []
for header in sorted(canonical_headers):
    fwd = src_dir / header
    if not fwd.exists():
        missing_forwarders.append(header)
        continue
    txt = fwd.read_text(encoding='utf-8', errors='ignore').strip()
    expected = f'#pragma once\n#include "../include/{header}"'
    if txt != expected:
        invalid_forwarders.append(header)

include_re = re.compile(r'^\s*#\s*include\s*"([^"]+)"')
missing_resolvable = []
for cpp in sorted(src_dir.glob('*.cpp')):
    text = cpp.read_text(encoding='utf-8', errors='ignore').splitlines()
    for line_no, line in enumerate(text, start=1):
        m = include_re.match(line)
        if not m:
            continue
        name = m.group(1)
        if '/' in name:
            continue
        if name in canonical_headers and not (src_dir / name).exists():
            missing_resolvable.append((cpp.as_posix(), line_no, name))

if missing_forwarders:
    print('Missing src forwarding headers:')
    for h in missing_forwarders:
        print(' -', h)

if invalid_forwarders:
    print('Invalid src forwarding header contents (must match exact two-line template):')
    for h in invalid_forwarders:
        print(' -', h)

if missing_resolvable:
    print('Non-Arduino-resolvable includes from src/*.cpp:')
    for file, line, h in missing_resolvable:
        print(f' - {file}:{line}: #include "{h}" (include/{h} exists but src/{h} missing)')

if missing_forwarders or invalid_forwarders or missing_resolvable:
    sys.exit(1)

print('OK: include-path audit passed (forwarders present, exact template, and src includes Arduino-resolvable).')
