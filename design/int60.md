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
build/launcher.exe
```

`xtos.com` is a tiny-model DOS COM supervisor. `runtime.exe` is the first resident INT 60h installer. The apps remain normal MZ executables and still carry transitional in-process runtime services for graphics.

Current execution flow:

```text
xtos.com
  -> run runtime.exe
  -> verify resident runtime through INT 60h PING
  -> run launcher.exe
  -> launcher requests a next app with ExecRequest(path)
  -> launcher exits
  -> xtos.com reads and clears the resident next-app request
  -> run requested app as a sibling DOS process
  -> selected app exits
  -> run launcher.exe again
  -> launcher exits without a next-app request
  -> xtos.com calls resident RESTORE_TEXT_MODE
  -> xtos.com calls resident UNINSTALL
  -> xtos.com exits
```

The development flow is still supported:

```text
xtos.com app.exe
  -> run runtime.exe
  -> verify resident runtime through INT 60h PING
  -> run app.exe once
  -> app verifies resident runtime
  -> app temporarily installs its local full INT 60h handler
  -> app restores the resident INT 60h vector on exit
  -> xtos.com calls resident RESTORE_TEXT_MODE
  -> xtos.com calls resident UNINSTALL
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
XTOS_RESULT_UNSUPPORTED
XTOS_RESULT_BUSY
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
XTOS_OP_EXEC_REQUEST
XTOS_OP_EXEC_GET_NEXT
XTOS_OP_EXEC_CLEAR_NEXT
XTOS_OP_UNINSTALL
XTOS_OP_FORM_CREATE
XTOS_OP_FORM_DESTROY
XTOS_OP_FORM_ADD_LABEL
XTOS_OP_FORM_ADD_BUTTON
XTOS_OP_FORM_DRAW
XTOS_OP_FORM_DISPATCH
XTOS_OP_FORM_ADD_LIST
XTOS_OP_FORM_SET_LIST_ITEM
XTOS_OP_FORM_LIST_SELECTED
XTOS_OP_FORM_LIST_SET_SELECTED
XTOS_OP_FORM_SET_LABEL_TEXT
XTOS_OP_FORM_INVALIDATE
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
XTOS_OP_EXEC_REQUEST
XTOS_OP_EXEC_GET_NEXT
XTOS_OP_EXEC_CLEAR_NEXT
XTOS_OP_UNINSTALL
XTOS_OP_FORM_CREATE
XTOS_OP_FORM_DESTROY
XTOS_OP_FORM_ADD_LABEL
XTOS_OP_FORM_ADD_BUTTON
XTOS_OP_FORM_DRAW
XTOS_OP_FORM_DISPATCH
XTOS_OP_FORM_ADD_LIST
XTOS_OP_FORM_SET_LIST_ITEM
XTOS_OP_FORM_LIST_SELECTED
XTOS_OP_FORM_LIST_SET_SELECTED
XTOS_OP_FORM_SET_LABEL_TEXT
XTOS_OP_FORM_INVALIDATE
```

The app-local transitional handler still implements Screenshot. Canvas, Font,
Event/Input, Cursor, Exec, and runtime-owned Form opcodes are forwarded to the
saved resident vector so hardware-facing services, launcher handoff state, and
resident Form trees run on the resident stack/data path.

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
| `XTOS_OP_UNINSTALL` | resident | Restores text mode, validates INT 60h ownership, and restores the previous INT 60h vector. Resident memory is not freed yet. |
| `XTOS_OP_DISPLAY_*` | resident | Resident owns CGA mode/palette state and updates resident Screen state for Canvas. |
| `XTOS_OP_SYSTEM_PREFS_*` | resident | Resident owns `XTOS.CFG`; app pointers are copied during the INT 60h call only. |
| `XTOS_OP_CANVAS_*` | resident | Canvas drawing and text rendering run resident-side. App strings are consumed during the call only. |
| `XTOS_OP_FONT_*` | resident | Built-in font metadata and glyph enumeration for app UI and Font Viewer. |
| `XTOS_OP_GET_EVENT` | resident | Pops from the resident event queue and copies one `Event` to app memory during the call. |
| `XTOS_OP_PUMP_EVENTS` | resident | Polls keyboard/mouse hardware and pushes resident queue events. |
| `XTOS_OP_MOUSE_*` | resident | Owns INT 33h interaction and mouse state. |
| `XTOS_OP_CURSOR_*` | resident | Owns cursor position, saved background, draw/erase, and visibility. |
| `XTOS_OP_EXEC_REQUEST` | resident | Stores a single next executable path requested by the current app. |
| `XTOS_OP_EXEC_GET_NEXT` | resident | Copies the pending next executable path to the supervisor. |
| `XTOS_OP_EXEC_CLEAR_NEXT` | resident | Clears the pending next executable request. |
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
ExecRequest
XtosScreenshotCga
CanvasClear
CanvasClearRect
CanvasRect
CanvasDottedRect
CanvasFillRect
CanvasText
CanvasTextWidth
CanvasPresent
RtFormCreate
RtFormDestroy
RtFormAddLabel
RtFormAddButton
RtFormAddList
RtFormListSelected
RtFormListSetSelected
RtFormSetLabelText
RtFormInvalidate
RtFormDraw
RtFormDispatch
```

