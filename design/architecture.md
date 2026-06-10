# XTOS Architecture

---

## System Overview

XTOS is a single-task, event-driven graphical runtime system designed for CGA-era hardware constraints.

It consists of four core subsystems:

Event System → Control flow  
File System  → Persistence  
Exec System  → Application lifecycle  
UI System    → Rendering + interaction  

---

## Execution Model

- Only one application is active at a time
- The application runs a main event loop
- The system delivers events via polling

```c
while (GetEvent(&e)) {
    HandleEvent(&e);
    Draw();
}
```

---

## UI Model

XTOS does NOT use a window-per-control architecture.

Instead, it provides three UI paradigms:

### 1. Forms
Structured UI screens for dialogs, settings, and control panels.

### 2. Canvas
Immediate-mode drawing surface for graphics applications.

### 3. Document Views
Scrollable data-driven views for text, spreadsheets, and terminals.

---

## File System Model

- Simple CRUD + directory listing API
- DOS-backed in early phases
- Path-based access (drive letters initially)

---

## Execution Model

- Single active process
- Application loaded via system API
- No multitasking in early phases

---

## Input Model

- Event queue system
- Mouse, keyboard, timer events unified
- No OS-level message routing trees

---

## Rendering Model

- Direct framebuffer access via system API
- No retained window system
- Full-screen rendering per application

---

## Future Compatibility

Architecture is designed to allow later introduction of:

- Multi-window compositor layer
- Process isolation
- Preemptive multitasking

without breaking existing application APIs.
