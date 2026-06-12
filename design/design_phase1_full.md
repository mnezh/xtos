
# XTOS Phase 1 Design Specification
## Architecture Reset Edition

### Purpose

Phase 0 proved the viability of XTOS as a DOS-hosted graphical environment:
- CGA graphics
- UTF-8 bitmap fonts
- Mouse and keyboard support
- Forms, Lists, Labels, Buttons, Views
- Dirty rectangle rendering
- Control Panel, Font Viewer, Widget Showcase

Phase 1 focuses on separating the runtime from applications while preserving simplicity and debuggability.

---

# 1. Postmortem: What Failed

## Original Direction

The original design attempted:

XTOS.EXE <-> APP.EXE

with:
- DOS EXEC
- resident runtime
- API table discovery
- runtime→app callbacks
- app→runtime callbacks
- stack switching
- DS/SS switching
- far callback pointers

## Observed Failures

Symptoms:
- corrupted rendering
- corrupted memory
- callbacks not returning
- runtime instability

Root causes:
- IA-16 assumptions around DS/SS
- nested callback chains
- stack ownership ambiguity
- fragile far pointer ABI

Conclusion:

The problem was architectural, not graphical.

---

# 2. Explicitly Rejected Approaches

## No DOS Parent/Child Callback Architecture

Rejected:

XTOS.EXE ↔ APP.EXE

with arbitrary callback traffic.

## No Stack Switching

Rejected:
- supervisor stacks
- parked stacks
- stack trampolines
- stack restoration machinery

## No DS/SS Repair Logic

Rejected:
- compiler-dependent segment recovery
- DS restoration hacks
- SS synchronization tricks

## No Far Callback Tables

Rejected:

API structs full of far function pointers.

## No Custom Executable Format

Phase 1 keeps DOS MZ executables.

No:
- XAP
- ELF
- NE
- custom linker

## No Resource Compiler

Resources remain defined in C.

---

# 3. Architectural Principles

1. Explicit service boundaries.
2. Simplicity over cleverness.
3. Debuggability first.
4. DOS remains host, not framework.
5. Runtime services accessed through a stable ABI.
6. Runtime never depends on nested callback chains.

---

# 4. Memory Model

Preferred direction:

Large Model Runtime
Large Model Applications

Reason:

XTOS applications spend much of their time crossing runtime boundaries.

Small-model optimization no longer provides meaningful architectural benefits.

Current Phase 1A implementation note:

The active `gcc-ia16` toolchain does not support `-mcmodel=large`, so the
current build uses `-mcmodel=medium` plus explicit `__far` pointers in the
INT 60h parameter block. This preserves the Phase 1 direction without relying
on an unavailable compiler mode.

---

# 5. Runtime Layout

Runtime owns:
- graphics
- input
- fonts
- widgets
- preferences
- logging
- screenshots

Applications own:
- state
- strings
- business logic
- form definitions
- custom views

---

# 6. Runtime Service Boundary

XTOS services are exposed through:

INT 60h

Applications call stubs.

Stubs invoke INT 60h.

Runtime executes services.

---

# 7. GEM-Style Parameter Block

Conceptual model:

typedef struct XtosPb
{
    u16 opcode;
    u16 result;

    u16 __far *int_in;
    u16 __far *int_out;

    void __far *addr_in;
    void __far *addr_out;
} XtosPb;

The exact layout may evolve.

The principle does not:

Every runtime crossing is explicit.

---

# 8. Public API Direction

Applications continue to write:

DisplaySetMode(...)
CanvasText(...)
FormDraw(...)
SystemPrefsSave(...)

Implementation becomes:

Application
 -> Stub
 -> INT 60h
 -> Runtime

Transport details remain hidden.

---

# 9. Application Execution Model

Applications own their control flow.

Typical structure:

while (!done)
{
    GetEvent(&event);
    FormHandleEvent(&form, &event);
    FormDraw(&form);
}

Runtime provides services.

Runtime does not own application logic.

