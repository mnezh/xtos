#include "xtos/display.h"
#include "xtos/ui/form.h"

#define FORM_TITLE_HEIGHT 9
#define UI_TITLE_PADDING 4
#define UI_CLOSE_SIZE 7
#define UI_CLOSE_LEFT 4
#define UI_CLOSE_TOP 1
#define LIST_PADDING_Y 2
#define FORM_CONTENT_LEFT 1
#define FORM_CONTENT_TOP (FORM_TITLE_HEIGHT + 1)

void FormSetFocusedList(Form *form, List *list);
void FormSetFocusedButton(Form *form, Button *button);

static u8 rect_is_full_screen(const Rect *rect)
{
    if (rect == 0) {
        return 0;
    }

    return rect->left == 0 &&
           rect->top == 0 &&
           rect->right >= (u16)(DisplayWidth() - 1) &&
           rect->bottom >= (u16)(DisplayHeight() - 1);
}

static u16 title_tab_right(Form *form)
{
    u16 width;

    width = (u16)(UI_TITLE_PADDING + UI_CLOSE_SIZE +
                  UI_MARGIN + CanvasTextWidth(form->font, form->title) +
                  UI_TITLE_PADDING);

    if (width > (u16)(DisplayWidth() - 2)) {
        width = (u16)(DisplayWidth() - 2);
    }

    return width;
}

static u8 point_in_close_box(u16 x, u16 y)
{
    return x >= UI_CLOSE_LEFT &&
           x <= (u16)(UI_CLOSE_LEFT + UI_CLOSE_SIZE - 1) &&
           y >= UI_CLOSE_TOP &&
           y <= (u16)(UI_CLOSE_TOP + UI_CLOSE_SIZE - 1);
}

static u16 form_screen_x(u16 x)
{
    return (u16)(x + FORM_CONTENT_LEFT);
}

static u16 form_screen_y(u16 y)
{
    return (u16)(y + FORM_CONTENT_TOP);
}

void FormInit(Form *form, const char *title, const Font *font)
{
    form->title = title;
    form->font = font;
    form->label_count = 0;
    form->list_count = 0;
    form->button_count = 0;
    form->view_count = 0;
    form->focused_list = 0;
    form->focused_button = 0;
    form->pressed_button = 0;
    form->focused_type = FORM_FOCUS_NONE;
    form->static_drawn = 0;
}

static u8 point_in_button(Button *button, u16 x, u16 y)
{
    if (button == 0) {
        return 0;
    }

    return x >= button->left && x <= button->right &&
           y >= button->top && y <= button->bottom;
}

static u8 point_in_list(List *list, u16 x, u16 y)
{
    u16 bottom;

    if (list == 0) {
        return 0;
    }

    bottom = (u16)(list->y + LIST_PADDING_Y +
                   list->count * list->row_height +
                   LIST_PADDING_Y - 1);
    return x >= list->x &&
           x <= (u16)(list->x + list->width - 1) &&
           y >= list->y &&
           y <= bottom;
}

static u8 list_row_at(List *list, u16 y)
{
    if (y < (u16)(list->y + LIST_PADDING_Y)) {
        return 0;
    }

    return (u8)((y - list->y - LIST_PADDING_Y) / list->row_height);
}

static List *hit_list(Form *form, u16 x, u16 y)
{
    u8 i;

    for (i = 0; i < form->list_count; ++i) {
        if (point_in_list(form->lists[i], x, y)) {
            return form->lists[i];
        }
    }

    return 0;
}

static Button *hit_button(Form *form, u16 x, u16 y)
{
    u8 i;

    for (i = 0; i < form->button_count; ++i) {
        if (point_in_button(form->buttons[i], x, y)) {
            return form->buttons[i];
        }
    }

    return 0;
}

void FormAddLabel(Form *form, Label *label)
{
    if (form->label_count < FORM_MAX_LABELS) {
        label->x = form_screen_x(label->x);
        label->y = form_screen_y(label->y);
        form->labels[form->label_count] = label;
        ++form->label_count;
    }
}

void FormAddList(Form *form, List *list)
{
    if (form->list_count < FORM_MAX_LISTS) {
        list->x = form_screen_x(list->x);
        list->y = form_screen_y(list->y);
        form->lists[form->list_count] = list;
        ++form->list_count;

        if (form->focused_type == FORM_FOCUS_NONE) {
            FormSetFocusedList(form, list);
        }
    }
}

void FormAddButton(Form *form, Button *button)
{
    if (form->button_count < FORM_MAX_BUTTONS) {
        button->left = form_screen_x(button->left);
        button->top = form_screen_y(button->top);
        button->right = form_screen_x(button->right);
        button->bottom = form_screen_y(button->bottom);
        form->buttons[form->button_count] = button;
        ++form->button_count;

        if (form->focused_type == FORM_FOCUS_NONE) {
            FormSetFocusedButton(form, button);
        }
    }
}

