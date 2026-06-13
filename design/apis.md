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
int TextEncodeUtf8(u16 codepoint, char *out, u8 out_size);

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

## Legacy Forms And Controls

Forms are fixed, static-allocation containers for labels, lists, buttons, and
custom rectangular views. They own redraw policy: static chrome is drawn once,
controls invalidate their own bounds, and `FormDraw()` performs full or
dirty-rectangle redraws. This is the legacy app-local API kept for apps that
still own live UI state or custom views.

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

## Runtime-Owned Forms

Phase 1B adds a handle-based Form API for resident-owned standard controls.
Applications construct a form through explicit calls, then receive semantic
actions. The runtime owns live widget instances, drawing, focus, hit-testing,
button press state, and copied construction strings.

```c
typedef u16 FormId;
typedef u16 ControlId;

#define RT_FORM_INVALID 0

#define RT_FORM_ACTION_NONE 0
#define RT_FORM_ACTION_BUTTON 1
#define RT_FORM_ACTION_LIST_CHANGED 2
#define RT_FORM_ACTION_CLOSE 255

typedef struct FormAction {
    u16 type;
    u16 form_id;
    u16 control_id;
    u16 value;
} FormAction;

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

Current resident limits:

```text
forms:       4
labels:      8 per form
lists:       4 per form
list items:  8 per list
buttons:     8 per form
label text:  96 bytes per label
text pool:   512 bytes
```

Launcher, Showcase, and Control Panel use this API. Font Viewer and Smoke still
use legacy app-local Forms where needed. Runtime-owned custom views are
deferred.

Runtime lists support fixed static text items, resident selected/focused state,
keyboard up/down selection, mouse row selection, and
`RT_FORM_ACTION_LIST_CHANGED` with `control_id` set to the list ID and `value`
set to the selected index. List item strings are copied into resident-owned
storage during `RtFormAddList()`.

Label construction and label updates copy into fixed resident per-label text
buffers, so repeated status updates do not consume the shared construction text
pool.

Runtime Forms track one static dirty bit for chrome and per-control dirty bits
for labels, lists, and buttons. The first draw clears the screen, draws static
chrome, marks all controls dirty, then draws them. Later `RtFormDraw()` calls
only redraw dirty controls and skip `CanvasPresent()` entirely when nothing is
dirty.

`RtFormDispatch()` invalidates the app only when focus, selection, press state,
label text, or a semantic action changes. Mouse movement alone does not redraw
the form. List selection redraws only the previous and new selected rows; list
focus changes redraw the frame. Labels clear the old text area before drawing
updated text. Runtime Forms do not yet use a generic dirty-rectangle engine.

`RtFormInvalidate()` marks runtime-owned static chrome and every resident
control dirty, then invalidates the app. Control Panel uses it after applying
display preferences so a display reset redraws the complete resident form
without legacy form invalidation.

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

## Execution API

```c
int ExecRequest(const char *path);
```

`ExecRequest()` records a desired next XTOS application in the resident runtime
and returns nonzero on success. The current app then exits with `AppQuit()`.
`xtos.com` owns DOS EXEC and launches the requested app as the next sibling
process. This is synchronous single-task handoff, not multitasking or nested
child execution.

`ExecRequest(NULL)`, `ExecRequest("")`, and paths of 64 bytes or longer return
0 and leave any current pending request unchanged. Valid paths are copied during
the INT 60h call; the runtime does not retain the app's pointer.

## Placeholder APIs

```c
typedef struct FileRef {
    unsigned int id;
} FileRef;
```

`xtos/file.h` currently exposes a placeholder handle type only. There are no
public file operations yet.

## Design Rules

- Keep APIs procedural and explicit.
- Keep runtime ownership clear: apps own data, Forms own control redraw,
  runtime owns input devices and cursor.
- Avoid dynamic allocation, callbacks, window hierarchies, and retained widget
  frameworks in Phase 0.
