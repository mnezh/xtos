# XTOS API Specification (Phase 0)

XTOS Phase 0 exposes small procedural APIs suitable for DOS, CGA, 8086,
Turbo C style code, and static allocation. Runtime services hide BIOS,
keyboard, mouse, display, and persistence details.

## Application

```c
typedef struct Application {
    void (*Init)(void);
    void (*HandleEvent)(Event *event);
    void (*Draw)(void);
    void (*Shutdown)(void);
} Application;

int AppRun(Application *app);
void AppQuit(void);
```

`AppRun()` loads and applies system preferences, calls `Init()`, pumps
keyboard/mouse events, redraws only when invalidated, and restores text mode
through application shutdown.

## Events

```c
#define XTOS_KEY_ENTER  0x000d
#define XTOS_KEY_ESCAPE 0x001b
#define XTOS_KEY_TAB    0x0009
#define XTOS_KEY_ALT_Q  0x1000
#define XTOS_KEY_UP     0x4800
#define XTOS_KEY_DOWN   0x5000

#define XTOS_MOUSE_LEFT  0x0001
#define XTOS_MOUSE_RIGHT 0x0002

enum EventType {
    EVENT_NONE,
    EVENT_KEYDOWN,
    EVENT_KEYUP,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_DOWN,
    EVENT_MOUSE_UP,
    EVENT_TICK,
    EVENT_QUIT
};

#define EVENT_MOUSEMOVE EVENT_MOUSE_MOVE
#define EVENT_MOUSEDOWN EVENT_MOUSE_DOWN
#define EVENT_MOUSEUP   EVENT_MOUSE_UP

typedef struct Event {
    enum EventType type;
    u16 key;
    u16 x;
    u16 y;
    u16 buttons;
} Event;

int GetEvent(Event *event);
```

Keyboard events use `key`. DOS extended key chords are exposed as BIOS scan
codes in the high byte; `XTOS_KEY_ALT_Q` is the standard app quit shortcut.
Mouse events use logical screen coordinates and `buttons` (`XTOS_MOUSE_LEFT`,
`XTOS_MOUSE_RIGHT`).

## Display

```c
enum DisplayMode {
    DISPLAY_MODE_LOW,
    DISPLAY_MODE_HIGH
};

enum DisplayPalette {
    DISPLAY_PALETTE_BLACK_ON_WHITE,
    DISPLAY_PALETTE_WHITE_ON_BLACK,
    DISPLAY_PALETTE_WHITE_ON_BLUE,
    DISPLAY_PALETTE_BROWN_ON_BLUE,
    DISPLAY_PALETTE_BLUE_ON_WHITE,
    DISPLAY_PALETTE_COUNT
};

typedef struct DisplayModeInfo {
    enum DisplayMode id;
    const char *name;
    u16 width;
    u16 height;
    u8 colors;
} DisplayModeInfo;

typedef struct DisplayPaletteInfo {
    enum DisplayPalette id;
    const char *name;
} DisplayPaletteInfo;

void DisplaySetMode(enum DisplayMode mode);
enum DisplayMode DisplayCurrentMode(void);

void DisplaySetPalette(enum DisplayPalette palette);
enum DisplayPalette DisplayCurrentPalette(void);

u16 DisplayWidth(void);
u16 DisplayHeight(void);

u8 DisplayModeCount(void);
const DisplayModeInfo *DisplayModeInfoAt(u8 index);
u8 DisplayPaletteCount(void);
const DisplayPaletteInfo *DisplayPaletteInfoAt(u8 index);
void DisplayShutdown(void);
```

Ordinary apps should not hardcode startup display settings. `AppRun()` applies
system preferences before app init. Control Panel and diagnostics may still
call display setters.

## System Preferences

```c
typedef struct SystemPrefs {
    enum DisplayMode mode;
    enum DisplayPalette palette;
} SystemPrefs;

int SystemPrefsLoad(SystemPrefs *prefs);
int SystemPrefsSave(const SystemPrefs *prefs);
const SystemPrefs *SystemPrefsCurrent(void);
void SystemPrefsApply(const SystemPrefs *prefs);
```

Phase 0 stores preferences in `XTOS.CFG` and currently persists display mode
and palette only. Missing or invalid settings default to 320x200
`DISPLAY_PALETTE_BLUE_ON_WHITE`.

