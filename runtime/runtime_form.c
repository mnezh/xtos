#include "canvas.h"
#include "display.h"
#include "log.h"
#include "runtime_form.h"

#define RT_FORM_MAX_FORMS 4
#define RT_FORM_MAX_LABELS 8
#define RT_FORM_MAX_LISTS 4
#define RT_FORM_MAX_BUTTONS 8
#define RT_FORM_MAX_LIST_ITEMS 8
#define RT_FORM_MAX_LABEL_TEXT 96
#define RT_FORM_MAX_TEXT_BYTES 512

#define FORM_TITLE_HEIGHT 9
#define UI_TITLE_PADDING 4
#define UI_CLOSE_SIZE 7
#define UI_CLOSE_LEFT 4
#define UI_CLOSE_TOP 1
#define UI_MARGIN 4
#define UI_BUTTON_PAD_X 4
#define FORM_CONTENT_LEFT 1
#define FORM_CONTENT_TOP (FORM_TITLE_HEIGHT + 1)
#define LIST_PADDING_Y 2

#define RT_FORM_FOCUS_NONE 0
#define RT_FORM_FOCUS_LIST 1
#define RT_FORM_FOCUS_BUTTON 2

typedef struct RuntimeLabel {
    u8 used;
    u8 dirty;
    ControlId id;
    u16 x;
    u16 y;
    FontId font_id;
    enum CanvasColorRole color;
    char text[RT_FORM_MAX_LABEL_TEXT];
    u16 dirty_width;
} RuntimeLabel;

typedef struct RuntimeList {
    u8 used;
    u8 dirty;
    u8 dirty_frame;
    u16 dirty_rows;
    ControlId id;
    u16 x;
    u16 y;
    u16 width;
    FontId font_id;
    const char *items[RT_FORM_MAX_LIST_ITEMS];
    u8 count;
    u8 selected;
    u8 row_height;
    u8 focused;
} RuntimeList;

typedef struct RuntimeButton {
    u8 used;
    u8 dirty;
    ControlId id;
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
    FontId font_id;
    const char *text;
    u8 focused;
    u8 pressed;
} RuntimeButton;

typedef struct RuntimeForm {
    u8 used;
    u8 dirty_static;
    FormId id;
    FontId font_id;
    const char *title;
    u8 label_count;
    u8 list_count;
    u8 button_count;
    u8 focused_type;
    u8 focused_list;
    u8 focused_button;
    u8 pressed_button;
    RuntimeLabel labels[RT_FORM_MAX_LABELS];
    RuntimeList lists[RT_FORM_MAX_LISTS];
    RuntimeButton buttons[RT_FORM_MAX_BUTTONS];
} RuntimeForm;

static RuntimeForm runtime_forms[RT_FORM_MAX_FORMS];
static char runtime_form_text[RT_FORM_MAX_TEXT_BYTES];
static u16 runtime_form_text_used;
static FormId runtime_next_form_id = 1;

const Font *RuntimeFontById(FontId id);
enum DisplayMode RuntimeDisplayCurrentMode(void);
u8 RuntimeFontHeight(FontId id);

static u16 screen_width(void)
{
    return RuntimeDisplayCurrentMode() == DISPLAY_MODE_HIGH ? 640 : 320;
}

static u16 screen_height(void)
{
    return 200;
}

static u16 form_screen_x(u16 x)
{
    return (u16)(x + FORM_CONTENT_LEFT);
}

static u16 form_screen_y(u16 y)
{
    return (u16)(y + FORM_CONTENT_TOP);
}

static RuntimeForm *form_by_id(FormId form_id)
{
    u8 i;

    if (form_id == RT_FORM_INVALID) {
        return 0;
    }

    for (i = 0; i < RT_FORM_MAX_FORMS; ++i) {
        if (runtime_forms[i].used && runtime_forms[i].id == form_id) {
            return &runtime_forms[i];
        }
    }

    return 0;
}

static u8 any_form_used(void)
{
    u8 i;

    for (i = 0; i < RT_FORM_MAX_FORMS; ++i) {
        if (runtime_forms[i].used) {
            return 1;
        }
    }

    return 0;
}

