#include "log.h"

#ifdef XTOS_DEBUG
#include <stdio.h>

#define XTOS_LOG_FILE "XTOS.LOG"

void XtosLogPrefix(const char *prefix, const char *message)
{
    FILE *file;

    file = fopen(XTOS_LOG_FILE, "a");
    if (file == 0) {
        return;
    }

    fprintf(file, "%s %s\n", prefix, message);
    fclose(file);
}

void XtosLogPrefixU16(const char *prefix, const char *label, u16 value)
{
    FILE *file;

    file = fopen(XTOS_LOG_FILE, "a");
    if (file == 0) {
        return;
    }

    fprintf(file, "%s %s=%u\n", prefix, label, (unsigned int)value);
    fclose(file);
}
#endif
