#ifndef RUNTIME_INT60_H
#define RUNTIME_INT60_H

#include "../xtos/abi.h"

void XtosInt60Install(void);
void XtosInt60UseResidentDataSegment(void);
void XtosInt60Restore(void);
void XtosInt60RestoreDataSegment(void);
u16 XtosInt60VectorPresent(void);
u16 XtosInt60Call(XtosPb XTOS_FAR *pb);
u16 XtosInt60CallResident(XtosPb XTOS_FAR *pb);
u16 XtosInt60Dispatch(XtosPb XTOS_FAR *pb);

#endif