void FormAddView(Form *form, View *view)
{
    if (form->view_count < FORM_MAX_VIEWS) {
        view->left = form_screen_x(view->left);
        view->top = form_screen_y(view->top);
        view->right = form_screen_x(view->right);
        view->bottom = form_screen_y(view->bottom);
        form->views[form->view_count] = view;
        ++form->view_count;
    }
}

void FormDrawStatic(Form *form)
{
    u16 right;
    u16 bottom;
    u16 tab_right;
    u16 close_right;
    u16 close_bottom;

    right = (u16)(DisplayWidth() - 1);
    bottom = (u16)(DisplayHeight() - 1);
    tab_right = title_tab_right(form);
    close_right = (u16)(UI_CLOSE_LEFT + UI_CLOSE_SIZE - 1);
    close_bottom = (u16)(UI_CLOSE_TOP + UI_CLOSE_SIZE - 1);

    CanvasRect(0, FORM_TITLE_HEIGHT, right, bottom, CANVAS_PRIMARY_FOREGROUND);

    CanvasFillRect(0, 0, tab_right, FORM_TITLE_HEIGHT,
                   CANVAS_INVERTED_BACKGROUND);
    /* Cheap rounded tab corners: 00xxxx / 0xxxxx / xxxxxx. */
    CanvasClearRect(0, 0, 0, 0);
    CanvasClearRect(1, 0, 1, 0);
    CanvasClearRect(0, 1, 0, 1);
    CanvasClearRect((u16)(tab_right - 1), 0, (u16)(tab_right - 1), 0);
    CanvasClearRect(tab_right, 0, tab_right, 0);
    CanvasClearRect(tab_right, 1, tab_right, 1);

    CanvasRect(UI_CLOSE_LEFT, UI_CLOSE_TOP, close_right, close_bottom,
               CANVAS_INVERTED_FOREGROUND);

    if (form->title != 0 && form->font != 0) {
        CanvasText((u16)(UI_CLOSE_LEFT + UI_CLOSE_SIZE + UI_MARGIN),
                   1, form->font, CANVAS_INVERTED_FOREGROUND, form->title);
    }
}

void ViewInit(View *view, u16 left, u16 top, u16 right, u16 bottom,
              ViewDrawProc draw, void *data)
{
    view->left = left;
    view->top = top;
    view->right = right;
    view->bottom = bottom;
    view->draw = draw;
    view->data = data;
}

void ViewInvalidate(View *view)
{
    Rect rect;

    if (view == 0) {
        return;
    }

    rect.left = view->left;
    rect.top = view->top;
    rect.right = view->right;
    rect.bottom = view->bottom;
    InvalidateRect(&rect);
}

void ViewDraw(View *view, const Rect *dirty)
{
    Rect bounds;

    if (view == 0) {
        return;
    }

    bounds.left = view->left;
    bounds.top = view->top;
    bounds.right = view->right;
    bounds.bottom = view->bottom;

    CanvasClearRect(bounds.left, bounds.top, bounds.right, bounds.bottom);

    if (view->draw != 0) {
        view->draw(view, dirty != 0 ? dirty : &bounds, view->data);
    }
}

void FormDrawControls(Form *form)
{
    u8 i;

    for (i = 0; i < form->view_count; ++i) {
        ViewDraw(form->views[i], 0);
    }

    for (i = 0; i < form->label_count; ++i) {
        LabelDraw(form->labels[i]);
    }

    for (i = 0; i < form->list_count; ++i) {
        ListDraw(form->lists[i]);
    }

    for (i = 0; i < form->button_count; ++i) {
        ButtonDraw(form->buttons[i]);
    }
}

void FormDrawControlsInRect(Form *form, const Rect *rect)
{
    u8 i;
    Rect bounds;

    if (form == 0 || rect == 0) {
        return;
    }

    for (i = 0; i < form->view_count; ++i) {
        bounds.left = form->views[i]->left;
        bounds.top = form->views[i]->top;
        bounds.right = form->views[i]->right;
        bounds.bottom = form->views[i]->bottom;

        if (RectIntersects(&bounds, rect)) {
            ViewDraw(form->views[i], rect);
        }
    }

    for (i = 0; i < form->label_count; ++i) {
        LabelDrawInRect(form->labels[i], rect);
    }

    for (i = 0; i < form->list_count; ++i) {
        ListDrawInRect(form->lists[i], rect);
    }

    for (i = 0; i < form->button_count; ++i) {
        ButtonDrawInRect(form->buttons[i], rect);
    }
}

void FormDraw(Form *form)
{
    Rect dirty;

    if (form == 0) {
        return;
    }

    InvalidationGet(&dirty);

    if (!form->static_drawn || rect_is_full_screen(&dirty)) {
        CanvasClear();
        FormDrawStatic(form);
        FormDrawControls(form);
        form->static_drawn = 1;
        CanvasPresent();
        return;
    }

    FormDrawControlsInRect(form, &dirty);
    CanvasPresent();
}

