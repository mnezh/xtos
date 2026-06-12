# XTOS INT 60h ABI

This document describes the Phase 1A implementation that exists now. It is not a future loader or residency design.

## Build Model

The current IA-16 toolchain does not support `-mcmodel=large`. XTOS therefore builds with:

```text
-mcmodel=medium
```

Medium model gives far code calls. Runtime service data crossings use explicit `__far` pointers in the parameter block. Applications and runtime are still linked into one DOS MZ executable in Phase 1A.

## Parameter Block

The public ABI type is `XtosPb` in `xtos/abi.h`:

```c
typedef struct XtosPb {
    u16 opcode;
    u16 result;
    u16 __far *int_in;
    u16 __far *int_out;
    void __far *addr_in;
    void __far *addr_out;
} XtosPb;
```

Conventions:

- `opcode` selects the runtime service.
- `result` is written by the runtime.
- `int_in` points to a service-specific array of `u16` inputs.
- `int_out` points to a service-specific array of `u16` outputs.
- `addr_in` and `addr_out` carry service-specific address arguments.
- Pointers are valid only during the service call. The runtime must not retain application pointers.

## INT 60h Calling Convention

The app-facing wrapper builds an `XtosPb` and calls `XtosInt60Call`.

`XtosInt60Call` passes the parameter block to the interrupt handler as:

```text
DS:DX = XtosPb far pointer
INT 60h
```

The INT 60h handler saves registers, pushes the far pointer as a C argument, calls `XtosInt60Dispatch`, restores registers, and returns with `iret`.

IA-16 far pointer arguments are passed as offset then segment on the stack. The hand-written assembly was checked against generated `gcc-ia16` assembly for `-mcmodel=medium`.

## Results

Current result codes:

```text
XTOS_RESULT_OK
XTOS_RESULT_UNKNOWN_OPCODE
XTOS_RESULT_BAD_PARAMETER
XTOS_RESULT_IO_ERROR
```

Public wrappers generally preserve existing public API signatures. For void APIs, failures are not surfaced yet. For APIs returning values, wrappers read from `int_out` or the parameter block result.

## Opcodes

Current opcodes:

```text
XTOS_OP_PING
XTOS_OP_LOG
XTOS_OP_SCREENSHOT_CGA
XTOS_OP_DISPLAY_SET_MODE
XTOS_OP_DISPLAY_CURRENT_MODE
XTOS_OP_DISPLAY_SET_PALETTE
XTOS_OP_DISPLAY_CURRENT_PALETTE
XTOS_OP_GET_EVENT
XTOS_OP_SYSTEM_PREFS_LOAD
XTOS_OP_SYSTEM_PREFS_SAVE
XTOS_OP_SYSTEM_PREFS_CURRENT
XTOS_OP_SYSTEM_PREFS_APPLY
XTOS_OP_CANVAS_CLEAR
XTOS_OP_CANVAS_CLEAR_RECT
XTOS_OP_CANVAS_RECT
XTOS_OP_CANVAS_DOTTED_RECT
XTOS_OP_CANVAS_FILL_RECT
XTOS_OP_CANVAS_TEXT
XTOS_OP_CANVAS_TEXT_WIDTH
XTOS_OP_CANVAS_PRESENT
```

## Routed APIs

These public APIs currently route through INT 60h:

```text
DisplaySetMode
DisplayCurrentMode
DisplaySetPalette
DisplayCurrentPalette
GetEvent
SystemPrefsLoad
SystemPrefsSave
SystemPrefsCurrent
SystemPrefsApply
XtosScreenshotCga
CanvasClear
CanvasClearRect
CanvasRect
CanvasDottedRect
CanvasFillRect
CanvasText
CanvasTextWidth
CanvasPresent
```

Form APIs remain direct by design. They involve app-owned structs, focus state, views, invalidation, and custom draw function pointers.

## Canvas Parameter Conventions

Canvas opcodes use `int_in` for scalar arguments:

- `CanvasClearRect`: `left, top, right, bottom`
- `CanvasRect`, `CanvasDottedRect`, `CanvasFillRect`: `left, top, right, bottom, role`
- `CanvasText`: `x, y, role`

`CanvasText` uses:

- `addr_in`: `const Font *`
- `addr_out`: `const char *`

`CanvasTextWidth` uses:

- `addr_in`: `const Font *`
- `addr_out`: `const char *`
- `int_out[0]`: width

These pointers are consumed immediately and are not stored.

## Debug Logging

With `XTOS_DEBUG`, logging writes `XTOS.LOG`.

Current useful log points:

- app start/shutdown
- selected app wrapper calls, excluding event polling and Canvas drawing
- display mode and palette changes
- screenshot dump
- unknown opcode

Canvas drawing is intentionally not logged by default because it is hot-path rendering.

## Screenshot Workflow

After each bundled app completes its first draw, the runtime captures one raw CGA dump:

```text
font.cga
control.cga
showcase.cga
```

The capture path is supplied by the app's `Application.first_draw_screenshot_path` field. Apps that leave it null do not auto-capture.

Press `Alt+S` while an XTOS app is running to request an additional runtime screenshot dump. The runtime writes:

```text
XTOS.CGA
```

The file is a raw 16 KiB dump of CGA memory at `B800:0000`.

Convert it on the host:

```sh
tools/cga2bmp.py XTOS.CGA XTOS.png --mode 320
tools/cga2bmp.py XTOS.CGA XTOS.bmp --mode 640
```

Equivalent Makefile helper:

```sh
make cga-png INPUT=XTOS.CGA OUTPUT=XTOS.png MODE=320
make cga-bmp INPUT=XTOS.CGA OUTPUT=XTOS.bmp MODE=640
```

Use `MODE=320` for `DISPLAY_MODE_LOW` and `MODE=640` for `DISPLAY_MODE_HIGH`.

## Manual DOSBox Validation

Build all apps:

```sh
make validate-build
```

Run each app:

```sh
make run-font
make run-control
make run-showcase
```

Check:

- screen renders correctly
- keyboard navigation works
- mouse movement/clicks work
- Control Panel preferences load and save
- no visible CGA corruption
- `XTOS.LOG` appears in debug builds
- first draw writes `<appname>.cga`
- `Alt+S` writes `XTOS.CGA`
- `tools/cga2bmp.py` converts the dump

## Current Non-Goals

The INT 60h ABI is not a callback table, loader, resource system, app switcher, or multitasking layer. It does not introduce runtime-to-app callbacks, stack switching, parent/child DOS callback chains, or custom executable formats.