Current Phase 1B orchestration skeleton:

    xtos.com
      -> runtime.exe installs INT 60h and remains resident
      -> xtos.com launches launcher.exe by default
      -> launcher.exe requests a next app through ExecRequest(path)
      -> launcher.exe exits
      -> xtos.com reads the requested next app from the resident runtime
      -> xtos.com launches the requested app as a sibling DOS process
      -> the app exits normally through AppQuit()
      -> xtos.com relaunches launcher.exe
      -> launcher.exe exits without a next-app request for Exit XTOS
      -> app.exe verifies resident runtime with PING/STATUS
      -> app.exe temporarily installs its transitional in-process INT 60h
         handler for services not yet moved into the resident runtime
      -> Display, SystemPrefs, Canvas, Text, Font metadata, Event/Input, and
         Cursor calls are forwarded to the resident runtime
      -> app.exe restores the resident INT 60h vector on exit
      -> xtos.com asks the resident runtime to restore text mode
      -> xtos.com asks the resident runtime to uninstall INT 60h
      -> xtos.com exits

The resident runtime currently proves DOS TSR orchestration and owns Display
mode/palette, SystemPrefs, Canvas drawing, text rendering, built-in system
fonts, font metadata, event queue, keyboard polling, mouse polling, and cursor
ownership. Screenshot, Forms, widgets, invalidation, custom views, and the app
loop still remain transitional/app-local.

The launcher is a normal XTOS application. It does not DOS EXEC child
processes itself. It records the desired next executable in the resident
runtime with `ExecRequest(path)` and then exits. `xtos.com` remains the DOS
process owner and launches each requested application as a sibling process.
The supervisor loop is guarded by a launch counter so a broken launcher/app
cannot relaunch forever.

---

# 10. Fonts

System fonts should move into runtime ownership.

Applications acquire fonts through APIs.

Current Phase 1A API:

FontGet(FONT_SYSTEM);
FontGet(FONT_SMALL);
FontGet(FONT_LARGE);

Applications should stop embedding font data.

Current Phase 1A implementation status:

Built-in font data is linked into `runtime.exe`, not the bundled app binaries.
`FontGet()` still returns a `const Font *` for source compatibility, but apps
must treat that value as an opaque built-in font handle. App-local code must use
metadata helpers such as `FontName()`, `FontWidth()`, `FontHeight()`,
`FontGlyphCount()`, `FontCodepointAt()`, and `FontGlyphWidthAt()` instead of
dereferencing the `Font` internals.

Canvas text calls cross the resident boundary as `font_id` plus an app-owned
UTF-8 string pointer. The resident runtime copies/reads the string only during
the INT 60h call and renders text against resident-owned font data.

Font Viewer uses resident metadata services for font names, dimensions, glyph
counts, codepoints, and glyph widths, so it can keep the glyph table without
walking app-local copies of the built-in fonts.

---

# 10A. Input and Cursor

Current Phase 1A implementation status:

The resident runtime owns hardware-facing input:

- BIOS keyboard polling and XTOS key generation
- INT 33h mouse polling
- mouse movement/button transition state
- event queue storage, head/tail indices, and overflow behavior
- mouse cursor position, saved background, draw/erase, and visibility

Applications still own the main loop through `AppRun()`. The loop calls
resident input services and receives copied `Event` values, but the runtime
does not call back into the app and does not retain app event pointers.

Forms, Labels, Lists, Buttons, Views, invalidation, custom views, and
application state remain app-local. These objects contain focus state, app
strings, callback function pointers, and app-owned data.

Remaining migration risks:

- `Application` lifecycle callbacks
- `ViewDrawProc` custom view callbacks
- Form focus/action routing
- app-owned strings and control state
- invalidation regions tied to app-owned UI

Moving those pieces would reintroduce runtime-to-app callback pressure, so they
remain intentionally app-local.

---

# 11. Logging

Mandatory Phase 1 infrastructure.

Suggested macro:

XTOS_LOG(...)

Debug builds:
- enabled

Release builds:
- compiled out

Log file:

XTOS.LOG

Suggested prefixes:

[BOOT]
[RT]
[APP]
[INT60]

---

# 12. Screenshot Infrastructure

Phase 1 target:

XTOS.CGA

Raw dump:
- B800:0000
- 16384 bytes

Host-side converter:

tools/cga2bmp.py

Supports:
- 320x200x4
- 640x200x2

Outputs:
- BMP
- PNG

---

# 13. Phase 1A Goals

1. Runtime separation.
2. INT 60h service boundary.
3. Logging.
4. Screenshot support.
5. Existing apps remain operational.
6. Automated smoke validation.

Applications:
- Control Panel
- Font Viewer
- Widget Showcase

must continue to run.

---

# 14. Phase 1A Completion Status