int FormHandleEvent(Form *form, Event *event)
{
    List *list;
    Button *button;
    u8 row;

    if (form == 0 || event == 0) {
        return FORM_ACTION_NONE;
    }

    if (event->type == EVENT_KEYDOWN && event->key == XTOS_KEY_TAB) {
        FormFocusNextControl(form);
        return FORM_ACTION_NONE;
    }

    if (event->type == EVENT_MOUSE_DOWN) {
        if (point_in_close_box(event->x, event->y)) {
            return FORM_ACTION_CLOSE;
        }

        list = hit_list(form, event->x, event->y);

        if (list != 0) {
            row = list_row_at(list, event->y);
            FormSetFocusedList(form, list);
            ListSetSelected(list, row);
            return FORM_ACTION_NONE;
        }

        button = hit_button(form, event->x, event->y);

        if (button != 0) {
            FormSetFocusedButton(form, button);
            form->pressed_button = button;
            ButtonSetPressed(button, 1);
            return FORM_ACTION_NONE;
        }
    }

    if (event->type == EVENT_MOUSE_UP) {
        button = form->pressed_button;
        form->pressed_button = 0;

        if (button != 0) {
            ButtonSetPressed(button, 0);

            if (point_in_button(button, event->x, event->y)) {
                return button->id;
            }
        }

        return FORM_ACTION_NONE;
    }

    if (form->focused_type == FORM_FOCUS_LIST && form->focused_list != 0) {
        ListHandleEvent(form->focused_list, event);
    } else if (form->focused_type == FORM_FOCUS_BUTTON &&
               form->focused_button != 0 &&
               event->type == EVENT_KEYDOWN &&
               (event->key == XTOS_KEY_ENTER || event->key == ' ')) {
        return form->focused_button->id;
    }

    return FORM_ACTION_NONE;
}

void FormInvalidate(Form *form)
{
    if (form != 0) {
        form->static_drawn = 0;
    }

    InvalidateAll();
}

void FormInvalidateAll(Form *form)
{
    FormInvalidate(form);
}

void FormSetFocusedList(Form *form, List *list)
{
    if (form == 0 || list == 0 ||
        (form->focused_type == FORM_FOCUS_LIST &&
         form->focused_list == list)) {
        return;
    }

    if (form->focused_list != 0) {
        ListSetFocused(form->focused_list, 0);
    }

    if (form->focused_button != 0) {
        ButtonSetFocused(form->focused_button, 0);
    }

    form->focused_list = list;
    form->focused_button = 0;
    form->pressed_button = 0;
    form->focused_type = FORM_FOCUS_LIST;
    ListSetFocused(form->focused_list, 1);
}

void FormSetFocusedButton(Form *form, Button *button)
{
    if (form == 0 || button == 0 ||
        (form->focused_type == FORM_FOCUS_BUTTON &&
         form->focused_button == button)) {
        return;
    }

    if (form->focused_list != 0) {
        ListSetFocused(form->focused_list, 0);
    }

    if (form->focused_button != 0) {
        ButtonSetFocused(form->focused_button, 0);
    }

    form->focused_list = 0;
    form->focused_button = button;
    form->pressed_button = 0;
    form->focused_type = FORM_FOCUS_BUTTON;
    ButtonSetFocused(form->focused_button, 1);
}

void FormFocusNextList(Form *form)
{
    u8 i;

    if (form == 0 || form->list_count == 0) {
        return;
    }

    for (i = 0; i < form->list_count; ++i) {
        if (form->lists[i] == form->focused_list) {
            FormSetFocusedList(form,
                               form->lists[(u8)((i + 1) %
                                            form->list_count)]);
            return;
        }
    }

    FormSetFocusedList(form, form->lists[0]);
}

void FormFocusNextControl(Form *form)
{
    u8 i;

    if (form == 0) {
        return;
    }

    if (form->focused_type == FORM_FOCUS_LIST) {
        for (i = 0; i < form->list_count; ++i) {
            if (form->lists[i] == form->focused_list) {
                if ((u8)(i + 1) < form->list_count) {
                    FormSetFocusedList(form, form->lists[(u8)(i + 1)]);
                } else if (form->button_count > 0) {
                    FormSetFocusedButton(form, form->buttons[0]);
                } else if (form->list_count > 0) {
                    FormSetFocusedList(form, form->lists[0]);
                }

                return;
            }
        }
    }

    if (form->focused_type == FORM_FOCUS_BUTTON) {
        for (i = 0; i < form->button_count; ++i) {
            if (form->buttons[i] == form->focused_button) {
                if ((u8)(i + 1) < form->button_count) {
                    FormSetFocusedButton(form, form->buttons[(u8)(i + 1)]);
                } else if (form->list_count > 0) {
                    FormSetFocusedList(form, form->lists[0]);
                } else if (form->button_count > 0) {
                    FormSetFocusedButton(form, form->buttons[0]);
                }

                return;
            }
        }
    }

    if (form->list_count > 0) {
        FormSetFocusedList(form, form->lists[0]);
    } else if (form->button_count > 0) {
        FormSetFocusedButton(form, form->buttons[0]);
    }
}

List *FormFocusedList(Form *form)
{
    if (form == 0) {
        return 0;
    }

    return form->focused_list;
}

Button *FormFocusedButton(Form *form)
{
    if (form == 0) {
        return 0;
    }

    return form->focused_button;
}
