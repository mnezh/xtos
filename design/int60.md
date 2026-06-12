# XTOS INT 60h ABI

This document describes the Phase 1A implementation that exists now. It is not a future loader or residency design.

## Build Model

The current IA-16 toolchain does not support `-mcmodel=large`. XTOS therefore builds with:

```text
-mcmodel=medium
```

Medium model gives far code calls. Runtime service data crossings use explicit `__far` pointers in the parameter block.

The current build now produces:

```text
build/xtos.com
build/runtime.exe
build/font.exe
build/control.exe
build/showcase.exe
build/smoke.exe
```

`xtos.com` is a tiny-model DOS COM supervisor. `runtime.exe` is the first resident INT 60h installer. The apps remain normal MZ executables and still carry transitional in-process runtime services for graphics.

Current execution flow:

```text
xtos.com app.exe
  -> run runtime.exe
  -> verify resident runtime through INT 60h PING
  -> run app.exe
  -> app verifies resident runtime
  -> app temporarily installs its local full INT 60h handler
  -> app restores the resident INT 60h vector on exit
  -> xtos.com calls resident RESTORE_TEXT_MODE
  -> xtos.com exits
```

This is an orchestration skeleton, not the final split runtime.

## Parameter Block

The current ABI version is:

```c
#define XTOS_ABI_VERSION 1
```

Runtime and app stubs include the same `xtos/abi.h` header in Phase 1A. There is no separate loader-time negotiation yet.

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

## Resident Dispatcher Invariants

These are hard ABI rules for Phase 1A resident services:

- The resident INT 60h handler must switch to a resident-owned SS:SP before
  calling resident C code.
- Resident C must never run on the app's SS. The IA-16 medium-model compiler
  may address globals through SS, so using the app stack segment can corrupt
  app memory even when DS is correct.
- The resident data segment must be captured from
  `__ia16_near_data_segment`, not from the incidental DS value live at install
  time.
- The resident handler must restore the caller's SS:SP, DS, ES, and preserved
  general registers before `iret`.
- The resident handler must return the dispatcher status in AX after restoring
  caller state.
- The resident dispatcher must preserve the parameter-block pointer and any
  live registers it needs after calling resident C services. In particular,
  BX cannot be assumed to survive a C call.
- Application pointers from the parameter block are valid only during the
  service call and must not be retained by resident code.
- The app-local transitional INT 60h handler intentionally does not
  stack-switch. It continues to run app-local services on the app stack because
  those services operate on app-owned state.