static char *copy_form_text(const char *text)
{
    u16 len;
    u16 i;
    char *dest;

    if (text == 0) {
        return 0;
    }

    len = 0;
    while (text[len] != 0 && len < 95) {
        ++len;
    }

    if ((u16)(runtime_form_text_used + len + 1) >
        RT_FORM_MAX_TEXT_BYTES) {
        return 0;
    }

    dest = &runtime_form_text[runtime_form_text_used];
    for (i = 0; i <= len; ++i) {
        dest[i] = text[i];
    }
    dest[len] = 0;
    runtime_form_text_used = (u16)(runtime_form_text_used + len + 1);
    return dest;
}

static u8 copy_label_text(RuntimeLabel *label, const char *text)
{
    u16 i;

    if (label == 0 || text == 0) {
        return 0;
    }

    for (i = 0; i < (u16)(RT_FORM_MAX_LABEL_TEXT - 1) && text[i] != 0; ++i) {
        label->text[i] = text[i];
    }
    label->text[i] = 0;
    return 1;
}

static u16 label_text_width(RuntimeLabel *label)
{
    if (label == 0) {
        return 0;
    }

    return RuntimeCanvasTextWidth(RuntimeFontById(label->font_id),
                                  label->text);
}

static void mark_label_dirty(RuntimeLabel *label, u16 dirty_width)
{
    if (label == 0) {
        return;
    }

    label->dirty = 1;
    if (dirty_width > label->dirty_width) {
        label->dirty_width = dirty_width;
    }
}

static void mark_list_dirty(RuntimeList *list)
{
    u8 i;

    if (list != 0) {
        list->dirty = 1;
        list->dirty_rows = 0;
        for (i = 0; i < list->count; ++i) {
            list->dirty_rows |= (u16)(1u << i);
        }
    }
}

static void mark_list_row_dirty(RuntimeList *list, u8 row)
{
    if (list != 0 && row < list->count) {
        list->dirty = 1;
        list->dirty_rows |= (u16)(1u << row);
    }
}

static void mark_list_frame_dirty(RuntimeList *list)
{
    if (list != 0) {
        list->dirty_frame = 1;
    }
}

static void mark_button_dirty(RuntimeButton *button)
{
    if (button != 0) {
        button->dirty = 1;
    }
}