Phase 1A is complete at the current checkpoint.

Resident runtime owns:

- Display and CGA mode/palette state
- SystemPrefs
- Canvas drawing and text rendering
- built-in font data and font metadata
- event queue storage
- keyboard polling
- mouse polling
- cursor save/restore/draw ownership
- PING, STATUS, SELFTEST, LOG, and RESTORE_TEXT_MODE

Applications own:

- `AppRun()` and lifecycle callback dispatch
- Forms, Labels, Lists, Buttons, and Views
- invalidation
- application state, strings, and business logic
- custom view callbacks

Current binary size snapshot:

```text
runtime.exe   64048
launcher.exe  43920
font.exe      44496
control.exe   44656
showcase.exe  44256
smoke.exe     44592
```

The app binaries no longer link built-in font data, Canvas/Text draw backends,
keyboard BIOS polling, mouse INT 33h polling, or the cursor backend. They keep
small public API stubs and app-local UI/lifecycle code.

---

# 15. Deferred to Phase 1B/2

Deferred intentionally:

- runtime-owned Forms/widgets
- launcher shell and application switching
- application loading/unloading policy
- resource compiler or resource package format
- custom executable formats
- app-local custom views policy beyond the current callback boundary

Runtime-owned Forms and custom views would require retained app state or
runtime-to-app callbacks, which Phase 1A explicitly avoids.

---

# 16. Phase 1B Launcher Architecture

Phase 1B starts with the smallest launcher architecture:

    supervisor
      -> resident runtime
      -> launcher app
      -> selected app
      -> launcher app

Implemented Phase 1B pieces:

- `xtos.com` with no argument runs `launcher.exe`.
- `xtos.com app.exe` still runs a single app directly for development.
- `launcher.exe` is a normal XTOS app using Forms/widgets.
- Public app API `ExecRequest(const char *path)` records a desired next
  executable in resident runtime state.
- `xtos.com` reads and clears the next-app request after each child exits.
- Apps are launched synchronously as sibling DOS processes; there is no nested
  launcher-to-app DOS EXEC and no multitasking.
- The supervisor exits after 64 launches as a guard against relaunch loops.
- On exit, `xtos.com` restores text mode and requests resident INT 60h
  uninstall.

Normal app exit contract:

- `AppQuit()` exits the current app and returns to its DOS caller.
- If the caller is `xtos.com` in launcher mode and the exiting app requested a
  next executable, `xtos.com` runs that executable.
- If a selected app exits without a next executable request, `xtos.com`
  relaunches `launcher.exe`.
- If `launcher.exe` exits without a next executable request, XTOS exits back to
  DOS after restoring text mode.

`ExecRequest(path)` contract:

- nonempty path shorter than 64 bytes records the pending next executable and
  returns nonzero
- `NULL`, empty, or too-long paths return 0
- failed requests do not alter the current pending request

Runtime uninstall contract:

- `XTOS_OP_UNINSTALL` restores text mode and verifies that INT 60h still points
  at the resident runtime handler.
- If the vector is still owned by XTOS, the previous INT 60h vector is restored
  and the call reports success.
- If another handler has hooked INT 60h, uninstall fails with a busy result and
  leaves the runtime resident.
- Resident memory release is not attempted yet; "uninstall ok" currently means
  "INT 60h vector restored safely".

Non-goals remain: multitasking, task switching, process manager, runtime-owned
forms, resources, dynamic discovery, executable metadata, and icons.

---

# 17. Success Criteria

Phase 1A succeeds when:

- runtime boundary exists
- logging works
- screenshots work
- apps run unchanged
- architecture remains debuggable

Phase 1A reached this state when:

- `xtos.com` exists as the tiny supervisor entry point
- `runtime.exe` exists as the resident INT 60h installer
- resident runtime detection works before apps install transitional handlers
- `make run-smoke` exits automatically
- `make smoke-png` generates `build/smoke.png` automatically
- `XTOS.LOG` contains deterministic `[TEST]` smoke markers
- existing apps still build and run through `xtos.com`
- resident runtime owns hardware/input/display/rendering services
- Forms/widgets remain direct because they own app state and callbacks

---

# 18. Failure Criteria

Phase 1A is considered off-track if implementation requires:

- stack-switching trampolines
- nested callback chains
- DS/SS repair logic
- far callback tables
- custom executable formats
- resource compilers

These paths have already been explored and rejected.
