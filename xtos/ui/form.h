#ifndef XTOS_UI_FORM_H
#define XTOS_UI_FORM_H

#include "../event.h"
#include "../types.h"
#include "canvas.h"
#include "font.h"
#include "invalidation.h"
#include "layout.h"

#define FORM_MAX_LABELS 8
#define FORM_MAX_LISTS 4
#define FORM_MAX_BUTTONS 4
#define FORM_MAX_VIEWS 4

#define FORM_ACTION_NONE 0
#define FORM_ACTION_CLOSE 255

#define FORM_FOCUS_NONE 0
#define FORM_FOCUS_LIST 1
#define FORM_FOCUS_BUTTON 2

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

typedef struct View View;

typedef void (*ViewDrawProc)(View *view, const Rect *dirty, void *data);

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

#endif
