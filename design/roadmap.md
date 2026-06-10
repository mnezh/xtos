# XTOS Roadmap

XTOS development is divided into progressive phases, each increasing system maturity while preserving API compatibility.

---

## Phase 0 — SDK & Simulator (macOS)

### Goals
- Define XTOS API in C89
- Build SDL-based simulation runtime
- Implement font, cursor, and framebuffer rendering
- Create resource toolchain

### Deliverables
- XTOS SDK headers
- SDL runtime
- Basic UI toolkit (Form + Canvas)
- Reference applications

---

## Phase 1 — DOS-hosted runtime

### Goals
- Run XTOS under MS-DOS
- Implement CGA graphics mode system
- Event system + input handling
- Application loader

### Constraints
- Single active application
- No multitasking
- DOS used only as bootstrap layer

---

## Phase 2 — UI Framework maturity

### Goals
- Stable widget system
- System font architecture
- Cursor states and UI feedback
- Improved resource system

### Focus
- Developer consistency
- UI uniformity across apps

---

## Phase 3 — Display abstraction layer

### Goals
- Logical coordinate system
- Resolution independence (320/640 modes)
- Device-independent rendering pipeline

---

## Phase 4 — Productivity applications

### Applications
- Text editor (Document View system)
- Paint application (Canvas system)
- Control panel
- Basic utilities

---

## Phase 5 — Kernel independence

### Goals
- Remove DOS dependency
- Implement XTOS native kernel
- Boot directly from BIOS
- Own filesystem and drivers

---

## Long-term vision

Optional future evolution toward multi-window desktop system built on top of existing UI primitives (Forms, Canvas, Documents).
