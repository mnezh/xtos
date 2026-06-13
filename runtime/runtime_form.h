#ifndef RUNTIME_RUNTIME_FORM_H
#define RUNTIME_RUNTIME_FORM_H

#include "../xtos/abi.h"
#include "../xtos/event.h"
#include "../xtos/ui/runtime_form.h"

FormId RuntimeFormCreateText(FontId font_id, const char *title);
u16 RuntimeFormDestroy(FormId form_id);
u16 RuntimeFormAddLabelText(FormId form_id, ControlId control_id,
                            u16 x, u16 y, FontId font_id, u16 color,
                            const char *text);
u16 RuntimeFormAddButtonText(FormId form_id, ControlId control_id,
                             u16 left, u16 top, u16 right, u16 bottom,
                             FontId font_id, const char *text);
u16 RuntimeFormAddList(FormId form_id, ControlId control_id,
                       u16 x, u16 y, u16 width, FontId font_id, u16 count);
u16 RuntimeFormSetListItemText(FormId form_id, ControlId control_id,
                               u16 index, const char *text);
u16 RuntimeFormListSelected(FormId form_id, ControlId control_id,
                            u16 *selected);
u16 RuntimeFormListSetSelected(FormId form_id, ControlId control_id,
                               u16 selected);
u16 RuntimeFormSetLabelText(FormId form_id, ControlId control_id,
                            const char *text);
u16 RuntimeFormInvalidate(FormId form_id);
u16 RuntimeFormDraw(FormId form_id);
u16 RuntimeFormDispatchFar(FormId form_id, const Event XTOS_FAR *event,
                           FormAction XTOS_FAR *action);

#endif