The legacy app-local `Form*`, `Label*`, `List*`, `Button*`, and `View*` APIs
remain direct for apps that still own live UI state. The new `RtForm*` APIs are
handle-based resident services for runtime-owned standard controls.

Legacy widget drawing calls now cross through resident Canvas services. Runtime
Form drawing/focus/hit-testing/button dispatch runs resident-side and returns
semantic `FormAction` values to the app.

## Runtime-Owned Forms

Phase 1B starts the Forms migration with resident-owned standard controls used
by `launcher.exe`, `showcase.exe`, and `control.exe`.

Public app-facing types live in `xtos/ui/runtime_form.h`:

```c
typedef u16 FormId;
typedef u16 ControlId;

typedef struct FormAction {
    u16 type;
    u16 form_id;
    u16 control_id;
    u16 value;
} FormAction;
```

Current action types:

```text
RT_FORM_ACTION_NONE
RT_FORM_ACTION_BUTTON
RT_FORM_ACTION_LIST_CHANGED
RT_FORM_ACTION_CLOSE
```

Current APIs:

```c
FormId RtFormCreate(const char *title, FontId font_id);
int RtFormDestroy(FormId form_id);
int RtFormAddLabel(FormId form_id, ControlId control_id,
                   u16 x, u16 y, FontId font_id,
                   enum CanvasColorRole color, const char *text);
int RtFormAddButton(FormId form_id, ControlId control_id,
                    u16 left, u16 top, u16 right, u16 bottom,
                    FontId font_id, const char *text);
int RtFormAddList(FormId form_id, ControlId control_id,
                  u16 x, u16 y, u16 width, FontId font_id,
                  const char * const *items, u8 count);
u8 RtFormListSelected(FormId form_id, ControlId control_id);
int RtFormListSetSelected(FormId form_id, ControlId control_id, u8 selected);
int RtFormSetLabelText(FormId form_id, ControlId control_id,
                       const char *text);
int RtFormInvalidate(FormId form_id);
int RtFormDraw(FormId form_id);
int RtFormDispatch(FormId form_id, const Event *event, FormAction *action);
```

Resident storage is fixed-size and uses no dynamic allocation:

```text
RT_FORM_MAX_FORMS       4
RT_FORM_MAX_LABELS      8 per form
RT_FORM_MAX_LISTS       4 per form
RT_FORM_MAX_LIST_ITEMS  8 per list
RT_FORM_MAX_BUTTONS     8 per form
RT_FORM_MAX_LABEL_TEXT  96 bytes per label
RT_FORM_MAX_TEXT_BYTES  512
```

App strings passed during construction are copied by the resident dispatcher
into resident scratch storage, then copied into the live Form text pool.
Runtime Forms do not retain app pointers. Label text uses fixed per-label
storage so status updates do not consume the shared construction text pool.

Runtime Forms support labels, buttons, and fixed static text lists. Lists have
resident-owned selected/focused state, keyboard up/down handling, mouse row
selection, and `RT_FORM_ACTION_LIST_CHANGED` semantic actions where
`control_id` is the list ID and `value` is the selected index.

`launcher.exe` uses runtime-owned labels/buttons. `showcase.exe` uses
runtime-owned labels/lists/buttons and updates its status label through
`RtFormSetLabelText()`. `control.exe` uses runtime-owned labels/lists/buttons,
keeps preference policy app-local, and calls `RtFormInvalidate()` after
preference apply/save because those operations may reset display state. Font
Viewer and Smoke still use legacy app-local Forms where needed.

List items are copied item-by-item by the app wrapper: `RtFormAddList()` creates
the resident list, then copies each item through a resident string-copy opcode.
This avoids retaining or walking app pointer arrays in resident code.

Resource descriptors, custom views, and runtime-to-app callbacks remain
deferred.