## Canvas

```c
enum CanvasColorRole {
    CANVAS_PRIMARY_BACKGROUND,
    CANVAS_PRIMARY_FOREGROUND,
    CANVAS_EXTRA_1_ON_BACKGROUND,
    CANVAS_EXTRA_2_ON_BACKGROUND,
    CANVAS_INVERTED_BACKGROUND,
    CANVAS_INVERTED_FOREGROUND,
    CANVAS_INVERTED_EXTRA_1,
    CANVAS_INVERTED_EXTRA_2
};

void CanvasClear(void);
void CanvasClearRect(u16 left, u16 top, u16 right, u16 bottom);
void CanvasRect(u16 left, u16 top, u16 right, u16 bottom,
                enum CanvasColorRole role);
void CanvasDottedRect(u16 left, u16 top, u16 right, u16 bottom,
                      enum CanvasColorRole role);
void CanvasFillRect(u16 left, u16 top, u16 right, u16 bottom,
                    enum CanvasColorRole role);
void CanvasText(u16 x, u16 y, const Font *font,
                enum CanvasColorRole role, const char *text);
u16 CanvasTextWidth(const Font *font, const char *text);
void CanvasPresent(void);
```

Canvas is immediate-mode drawing over the active CGA backend. Public text
strings are UTF-8 `const char *`; the renderer decodes them to `u16` Unicode
code points and draws matching bitmap font glyphs.

## Text And Fonts

```c
#define XTOS_TEXT_REPLACEMENT 0xfffd

int TextNextCodepoint(const char **cursor, u16 *codepoint);

#define FONT_MONOSPACE 1
#define FONT_PROPORTIONAL 2

typedef struct GlyphMap {
    u16 codepoint;
    u16 glyph_index;
} GlyphMap;

typedef struct Font {
    const char *name;
    u8 width;
    u8 height;
    u8 ascent;
    u8 descent;
    u16 glyph_count;
    u16 replacement_glyph;
    u8 flags;
    const u8 *glyphs;
    const GlyphMap *map;
    const u8 *widths;
} Font;

u16 FontGlyphCount(const Font *font);
u16 FontCodepointAt(const Font *font, u16 index);
u8 FontGlyphWidthAt(const Font *font, u16 index);
u16 FontTextWidth(const Font *font, const char *text);

extern const Font Font4x6;
extern const Font Font5x7;
extern const Font Font5x8;
```

XTOS system text encoding is UTF-8. Application string literals, config files,
and public text APIs use UTF-8. Fonts map Unicode code points to packed bitmap
glyphs through static `GlyphMap` tables. Missing glyphs render through the
font's replacement glyph when available, then skip safely.

Current text limitations are intentional: no shaping, no bidirectional layout,
no combining mark composition, no CJK support, no font fallback, and no code
page switching.

## Forms And Controls

Forms are fixed, static-allocation containers for labels, lists, buttons, and
custom rectangular views. They own redraw policy: static chrome is drawn once,
controls invalidate their own bounds, and `FormDraw()` performs full or
dirty-rectangle redraws.

