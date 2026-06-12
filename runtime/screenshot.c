#include <stdio.h>

#include "log.h"
#include "screenshot.h"

#define CGA_DUMP_FILE "XTOS.CGA"
#define CGA_DUMP_SIZE 16384

int RuntimeScreenshotCga(const char *path)
{
    FILE *file;
    u16 offset;

    if (path == 0) {
        path = CGA_DUMP_FILE;
    }

    file = fopen(path, "wb");
    if (file == 0) {
        return 0;
    }

    for (offset = 0; offset < CGA_DUMP_SIZE; ++offset) {
        fputc(RuntimeCgaReadByte(offset), file);
    }

    fclose(file);
    XTOS_LOG("screenshot_cga");
    return 1;
}
