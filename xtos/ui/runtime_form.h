#ifndef XTOS_UI_RUNTIME_FORM_H
#define XTOS_UI_RUNTIME_FORM_H

#include "../event.h"
#include "../types.h"
#include "canvas.h"
#include "font.h"

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

#endif
