#ifndef RUNTIME_LOG_H
#define RUNTIME_LOG_H

#include "../xtos/types.h"

#ifdef XTOS_DEBUG
void XtosLogPrefix(const char *prefix, const char *message);
void XtosLogPrefixU16(const char *prefix, const char *label, u16 value);
#define XTOS_LOG(message) XtosLogPrefix("[RT]", (message))
#define XTOS_LOG_U16(label, value) XtosLogPrefixU16("[RT]", (label), (value))
#define XTOS_LOG_PREFIX(prefix, message) XtosLogPrefix((prefix), (message))
#define XTOS_LOG_PREFIX_U16(prefix, label, value) \
    XtosLogPrefixU16((prefix), (label), (value))
#else
#define XTOS_LOG(message) ((void)0)
#define XTOS_LOG_U16(label, value) ((void)0)
#define XTOS_LOG_PREFIX(prefix, message) ((void)0)
#define XTOS_LOG_PREFIX_U16(prefix, label, value) ((void)0)
#endif

#endif
