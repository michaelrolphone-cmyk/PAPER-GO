# Build and Header Requirements

## Header placement contract (Arduino compatibility)

This project must compile in Arduino environments that only resolve quoted includes from `src/` and do not consistently add `include/` to the search path.

To prevent recurring `fatal error: <Header>.h: No such file or directory` failures:

1. Canonical headers remain in `include/`.
2. Every header in `include/*.h` **must** have a same-named forwarding header in `src/*.h`.
3. Each forwarding header must be exactly:

```cpp
#pragma once
#include "../include/<Header>.h"
```

4. When adding a new header to `include/`, add the matching `src/` forwarding header in the same commit.
5. `src/*.cpp` files should continue including headers by basename (for example, `#include "CommonControlsLogic.h"`).

## Pre-commit verification for header mirrors

Run this check before commit:

```bash
python - <<'PY'
import glob, os, sys
missing=[]
for h in glob.glob('include/*.h'):
    b=os.path.basename(h)
    s=os.path.join('src', b)
    if not os.path.exists(s):
        missing.append(b)
if missing:
    print('Missing src forwarding headers:')
    for m in missing: print(' -', m)
    sys.exit(1)
print('OK: all include/*.h headers have src/*.h forwarding headers')
PY
```
