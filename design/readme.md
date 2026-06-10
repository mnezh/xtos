# XTOS Design Documentation

This directory contains the formal design specifications for XTOS.

---

## Documents

- `roadmap.md` — Development phases and milestones
- `architecture.md` — System structure and runtime model
- `apis.md` — XTOS C API specification (Event, File, Exec, UI)
- `readme.md` — Overview of design philosophy and document map

---

## Purpose

These documents define:

- System architecture boundaries
- API contracts between applications and runtime
- Evolution path from DOS-hosted system to standalone OS
- Constraints for implementation consistency

---

## Stability Rule

Design documents are considered **more authoritative than implementation details**.

Code must adapt to design, not the reverse.
