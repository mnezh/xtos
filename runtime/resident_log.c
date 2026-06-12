#include "log.h"
#include "resident_dos.h"

#ifdef XTOS_DEBUG
#define XTOS_LOG_FILE "XTOS.LOG"

static const char newline[] = "\r\n";
static const char space[] = " ";
static char log_digits[6];
static char log_line[32];

static u16 str_len(const char *text)
{
    u16 len;

    len = 0;
    while (text[len] != 0) {
        ++len;
    }

    return len;
}

void XtosLogPrefix(const char *prefix, const char *message)
{
    int handle;

    handle = ResidentDosOpenAppend(XTOS_LOG_FILE);
    if (handle < 0) {
        handle = ResidentDosCreate(XTOS_LOG_FILE);
    }

    if (handle < 0) {
        return;
    }

    ResidentDosWrite((u16)handle, prefix, str_len(prefix));
    ResidentDosWrite((u16)handle, space, 1);
    ResidentDosWrite((u16)handle, message, str_len(message));
    ResidentDosWrite((u16)handle, newline, 2);
    ResidentDosClose((u16)handle);
}

void XtosLogPrefixU16(const char *prefix, const char *label, u16 value)
{
    u16 i;
    u16 out;
    u16 pos;

    i = 0;
    do {
        log_digits[i++] = (char)('0' + (value % 10));
        value = (u16)(value / 10);
    } while (value != 0 && i < sizeof(log_digits));

    pos = 0;
    while (*label != 0 && pos < sizeof(log_line) - 1) {
        log_line[pos++] = *label++;
    }
    if (pos < sizeof(log_line) - 1) {
        log_line[pos++] = '=';
    }
    while (i != 0 && pos < sizeof(log_line) - 1) {
        out = (u16)(i - 1);
        log_line[pos++] = log_digits[out];
        --i;
    }
    log_line[pos] = 0;

    XtosLogPrefix(prefix, log_line);
}
#endif
