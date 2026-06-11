# XTOS Roadmap

XTOS development is divided into progressive phases, each increasing system maturity while preserving API compatibility.

---

## Phase 0 — DOS SDK & Runtime Foundation

### Goals
- Define XTOS API in C89
- Build with a containerized 8086/DOS toolchain
- Run and test apps under DOSBox
- Implement CGA display modes, input polling, and the event queue
- Implement font, cursor, framebuffer rendering, and invalidation
- Provide the first Forms + Canvas UI runtime
- Create the initial font/resource tooling

### Deliverables
- XTOS SDK headers
- DOS-hosted runtime
- Basic UI toolkit (Form + Canvas)
- Reference applications built as DOS executables
- Docker/DOSBox development workflow

---

## Phase 1 — Application Runtime

### Goals
- Add an application loader and execution API
- Define runtime packaging conventions
- Add file and resource access APIs
- Prototype a shell or launcher
- Strengthen app/runtime boundaries

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