```c
#define FORM_MAX_LABELS 8
#define FORM_MAX_LISTS 4
#define FORM_MAX_BUTTONS 4
#define FORM_MAX_VIEWS 4

#define FORM_ACTION_NONE 0
#define FORM_ACTION_CLOSE 255

#define FORM_FOCUS_NONE 0
#define FORM_FOCUS_LIST 1
#define FORM_FOCUS_BUTTON 2

typedef struct View View;
typedef void (*ViewDrawProc)(View *view, const Rect *dirty, void *data);

typedef struct Label {
    u16 x;
    u16 y;
    const Font *font;
    const char *text;
    enum CanvasColorRole color;
    u16 dirty_width;
} Label;

typedef struct List {
    u16 x;
    u16 y;
    u16 width;
    const Font *font;
    const char * const *items;
    u8 count;
    u8 selected;
    u8 row_height;
    u8 focused;
} List;

typedef struct Button {
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
    const Font *font;
    const char *text;
    u8 id;
    u8 focused;
    u8 pressed;
} Button;

struct View {
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
    ViewDrawProc draw;
    void *data;
};

typedef struct Form {
    const char *title;
    const Font *font;
    Label *labels[FORM_MAX_LABELS];
    List *lists[FORM_MAX_LISTS];
    Button *buttons[FORM_MAX_BUTTONS];
    View *views[FORM_MAX_VIEWS];
    u8 label_count;
    u8 list_count;
    u8 button_count;
    u8 view_count;
    List *focused_list;
    Button *focused_button;
    Button *pressed_button;
    u8 focused_type;
    u8 static_drawn;
} Form;

void FormInit(Form *form, const char *title, const Font *font);
void FormAddLabel(Form *form, Label *label);
void FormAddList(Form *form, List *list);
void FormAddButton(Form *form, Button *button);
void FormAddView(Form *form, View *view);
void FormDraw(Form *form);
void FormDrawStatic(Form *form);
void FormDrawControls(Form *form);
void FormDrawControlsInRect(Form *form, const Rect *rect);
int FormHandleEvent(Form *form, Event *event);
void FormInvalidate(Form *form);
void FormInvalidateAll(Form *form);
void FormSetFocusedList(Form *form, List *list);
void FormSetFocusedButton(Form *form, Button *button);
void FormFocusNextList(Form *form);
void FormFocusNextControl(Form *form);
List *FormFocusedList(Form *form);
Button *FormFocusedButton(Form *form);

void LabelInit(Label *label, u16 x, u16 y, const Font *font,
               const char *text, enum CanvasColorRole color);
void LabelSetText(Label *label, const char *text);
void LabelDraw(Label *label);
void LabelDrawInRect(Label *label, const Rect *rect);
void LabelInvalidate(Label *label);

void ListInit(List *list, u16 x, u16 y, u16 width, const Font *font,
              const char * const *items, u8 count);
void ListDraw(List *list);
void ListDrawRow(List *list, u8 row);
void ListDrawInRect(List *list, const Rect *rect);
void ListHandleEvent(List *list, Event *event);
u8 ListSelected(List *list);
void ListSetSelected(List *list, u8 selected);
void ListSetFocused(List *list, u8 focused);
void ListInvalidateRow(List *list, u8 row);
void ListInvalidate(List *list);

void ButtonInit(Button *button, u8 id, u16 left, u16 top,
                u16 right, u16 bottom, const Font *font,
                const char *text);
void ButtonSetText(Button *button, const char *text);
void ButtonDraw(Button *button);
void ButtonDrawInRect(Button *button, const Rect *rect);
void ButtonSetFocused(Button *button, u8 focused);
void ButtonSetPressed(Button *button, u8 pressed);
void ButtonInvalidate(Button *button);

void ViewInit(View *view, u16 left, u16 top, u16 right, u16 bottom,
              ViewDrawProc draw, void *data);
void ViewInvalidate(View *view);
void ViewDraw(View *view, const Rect *dirty);
```

`FormHandleEvent()` returns `FORM_ACTION_NONE` or a button ID. View draw
procedures are small app-owned drawing hooks for custom panes; Form clears the
view rectangle and controls redraw timing. No dynamic allocation is used.

```c
#define UI_MARGIN_SMALL 2
#define UI_MARGIN 4
#define UI_GAP 8
#define UI_ROW_GAP 2
#define UI_BUTTON_PAD_X 4
```

`xtos/ui/layout.h` provides these small manual-layout constants. It is not an
automatic layout system.

## Invalidation

```c
typedef struct Rect {
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
} Rect;

void InvalidateRect(const Rect *rect);
void InvalidateAll(void);
u8 InvalidationIsDirty(void);
void InvalidationGet(Rect *rect);
void InvalidationClear(void);
u8 RectIntersects(const Rect *a, const Rect *b);
```

The invalidation API tracks the current dirty rectangle for runtime-owned
partial redraw. Applications normally invalidate controls or views rather than
driving redraw orchestration directly.

## Placeholder APIs

```c
typedef struct ExecHandle {
    unsigned int id;
} ExecHandle;

typedef struct FileRef {
    unsigned int id;
} FileRef;
```

`xtos/exec.h` and `xtos/file.h` currently expose placeholder handle types only.
There are no public execution or file operations yet.

## Design Rules

- Keep APIs procedural and explicit.
- Keep runtime ownership clear: apps own data, Forms own control redraw,
  runtime owns input devices and cursor.
- Avoid dynamic allocation, callbacks, window hierarchies, and retained widget
  frameworks in Phase 0.