static u16 title_tab_right(RuntimeForm *form)
{
    u16 width;

    width = (u16)(UI_TITLE_PADDING + UI_CLOSE_SIZE + UI_MARGIN +
                  RuntimeCanvasTextWidth(RuntimeFontById(form->font_id),
                                         form->title) +
                  UI_TITLE_PADDING);

    if (width > (u16)(screen_width() - 2)) {
        width = (u16)(screen_width() - 2);
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

static u8 point_in_button(RuntimeButton *button, u16 x, u16 y)
{
    if (button == 0 || !button->used) {
        return 0;
    }

    return x >= button->left && x <= button->right &&
           y >= button->top && y <= button->bottom;
}

static RuntimeButton *hit_button(RuntimeForm *form, u16 x, u16 y)
{
    u8 i;

    for (i = 0; i < form->button_count; ++i) {
        if (point_in_button(&form->buttons[i], x, y)) {
            return &form->buttons[i];
        }
    }

    return 0;
}

static u16 list_bottom(RuntimeList *list)
{
    return (u16)(list->y + LIST_PADDING_Y +
                 list->count * list->row_height + LIST_PADDING_Y - 1);
}

static u8 point_in_list(RuntimeList *list, u16 x, u16 y)
{
    if (list == 0 || !list->used) {
        return 0;
    }

    return x >= list->x &&
           x <= (u16)(list->x + list->width - 1) &&
           y >= list->y &&
           y <= list_bottom(list);
}

static u8 list_row_at(RuntimeList *list, u16 y)
{
    if (y < (u16)(list->y + LIST_PADDING_Y)) {
        return 0;
    }

    return (u8)((y - list->y - LIST_PADDING_Y) / list->row_height);
}

static RuntimeList *hit_list(RuntimeForm *form, u16 x, u16 y)
{
    u8 i;

    for (i = 0; i < form->list_count; ++i) {
        if (point_in_list(&form->lists[i], x, y)) {
            return &form->lists[i];
        }
    }

    return 0;
}

static RuntimeLabel *label_by_id(RuntimeForm *form, ControlId control_id)
{
    u8 i;

    if (form == 0) {
        return 0;
    }

    for (i = 0; i < form->label_count; ++i) {
        if (form->labels[i].used && form->labels[i].id == control_id) {
            return &form->labels[i];
        }
    }

    return 0;
}

static RuntimeList *list_by_id(RuntimeForm *form, ControlId control_id)
{
    u8 i;

    if (form == 0) {
        return 0;
    }

    for (i = 0; i < form->list_count; ++i) {
        if (form->lists[i].used && form->lists[i].id == control_id) {
            return &form->lists[i];
        }
    }

    return 0;
}

static u8 clear_focus(RuntimeForm *form)
{
    u8 i;
    u8 changed;

    if (form == 0) {
        return 0;
    }

    changed = 0;

    for (i = 0; i < form->list_count; ++i) {
        if (form->lists[i].focused) {
            form->lists[i].focused = 0;
            mark_list_frame_dirty(&form->lists[i]);
            changed = 1;
        }
    }
    for (i = 0; i < form->button_count; ++i) {
        if (form->buttons[i].focused) {
            form->buttons[i].focused = 0;
            mark_button_dirty(&form->buttons[i]);
            changed = 1;
        }
    }

    return changed;
}

static u8 set_focused_list(RuntimeForm *form, RuntimeList *list)
{
    if (form == 0 || list == 0) {
        return 0;
    }

    if (list->focused && form->focused_type == RT_FORM_FOCUS_LIST) {
        return 0;
    }

    clear_focus(form);
    list->focused = 1;
    mark_list_frame_dirty(list);
    form->focused_list = (u8)(list - form->lists);
    form->focused_type = RT_FORM_FOCUS_LIST;
    form->pressed_button = 0xff;
    return 1;
}

static u8 set_focused_button(RuntimeForm *form, RuntimeButton *button)
{
    if (form == 0 || button == 0) {
        return 0;
    }

    if (button->focused && form->focused_type == RT_FORM_FOCUS_BUTTON) {
        return 0;
    }

    clear_focus(form);
    button->focused = 1;
    mark_button_dirty(button);
    form->focused_button = (u8)(button - form->buttons);
    form->focused_type = RT_FORM_FOCUS_BUTTON;
    return 1;
}

static u8 focus_next_control(RuntimeForm *form)
{
    u8 next;

    if (form == 0) {
        return 0;
    }

    if (form->focused_type == RT_FORM_FOCUS_LIST) {
        if ((u8)(form->focused_list + 1) < form->list_count) {
            return set_focused_list(form,
                                    &form->lists[(u8)(form->focused_list + 1)]);
        }
        if (form->button_count > 0) {
            return set_focused_button(form, &form->buttons[0]);
        }
        if (form->list_count > 0) {
            return set_focused_list(form, &form->lists[0]);
        }
    }

    if (form->focused_type == RT_FORM_FOCUS_BUTTON) {
        if ((u8)(form->focused_button + 1) < form->button_count) {
            next = (u8)(form->focused_button + 1);
            return set_focused_button(form, &form->buttons[next]);
        }
        if (form->list_count > 0) {
            return set_focused_list(form, &form->lists[0]);
        }
        if (form->button_count > 0) {
            return set_focused_button(form, &form->buttons[0]);
        }
    }

    if (form->list_count > 0) {
        return set_focused_list(form, &form->lists[0]);
    }
    if (form->button_count > 0) {
        return set_focused_button(form, &form->buttons[0]);
    }

    return 0;
}

static void clear_action(FormAction XTOS_FAR *action)
{
    if (action == 0) {
        return;
    }

    action->type = RT_FORM_ACTION_NONE;
    action->form_id = RT_FORM_INVALID;
    action->control_id = 0;
    action->value = 0;
}

static void set_action(FormAction XTOS_FAR *action, u16 type,
                       FormId form_id, ControlId control_id, u16 value)
{
    if (action == 0) {
        return;
    }

    action->type = type;
    action->form_id = form_id;
    action->control_id = control_id;
    action->value = value;
}

static void draw_button_outline(RuntimeButton *button,
                                enum CanvasColorRole role)
{
    RuntimeCanvasFillRect((u16)(button->left + 2), button->top,
                          (u16)(button->right - 2), button->top, role);
    RuntimeCanvasFillRect((u16)(button->left + 1), (u16)(button->top + 1),
                          (u16)(button->left + 1), (u16)(button->top + 1),
                          role);
    RuntimeCanvasFillRect((u16)(button->right - 1), (u16)(button->top + 1),
                          (u16)(button->right - 1), (u16)(button->top + 1),
                          role);
    RuntimeCanvasFillRect(button->left, (u16)(button->top + 2),
                          button->left, (u16)(button->bottom - 2), role);
    RuntimeCanvasFillRect(button->right, (u16)(button->top + 2),
                          button->right, (u16)(button->bottom - 2), role);
    RuntimeCanvasFillRect((u16)(button->left + 1),
                          (u16)(button->bottom - 1),
                          (u16)(button->left + 1),
                          (u16)(button->bottom - 1), role);
    RuntimeCanvasFillRect((u16)(button->right - 1),
                          (u16)(button->bottom - 1),
                          (u16)(button->right - 1),
                          (u16)(button->bottom - 1), role);
    RuntimeCanvasFillRect((u16)(button->left + 2), button->bottom,
                          (u16)(button->right - 2), button->bottom, role);
}

static void fill_button_shape(RuntimeButton *button,
                              enum CanvasColorRole role)
{
    RuntimeCanvasFillRect((u16)(button->left + 2), button->top,
                          (u16)(button->right - 2), button->top, role);
    RuntimeCanvasFillRect((u16)(button->left + 1), (u16)(button->top + 1),
                          (u16)(button->right - 1),
                          (u16)(button->top + 1), role);
    RuntimeCanvasFillRect(button->left, (u16)(button->top + 2),
                          button->right, (u16)(button->bottom - 2), role);
    RuntimeCanvasFillRect((u16)(button->left + 1),
                          (u16)(button->bottom - 1),
                          (u16)(button->right - 1),
                          (u16)(button->bottom - 1), role);
    RuntimeCanvasFillRect((u16)(button->left + 2), button->bottom,
                          (u16)(button->right - 2), button->bottom, role);
}

static void draw_button(RuntimeButton *button)
{
    u16 text_x;
    u16 text_y;
    u16 text_right;
    u16 underline_y;
    enum CanvasColorRole text_role;

    RuntimeCanvasClearRect(button->left, button->top,
                           button->right, button->bottom);

    if (button->pressed) {
        fill_button_shape(button, CANVAS_PRIMARY_FOREGROUND);
        text_role = CANVAS_PRIMARY_BACKGROUND;
    } else {
        draw_button_outline(button, CANVAS_PRIMARY_FOREGROUND);
        text_role = CANVAS_PRIMARY_FOREGROUND;
    }

    text_x = (u16)(button->left + UI_BUTTON_PAD_X);
    text_y = (u16)(button->top + 3);
    RuntimeCanvasText(text_x, text_y, RuntimeFontById(button->font_id),
                      text_role, button->text);

    if (button->focused && !button->pressed) {
        text_right = (u16)(text_x +
                           RuntimeCanvasTextWidth(
                               RuntimeFontById(button->font_id),
                               button->text) - 1);
        if (text_right > (u16)(button->right - 3)) {
            text_right = (u16)(button->right - 3);
        }

        underline_y = (u16)(text_y + RuntimeFontHeight(button->font_id));
        if (underline_y < button->bottom) {
            RuntimeCanvasFillRect(text_x, underline_y, text_right,
                                  underline_y, CANVAS_PRIMARY_FOREGROUND);
        }
    }

    button->dirty = 0;
}

static void draw_list_boundary(RuntimeList *list)
{
    u16 right;
    u16 bottom;

    right = (u16)(list->x + list->width - 1);
    bottom = list_bottom(list);

    RuntimeCanvasClearRect(list->x, list->y, right, list->y);
    RuntimeCanvasClearRect(list->x, bottom, right, bottom);
    RuntimeCanvasClearRect(list->x, (u16)(list->y + 1), list->x,
                           (u16)(bottom - 1));
    RuntimeCanvasClearRect(right, (u16)(list->y + 1), right,
                           (u16)(bottom - 1));

    if (list->focused) {
        RuntimeCanvasRect(list->x, list->y, right, bottom,
                          CANVAS_PRIMARY_FOREGROUND);
    } else {
        RuntimeCanvasDottedRect(list->x, list->y, right, bottom,
                                CANVAS_PRIMARY_FOREGROUND);
    }
}

static void draw_list_row(RuntimeList *list, u8 row)
{
    u16 y;
    u16 right;

    if (list == 0 || row >= list->count || list->items[row] == 0) {
        return;
    }

    y = (u16)(list->y + LIST_PADDING_Y + row * list->row_height);
    right = (u16)(list->x + list->width - 2);

    RuntimeCanvasClearRect((u16)(list->x + 1), y, right,
                           (u16)(y + list->row_height - 1));

    if (row == list->selected) {
        RuntimeCanvasFillRect((u16)(list->x + 1), y, right,
                              (u16)(y + list->row_height - 1),
                              CANVAS_INVERTED_BACKGROUND);
        RuntimeCanvasText((u16)(list->x + 3), (u16)(y + 1),
                          RuntimeFontById(list->font_id),
                          CANVAS_INVERTED_FOREGROUND, list->items[row]);
    } else {
        RuntimeCanvasText((u16)(list->x + 3), (u16)(y + 1),
                          RuntimeFontById(list->font_id),
                          CANVAS_PRIMARY_FOREGROUND, list->items[row]);
    }
}

static void draw_list(RuntimeList *list)
{
    u8 i;

    if (list == 0) {
        return;
    }

    for (i = 0; i < list->count; ++i) {
        if ((list->dirty_rows & (u16)(1u << i)) != 0) {
            draw_list_row(list, i);
        }
    }
    list->dirty = 0;
    list->dirty_rows = 0;
}

static u8 list_set_selected(RuntimeList *list, u8 selected)
{
    u8 old_selected;

    if (list == 0 || selected >= list->count ||
        selected == list->selected) {
        return 0;
    }

    old_selected = list->selected;
    list->selected = selected;
    mark_list_row_dirty(list, old_selected);
    mark_list_row_dirty(list, selected);
    return 1;
}

static void draw_label(RuntimeLabel *label)
{
    u16 width;

    if (label == 0) {
        return;
    }

    width = label->dirty_width;
    if (width == 0) {
        width = label_text_width(label);
    }

    if (width > 0) {
        RuntimeCanvasClearRect(label->x, label->y,
                               (u16)(label->x + width),
                               (u16)(label->y +
                                     RuntimeFontHeight(label->font_id)));
    }

    RuntimeCanvasText(label->x, label->y, RuntimeFontById(label->font_id),
                      label->color, label->text);
    label->dirty = 0;
    label->dirty_width = label_text_width(label);
}

static void draw_static(RuntimeForm *form)
{
    u16 right;
    u16 bottom;
    u16 tab_right;
    u16 close_right;
    u16 close_bottom;

    right = (u16)(screen_width() - 1);
    bottom = (u16)(screen_height() - 1);
    tab_right = title_tab_right(form);
    close_right = (u16)(UI_CLOSE_LEFT + UI_CLOSE_SIZE - 1);
    close_bottom = (u16)(UI_CLOSE_TOP + UI_CLOSE_SIZE - 1);

    RuntimeCanvasRect(0, FORM_TITLE_HEIGHT, right, bottom,
                      CANVAS_PRIMARY_FOREGROUND);
    RuntimeCanvasFillRect(0, 0, tab_right, FORM_TITLE_HEIGHT,
                          CANVAS_INVERTED_BACKGROUND);
    RuntimeCanvasClearRect(0, 0, 0, 0);
    RuntimeCanvasClearRect(1, 0, 1, 0);
    RuntimeCanvasClearRect(0, 1, 0, 1);
    RuntimeCanvasClearRect((u16)(tab_right - 1), 0,
                           (u16)(tab_right - 1), 0);
    RuntimeCanvasClearRect(tab_right, 0, tab_right, 0);
    RuntimeCanvasClearRect(tab_right, 1, tab_right, 1);
    RuntimeCanvasRect(UI_CLOSE_LEFT, UI_CLOSE_TOP, close_right,
                      close_bottom, CANVAS_INVERTED_FOREGROUND);

    RuntimeCanvasText((u16)(UI_CLOSE_LEFT + UI_CLOSE_SIZE + UI_MARGIN), 1,
                      RuntimeFontById(form->font_id),
                      CANVAS_INVERTED_FOREGROUND, form->title);
}

FormId RuntimeFormCreateText(FontId font_id, const char *title)
{
    u8 i;
    RuntimeForm *form;
    char *title_copy;

    title_copy = copy_form_text(title);
    if (title_copy == 0) {
        XTOS_LOG_PREFIX("[FORM]", "create_title_fail");
        return RT_FORM_INVALID;
    }

    for (i = 0; i < RT_FORM_MAX_FORMS; ++i) {
        if (!runtime_forms[i].used) {
            form = &runtime_forms[i];
            form->used = 1;
            form->dirty_static = 1;
            form->id = runtime_next_form_id++;
            if (runtime_next_form_id == RT_FORM_INVALID) {
                runtime_next_form_id = 1;
            }
            form->font_id = font_id;
            form->title = title_copy;
            form->label_count = 0;
            form->list_count = 0;
            form->button_count = 0;
            form->focused_type = RT_FORM_FOCUS_NONE;
            form->focused_list = 0;
            form->focused_button = 0;
            form->pressed_button = 0xff;
            XTOS_LOG_PREFIX_U16("[FORM]", "create", form->id);
            return form->id;
        }
    }

    XTOS_LOG_PREFIX("[FORM]", "create_pool_fail");
    return RT_FORM_INVALID;
}

u16 RuntimeFormDestroy(FormId form_id)
{
    RuntimeForm *form;

    form = form_by_id(form_id);
    if (form == 0) {
        return 0;
    }

    form->used = 0;
    if (!any_form_used()) {
        runtime_form_text_used = 0;
    }
    return 1;
}

u16 RuntimeFormAddLabelText(FormId form_id, ControlId control_id,
                            u16 x, u16 y, FontId font_id, u16 color,
                            const char *text)
{
    RuntimeForm *form;
    RuntimeLabel *label;

    form = form_by_id(form_id);
    if (form == 0 || form->label_count >= RT_FORM_MAX_LABELS) {
        XTOS_LOG_PREFIX("[FORM]", "add_label_fail");
        return 0;
    }

    label = &form->labels[form->label_count++];
    label->used = 1;
    label->dirty = 1;
    label->id = control_id;
    label->x = form_screen_x(x);
    label->y = form_screen_y(y);
    label->font_id = font_id;
    label->color = (enum CanvasColorRole)color;

    if (!copy_label_text(label, text)) {
        --form->label_count;
        XTOS_LOG_PREFIX("[FORM]", "add_label_text_fail");
        return 0;
    }
    label->dirty_width = label_text_width(label);

    XTOS_LOG_PREFIX_U16("[FORM]", "add_label", control_id);
    return 1;
}

u16 RuntimeFormAddButtonText(FormId form_id, ControlId control_id,
                             u16 left, u16 top, u16 right, u16 bottom,
                             FontId font_id, const char *text)
{
    RuntimeForm *form;
    RuntimeButton *button;
    char *text_copy;

    form = form_by_id(form_id);
    if (form == 0 || form->button_count >= RT_FORM_MAX_BUTTONS) {
        XTOS_LOG_PREFIX("[FORM]", "add_button_fail");
        return 0;
    }

    text_copy = copy_form_text(text);
    if (text_copy == 0) {
        XTOS_LOG_PREFIX("[FORM]", "add_button_text_fail");
        return 0;
    }

    button = &form->buttons[form->button_count];
    button->used = 1;
    button->dirty = 1;
    button->id = control_id;
    button->left = form_screen_x(left);
    button->top = form_screen_y(top);
    button->right = form_screen_x(right);
    button->bottom = form_screen_y(bottom);
    button->font_id = font_id;
    button->text = text_copy;
    button->focused = 0;
    button->pressed = 0;
    ++form->button_count;
    if (form->focused_type == RT_FORM_FOCUS_NONE) {
        set_focused_button(form, button);
    }
    XTOS_LOG_PREFIX_U16("[FORM]", "add_button", control_id);
    return 1;
}

u16 RuntimeFormAddList(FormId form_id, ControlId control_id,
                       u16 x, u16 y, u16 width, FontId font_id, u16 count)
{
    RuntimeForm *form;
    RuntimeList *list;
    u16 i;

    form = form_by_id(form_id);
    if (form == 0 || form->list_count >= RT_FORM_MAX_LISTS ||
        count == 0 || count > RT_FORM_MAX_LIST_ITEMS) {
        XTOS_LOG_PREFIX("[FORM]", "add_list_fail");
        return 0;
    }

    list = &form->lists[form->list_count];
    list->used = 1;
    list->dirty = 0;
    list->dirty_frame = 1;
    list->dirty_rows = 0;
    list->id = control_id;
    list->x = form_screen_x(x);
    list->y = form_screen_y(y);
    list->width = width;
    list->font_id = font_id;
    list->count = (u8)count;
    list->selected = 0;
    list->row_height = (u8)(RuntimeFontHeight(font_id) + 2);
    list->focused = 0;
    for (i = 0; i < RT_FORM_MAX_LIST_ITEMS; ++i) {
        list->items[i] = 0;
    }
    mark_list_dirty(list);
    ++form->list_count;

    if (form->focused_type == RT_FORM_FOCUS_NONE) {
        set_focused_list(form, list);
    }

    XTOS_LOG_PREFIX_U16("[FORM]", "add_list", control_id);
    return 1;
}

u16 RuntimeFormSetListItemText(FormId form_id, ControlId control_id,
                               u16 index, const char *text)
{
    RuntimeForm *form;
    RuntimeList *list;
    char *text_copy;

    form = form_by_id(form_id);
    list = list_by_id(form, control_id);
    if (list == 0 || index >= list->count) {
        XTOS_LOG_PREFIX("[FORM]", "set_list_item_fail");
        return 0;
    }

    text_copy = copy_form_text(text);
    if (text_copy == 0) {
        XTOS_LOG_PREFIX("[FORM]", "set_list_item_text_fail");
        return 0;
    }

    list->items[index] = text_copy;
    mark_list_row_dirty(list, (u8)index);
    return 1;
}

u16 RuntimeFormListSelected(FormId form_id, ControlId control_id,
                            u16 *selected)
{
    RuntimeForm *form;
    RuntimeList *list;

    form = form_by_id(form_id);
    list = list_by_id(form, control_id);
    if (list == 0 || selected == 0) {
        return 0;
    }

    *selected = list->selected;
    return 1;
}

u16 RuntimeFormListSetSelected(FormId form_id, ControlId control_id,
                               u16 selected)
{
    RuntimeForm *form;
    RuntimeList *list;

    form = form_by_id(form_id);
    list = list_by_id(form, control_id);
    if (list == 0 || selected >= list->count) {
        return 0;
    }

    return list_set_selected(list, (u8)selected);
}

u16 RuntimeFormSetLabelText(FormId form_id, ControlId control_id,
                            const char *text)
{
    RuntimeForm *form;
    RuntimeLabel *label;
    u16 old_width;
    u16 new_width;

    form = form_by_id(form_id);
    label = label_by_id(form, control_id);
    if (label == 0) {
        return 0;
    }

    old_width = label_text_width(label);
    if (!copy_label_text(label, text)) {
        return 0;
    }

    new_width = label_text_width(label);
    mark_label_dirty(label, old_width > new_width ? old_width : new_width);
    return 1;
}

u16 RuntimeFormInvalidate(FormId form_id)
{
    RuntimeForm *form;
    u8 i;

    form = form_by_id(form_id);
    if (form == 0) {
        return 0;
    }

    form->dirty_static = 1;
    for (i = 0; i < form->label_count; ++i) {
        mark_label_dirty(&form->labels[i],
                         label_text_width(&form->labels[i]));
    }
    for (i = 0; i < form->list_count; ++i) {
        mark_list_dirty(&form->lists[i]);
        form->lists[i].dirty_frame = 1;
    }
    for (i = 0; i < form->button_count; ++i) {
        mark_button_dirty(&form->buttons[i]);
    }
    return 1;
}

u16 RuntimeFormDraw(FormId form_id)
{
    RuntimeForm *form;
    u8 i;
    u8 drew;

    form = form_by_id(form_id);
    if (form == 0) {
        return 0;
    }

    drew = 0;

    if (form->dirty_static) {
        RuntimeCanvasClear();
        draw_static(form);
        form->dirty_static = 0;
        for (i = 0; i < form->label_count; ++i) {
            form->labels[i].dirty = 1;
        }
        for (i = 0; i < form->list_count; ++i) {
            mark_list_dirty(&form->lists[i]);
            form->lists[i].dirty_frame = 1;
        }
        for (i = 0; i < form->button_count; ++i) {
            form->buttons[i].dirty = 1;
        }
        drew = 1;
    }

    for (i = 0; i < form->label_count; ++i) {
        if (form->labels[i].dirty) {
            draw_label(&form->labels[i]);
            drew = 1;
        }
    }

    for (i = 0; i < form->list_count; ++i) {
        if (form->lists[i].dirty) {
            draw_list(&form->lists[i]);
            drew = 1;
        }
        if (form->lists[i].dirty_frame) {
            draw_list_boundary(&form->lists[i]);
            form->lists[i].dirty_frame = 0;
            drew = 1;
        }
    }

    for (i = 0; i < form->button_count; ++i) {
        if (form->buttons[i].dirty) {
            draw_button(&form->buttons[i]);
            drew = 1;
        }
    }

    if (drew) {
        RuntimeCanvasPresent();
    }
    return 1;
}

u16 RuntimeFormDispatchFar(FormId form_id, const Event XTOS_FAR *event,
                           FormAction XTOS_FAR *action)
{
    RuntimeForm *form;
    RuntimeButton *button;
    RuntimeList *list;
    Event event_copy;
    u16 changed;
    u8 row;

    clear_action(action);
    form = form_by_id(form_id);
    if (form == 0 || event == 0 || action == 0) {
        return 0;
    }

    event_copy = *event;
    changed = 0;

    if (event_copy.type == EVENT_KEYDOWN && event_copy.key == XTOS_KEY_TAB) {
        return focus_next_control(form);
    }

    if (event_copy.type == EVENT_KEYDOWN &&
        form->focused_type == RT_FORM_FOCUS_LIST &&
        form->focused_list < form->list_count) {
        list = &form->lists[form->focused_list];
        if (event_copy.key == XTOS_KEY_UP && list->selected > 0) {
            list_set_selected(list, (u8)(list->selected - 1));
            set_action(action, RT_FORM_ACTION_LIST_CHANGED, form->id,
                       list->id, list->selected);
            return 1;
        }
        if (event_copy.key == XTOS_KEY_DOWN &&
            (u8)(list->selected + 1) < list->count) {
            list_set_selected(list, (u8)(list->selected + 1));
            set_action(action, RT_FORM_ACTION_LIST_CHANGED, form->id,
                       list->id, list->selected);
            return 1;
        }
    }

    if (event_copy.type == EVENT_KEYDOWN &&
        form->focused_type == RT_FORM_FOCUS_BUTTON &&
        (event_copy.key == XTOS_KEY_ENTER || event_copy.key == ' ') &&
        form->button_count > 0) {
        button = &form->buttons[form->focused_button];
        set_action(action, RT_FORM_ACTION_BUTTON, form->id, button->id, 0);
        return 1;
    }

    if (event_copy.type == EVENT_MOUSE_DOWN) {
        if (point_in_close_box(event_copy.x, event_copy.y)) {
            set_action(action, RT_FORM_ACTION_CLOSE, form->id, 0, 0);
            return 1;
        }

        list = hit_list(form, event_copy.x, event_copy.y);
        if (list != 0) {
            changed = set_focused_list(form, list);
            row = list_row_at(list, event_copy.y);
            if (row < list->count && list_set_selected(list, row)) {
                set_action(action, RT_FORM_ACTION_LIST_CHANGED, form->id,
                           list->id, list->selected);
                changed = 1;
            }
            return changed;
        }

        button = hit_button(form, event_copy.x, event_copy.y);
        if (button != 0) {
            changed = set_focused_button(form, button);
            if (!button->pressed) {
                button->pressed = 1;
                mark_button_dirty(button);
                changed = 1;
            }
            form->pressed_button = (u8)(button - form->buttons);
        }
        return changed;
    }

    if (event_copy.type == EVENT_MOUSE_UP) {
        if (form->pressed_button < form->button_count) {
            button = &form->buttons[form->pressed_button];
            form->pressed_button = 0xff;
            if (button->pressed) {
                button->pressed = 0;
                mark_button_dirty(button);
                changed = 1;
            }
            if (point_in_button(button, event_copy.x, event_copy.y)) {
                set_action(action, RT_FORM_ACTION_BUTTON, form->id,
                           button->id, 0);
                changed = 1;
            }
        }
        return changed;
    }

    return 0;
}