This stack switch is not a runtime-to-app callback mechanism. It is only the
resident interrupt entry discipline required before resident-owned C services
execute.

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
XTOS_OP_RESTORE_TEXT_MODE
XTOS_OP_RUNTIME_STATUS
XTOS_OP_SELFTEST
XTOS_OP_FONT_COUNT
XTOS_OP_FONT_NAME
XTOS_OP_FONT_WIDTH
XTOS_OP_FONT_HEIGHT
XTOS_OP_FONT_GLYPH_COUNT
XTOS_OP_FONT_CODEPOINT_AT
XTOS_OP_FONT_GLYPH_WIDTH_AT
XTOS_OP_PUMP_EVENTS
XTOS_OP_MOUSE_INIT
XTOS_OP_MOUSE_PRESENT
XTOS_OP_CURSOR_SHOW
XTOS_OP_CURSOR_HIDE
XTOS_OP_CURSOR_RESET
```

Resident `runtime.exe` currently implements only:

```text
XTOS_OP_PING
XTOS_OP_LOG
XTOS_OP_DISPLAY_SET_MODE
XTOS_OP_DISPLAY_CURRENT_MODE
XTOS_OP_DISPLAY_SET_PALETTE
XTOS_OP_DISPLAY_CURRENT_PALETTE
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
XTOS_OP_RESTORE_TEXT_MODE
XTOS_OP_RUNTIME_STATUS
XTOS_OP_SELFTEST
XTOS_OP_FONT_COUNT
XTOS_OP_FONT_NAME
XTOS_OP_FONT_WIDTH
XTOS_OP_FONT_HEIGHT
XTOS_OP_FONT_GLYPH_COUNT
XTOS_OP_FONT_CODEPOINT_AT
XTOS_OP_FONT_GLYPH_WIDTH_AT
XTOS_OP_GET_EVENT
XTOS_OP_PUMP_EVENTS
XTOS_OP_MOUSE_INIT
XTOS_OP_MOUSE_PRESENT
XTOS_OP_CURSOR_SHOW
XTOS_OP_CURSOR_HIDE
XTOS_OP_CURSOR_RESET
```

The app-local transitional handler still implements Screenshot. Canvas, Font,
Event/Input, and Cursor opcodes are forwarded to the saved resident vector so
hardware-facing services run on the resident stack/data path.

## Resident vs Transitional Opcodes

Phase 1A intentionally has a narrow resident runtime. Apps verify the resident
runtime first, then install an app-local transitional handler for graphics and
stateful UI services.

| Opcode | Current owner | Notes |
| --- | --- | --- |
| `XTOS_OP_PING` | resident | Returns runtime magic/version. |
| `XTOS_OP_RUNTIME_STATUS` | resident | Reports whether the resident runtime is ready. |
| `XTOS_OP_SELFTEST` | resident | Returns magic, ABI version, and ready status for automated validation. |
| `XTOS_OP_LOG` | resident | Resident-safe logging does not retain app pointers. |
| `XTOS_OP_RESTORE_TEXT_MODE` | resident | Restores DOS text mode on supervisor shutdown. |
| `XTOS_OP_DISPLAY_*` | resident | Resident owns CGA mode/palette state and updates resident Screen state for Canvas. |
| `XTOS_OP_SYSTEM_PREFS_*` | resident | Resident owns `XTOS.CFG`; app pointers are copied during the INT 60h call only. |
| `XTOS_OP_CANVAS_*` | resident | Canvas drawing and text rendering run resident-side. App strings are consumed during the call only. |
| `XTOS_OP_FONT_*` | resident | Built-in font metadata and glyph enumeration for app UI and Font Viewer. |
| `XTOS_OP_GET_EVENT` | resident | Pops from the resident event queue and copies one `Event` to app memory during the call. |
| `XTOS_OP_PUMP_EVENTS` | resident | Polls keyboard/mouse hardware and pushes resident queue events. |
| `XTOS_OP_MOUSE_*` | resident | Owns INT 33h interaction and mouse state. |
| `XTOS_OP_CURSOR_*` | resident | Owns cursor position, saved background, draw/erase, and visibility. |
| `XTOS_OP_SCREENSHOT_CGA` | transitional app-local | Captures CGA memory from the app-local runtime. |
| `FontGet` handles | app-local wrapper over resident fonts | Apps receive opaque handles/IDs and must not dereference built-in font internals. Built-in font data lives in `runtime.exe`. |

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

Widget APIs such as Label, List, Button, and View also remain direct in Phase 1A
for the same reason: they own or reference mutable app-side state and may
involve app-provided view draw functions. Their drawing calls now cross through
resident Canvas services.

## Input and Cursor Ownership

Hardware-facing input is resident-owned in the current Phase 1A checkpoint:

- the event queue storage, head/tail indices, and overflow behavior live in
  `runtime.exe`
- keyboard polling and XTOS key generation use resident BIOS INT 16h calls
- mouse polling and button transition tracking use resident INT 33h calls
- cursor position, saved background, draw/erase, and visibility live in
  `runtime.exe`

`AppRun()` remains app-local. It still owns lifecycle callback dispatch and the
main event loop shape, but its input/cursor calls are forwarding stubs. Apps
receive copied `Event` values only; resident code does not retain app event
pointers.

Mouse cursor drawing executes resident-side. App binaries keep only small cursor
forwarding stubs and no longer link keyboard BIOS polling, mouse INT 33h
polling, or low-level draw/text implementation objects.

## Font Ownership

The public application-facing API is:

```c
FontGet(FONT_SMALL)
FontGet(FONT_SYSTEM)
FontGet(FONT_LARGE)
```

The current Phase 1A implementation keeps the public `FontGet()` shape but
treats returned pointers as app-local opaque handles for built-in resident
fonts. Apps use helpers such as `FontWidth()`, `FontHeight()`,
`FontGlyphCount()`, `FontCodepointAt()`, `FontGlyphWidthAt()`, and
`FontName()` for metadata. Those helpers route through resident `XTOS_OP_FONT_*`
services.

The real `Font` descriptor still contains near nested pointers:

```c
const u8 *glyphs;
const GlyphMap *map;
const u8 *widths;
```

Those pointers are now dereferenced only by resident font/text code. Returning a
raw resident `Font *` to app-local rendering remains forbidden. App-local Forms
and widgets store the opaque handle only and use metadata services for layout.

The concrete built-in globals `Font4x6`, `Font5x7`, and `Font5x8` are
resident-owned implementation symbols linked into `runtime.exe`. New
application UI must not depend on them directly.

Font Viewer uses the resident metadata services to enumerate names, dimensions,
glyph counts, codepoints, and glyph widths. It still draws the glyph table, but
it no longer walks app-local built-in font internals.

## Canvas Parameter Conventions

Canvas opcodes use `int_in` for scalar arguments:

- `CanvasClearRect`: `left, top, right, bottom`
- `CanvasRect`, `CanvasDottedRect`, `CanvasFillRect`: `left, top, right, bottom, role`
- `CanvasText`: `x, y, font_id, role`

`CanvasText` uses:

- `addr_in`: app-owned `const char *`

`CanvasTextWidth` uses:

- `int_in[0]`: `font_id`
- `addr_in`: app-owned `const char *`
- `int_out[0]`: width

Text pointers are consumed immediately by resident code and are not stored.

## Debug Logging

With `XTOS_DEBUG`, logging writes `XTOS.LOG`.

Current useful log points:

- app start/shutdown
- selected app wrapper calls, excluding event polling and Canvas drawing
- resident and transitional mirror display mode and palette changes
- resident SystemPrefs load/save/apply
- screenshot dump
- unknown opcode
- deterministic smoke validation markers with the `[TEST]` prefix

Canvas drawing is intentionally not logged by default because it is hot-path rendering.

`xtos.com` logs boot/supervisor events with the `[BOOT]` prefix. `runtime.exe`
logs installation with `[RT]` when file logging is available. Resident INT 60h
logging uses resident-owned DOS helpers so resident services do not pull in
hosted stdio state or retain app-owned pointers.

`SystemPrefsCurrent()` keeps the public pointer-returning API, but the INT 60h
opcode copies resident prefs into an app-local static cache during the call.
Apps do not receive a resident-owned pointer.

## Screenshot Workflow

After each bundled app completes its first draw, the transitional app-local runtime captures one raw CGA dump:

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

When apps are run by the Makefile targets, DOSBox changes into `build/`, so bundled first-draw captures are expected under `build/`.

For bundled first-draw captures:

```sh
make font-png
make control-png
make showcase-png
```

These read `build/font.cga`, `build/control.cga`, and `build/showcase.cga` and write matching `.png` files in `build/`.

The noninteractive smoke app is the automated validation path:

```sh
make run-smoke
make smoke-png
```

`make run-smoke` runs `xtos.com smoke.exe` and exits without user input after
the first draw. `make smoke-png` reads `build/smoke.cga` and writes
`build/smoke.png`.

Smoke validation produces:

```text
build/smoke.cga
build/smoke.png
build/XTOS.LOG
```

Use `docs/screenshots/` for checked-in visual baselines when a screenshot is
promoted to a regression reference.

## Remaining Migration Risks

The following systems intentionally remain app-local because they contain
app-owned state, pointers, or callbacks:

- `Application` lifecycle callbacks: `Init`, `HandleEvent`, `Draw`, and
  `Shutdown`
- `Form` focus state, selected controls, and action routing
- `ViewDrawProc` custom view callbacks and their `void *data`
- Labels, Lists, Buttons, and Views, which store app strings and mutable UI
  state
- invalidation state, which tracks app-owned dirty UI regions

Moving these would require runtime-to-app callbacks or retained app pointers,
which remains out of scope for Phase 1A.

## Phase 1A Ownership Audit

Current resident-owned implementation modules:

- built-in font data: `runtime/fonts/font_4x6.c`,
  `runtime/fonts/font_5x7.c`, `runtime/fonts/font_5x8.c`
- Canvas/Text draw backend: `runtime/canvas.c`, `runtime/draw.c`,
  `runtime/draw_cga.c`, `runtime/draw_text.c`
- keyboard BIOS polling: `runtime/keyboard.s`
- mouse INT 33h polling: `runtime/mouse.s` plus resident `runtime/mouse.c`
- cursor backend: resident `runtime/cursor.c`
- event queue and event pumping: resident `runtime/event.c`

Current app binaries no longer link:

- built-in font data
- Canvas/Text draw backend
- keyboard BIOS polling
- mouse INT 33h polling
- cursor backend

Current app binaries intentionally still link:

- `runtime/app.c` for `AppRun()` and lifecycle callback dispatch
- `runtime/ui/form.c`, `label.c`, `list.c`, `button.c` for app-owned UI state
- `runtime/invalidation.c` for app-owned dirty-region tracking
- forwarding stubs for Display, SystemPrefs, Canvas, Fonts, Event/Input, and
  Cursor APIs
- app source files and custom view callbacks

Current binary size snapshot:

```text
runtime.exe   62816
font.exe      43904
control.exe   44080
showcase.exe  43648
smoke.exe     43984
```

`ia16-elf-nm` reports these generated DOS binaries as stripped/no-symbol files,
so the ownership audit is based on Makefile link inputs and the generated
binary size snapshot.

## Deferred to Phase 1B/2

The following are not Phase 1A work:

- runtime-owned Forms/widgets
- launcher shell and app switching
- resource compiler or packaged resources
- custom executable format
- runtime-owned custom views
- runtime-to-app callbacks

Custom views remain app-local because they contain `ViewDrawProc` function
pointers and app-owned `void *data`. Runtime-owned custom views would require a
new app callback policy and is deferred.

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
make run-smoke
```

The run targets execute:

```text
xtos.com font.exe
xtos.com control.exe
xtos.com showcase.exe
xtos.com smoke.exe
```

from inside `build/`.

Check:

- screen renders correctly
- keyboard navigation works
- mouse movement/clicks work
- Control Panel preferences load and save
- no visible CGA corruption
- `XTOS.LOG` appears in debug builds
- first draw writes `<appname>.cga`
- `make smoke-png` exits without user input and creates `build/smoke.png`
- `XTOS.LOG` contains `[TEST] smoke start`, `[TEST] selftest ok`, `[TEST] screenshot written`, and `[TEST] smoke complete`
- `Alt+S` writes `XTOS.CGA`
- `tools/cga2bmp.py` converts the dump
- running an app directly without `xtos.com` prints `This is a XTOS application and cannot run in DOS`

When automated DOSBox inspection is unavailable, this checklist is the manual validation source of truth.

Runtime unload is deferred. The resident runtime restores text mode but remains
resident until DOSBox/DOS exits.

## Current Non-Goals

The INT 60h ABI is not a callback table, loader, resource system, app switcher, or multitasking layer. It does not introduce runtime-to-app callbacks, stack switching, parent/child DOS callback chains, or custom executable formats.
