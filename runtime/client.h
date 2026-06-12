#ifndef RUNTIME_CLIENT_H
#define RUNTIME_CLIENT_H

#include "../xtos/types.h"

int XtosRuntimePresent(void);
int XtosRuntimePing(void);
int XtosRuntimeStatus(u16 *magic, u16 *version);
int XtosRuntimeSelfTest(u16 *magic, u16 *version, u16 *status);
void XtosRuntimeLog(const char *message);
void XtosRuntimeRestoreTextMode(void);

#endif
