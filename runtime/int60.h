#ifndef RUNTIME_INT60_H
#define RUNTIME_INT60_H

#include "../xtos/abi.h"

void XtosInt60Install(void);
void XtosInt60Restore(void);
u16 XtosInt60Call(XtosPb XTOS_FAR *pb);
u16 XtosInt60Dispatch(XtosPb XTOS_FAR *pb);

#endif
