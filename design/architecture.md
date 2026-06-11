# XTOS Architecture

## System Overview

XTOS is a single-task, event-driven graphical runtime for DOS/CGA-era
hardware. Phase 0 is intentionally small: one active app, static allocation,
procedural APIs, direct CGA rendering, and no window manager.

Development uses a Docker-provided 8086/DOS compiler toolchain and DOSBox for
execution. Early runtime behavior is validated against the same DOS/CGA
assumptions used by the event, display, cursor, and drawing backends.

Current layers:

- Applications
- Public XTOS APIs (`app`, `event`, `display`, `system`, `ui`)
- Runtime services (event queue, UTF-8 text decoding, system preferences,
  forms, cursor)
- DOS/BIOS/CGA drivers

## Application Lifecycle

Applications provide an `Application` struct and call `AppRun()`.

`AppRun()` owns the common lifecycle:

1. Load system preferences from `XTOS.CFG`.
2. Apply display mode and palette.
3. Call app `Init()`.
4. Pump keyboard and mouse events.
5. Redraw only when invalidated.
6. Hide/show the system cursor around app redraw.
7. Call app `Shutdown()`.

Apps should usually not hardcode startup display setup.

## Input Model

Input is queue based. Runtime polling converts BIOS keyboard and INT 33h mouse
state into public `Event` values.

Supported Phase 0 events:

- `EVENT_KEYDOWN`
- `EVENT_MOUSE_MOVE`
- `EVENT_MOUSE_DOWN`
- `EVENT_MOUSE_UP`

The mouse cursor is runtime-owned, not application-owned. Cursor movement
restores the saved background and redraws the cursor without invalidating the
whole screen.

## Rendering Model

XTOS uses direct CGA rendering with invalidation:

- Controls invalidate their own bounds.
- The runtime tracks a simple dirty rectangle.
- `AppRun()` calls app draw only when dirty.
- Forms own static/full/partial redraw decisions.

There is no double buffering, shadow framebuffer, compositor, or window tree in
Phase 0.

## Text Model

XTOS system text encoding is UTF-8. Applications pass UTF-8 `const char *`
strings to public text APIs, and the runtime decodes them into `u16` Unicode
code points before bitmap glyph lookup. Generated fonts preserve BDF Unicode
encodings in static map tables, so Basic Latin, Central European Latin, and
Cyrillic can coexist when the selected font contains those glyphs.

This is deliberately not a full Unicode layout engine. Phase 0 does not support
shaping, bidirectional text, combining mark composition, CJK layout, font
fallback, locales, or code pages. Missing glyphs use the font's replacement
glyph when available and otherwise skip safely.

## UI Model

XTOS does not use a window-per-control architecture.

Phase 0 UI consists of:

- Canvas: immediate drawing primitives.
- Forms: fixed screens with static chrome and controls.
- Labels: non-interactive text.
- Lists: keyboard and mouse selection.
- Buttons: keyboard and mouse activation with action IDs.
- Views: Form-owned custom drawing rectangles for app-specific panes.

Forms manage focus across focusable controls (lists and buttons). Apps own
business logic and respond to returned action IDs. Views use a small draw
procedure but remain inside Form invalidation/redraw ownership.

## System Preferences

System preferences currently store display mode and palette. Persistence is a
minimal DOS-backed `XTOS.CFG`; it is not a registry, INI system, or general file
API.

## Future Compatibility

The architecture leaves room for later shells, document views, richer controls,
and application launching without requiring Phase 0 to introduce multitasking,
overlapping windows, or a compositor.