Current invalidation policy is intentionally cheap: `RtFormDispatch()` marks
the app dirty only when resident dispatch reports a visual state change or a
semantic action. Mouse movement with no hover/press behavior is a no-op and
does not trigger a redraw. Runtime Forms track static chrome dirty state plus
per-control dirty bits. `RtFormDraw()` draws chrome only when the form static
bit is dirty, then redraws only dirty labels, lists, and buttons; if nothing is
dirty it skips `CanvasPresent()`.

Dirty tracking is control-level plus a small per-list row mask, not a generic
rectangle engine. List selection redraws only the previous and new selected
rows; list focus changes redraw the frame. Dirty labels clear the old rendered
text area before drawing the new text.

`RtFormInvalidate()` marks static chrome, labels, buttons, list frames, and all
list rows dirty. It is intended for display reset boundaries, not ordinary
selection movement.

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

## Exec Request Conventions

`ExecRequest(path)` is the public app API for launcher-style handoff.

- `path == NULL`: returns 0, pending request unchanged.
- `path[0] == 0`: returns 0, pending request unchanged.
- path length >= 64 bytes: returns 0, pending request unchanged.
- valid nonempty path shorter than 64 bytes: copies the path into resident
  next-app storage and returns nonzero.

Apps do not DOS EXEC directly. The current app should call `AppQuit()` after a
successful request. `xtos.com` reads the pending request after the app exits and
launches the requested executable as a sibling DOS process.

## Debug Logging

With `XTOS_DEBUG`, logging writes `XTOS.LOG`.

Current useful log points:

- app start/shutdown
- selected app wrapper calls, excluding event polling and Canvas drawing
- resident and transitional mirror display mode and palette changes
- resident SystemPrefs load/save/apply
- exec request lifecycle
- supervisor launch/exit lifecycle
- uninstall request/result
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
- legacy `Form` focus state, selected controls, and action routing
- `ViewDrawProc` custom view callbacks and their `void *data`
- legacy Labels, Lists, Buttons, and Views, which store app strings and mutable
  UI state
- invalidation state, which tracks app-owned dirty UI regions

Runtime-owned launcher and Showcase controls no longer retain app pointers.
Moving custom views still requires a separate callback/data policy and remains
out of scope for this slice.

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
  in legacy apps
- `runtime/invalidation.c` for app-owned dirty-region tracking
- forwarding stubs for Display, SystemPrefs, Canvas, Fonts, Event/Input, and
  Cursor APIs, plus runtime-owned Form wrappers where used
- app source files and custom view callbacks

Current binary size snapshot:

```text
runtime.exe   70960
launcher.exe  45328
font.exe      46000
control.exe   46048
showcase.exe  45632
smoke.exe     46096
```

`ia16-elf-nm` reports these generated DOS binaries as stripped/no-symbol files,
so the ownership audit is based on Makefile link inputs and the generated
binary size snapshot.

## Deferred to Phase 1B/2

The following are not Phase 1A work:

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
make run-xtos
make run-font
make run-control
make run-showcase
make run-smoke
```

The run targets execute:

```text
xtos.com
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
- `xtos.com` with no argument starts `launcher.exe`
- launcher buttons request `control.exe`, `showcase.exe`, or `font.exe`
- after a selected app quits, `launcher.exe` is launched again
- launcher Exit XTOS returns to `xtos.com`, restores text mode, requests
  runtime uninstall, and exits
- `XTOS.LOG` contains `[BOOT] supervisor start`, app/launcher start/exit
  markers, `[RT] restore text mode`, `[RT] uninstall requested`, and
  `[RT] uninstall ok` or a failure reason
- no visible CGA corruption
- `XTOS.LOG` appears in debug builds
- first draw writes `<appname>.cga`
- `make smoke-png` exits without user input and creates `build/smoke.png`
- `XTOS.LOG` contains `[TEST] smoke start`, `[TEST] selftest ok`, `[TEST] screenshot written`, and `[TEST] smoke complete`
- `Alt+S` writes `XTOS.CGA`
- `tools/cga2bmp.py` converts the dump
- running an app directly without `xtos.com` prints `This is a XTOS application and cannot run in DOS`

When automated DOSBox inspection is unavailable, this checklist is the manual validation source of truth.

Runtime memory release is deferred. The current uninstall path safely restores
the previous INT 60h vector when XTOS still owns it. If INT 60h was hooked by
something else after XTOS, uninstall fails and leaves the resident runtime in
memory. Even on successful vector uninstall, the TSR memory block remains
allocated until DOSBox/DOS exits.

## Current Non-Goals

The INT 60h ABI is not a callback table, loader, resource system, app switcher, or multitasking layer. It does not introduce runtime-to-app callbacks, stack switching, parent/child DOS callback chains, or custom executable formats.
