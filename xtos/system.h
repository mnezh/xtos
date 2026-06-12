#ifndef XTOS_SYSTEM_H
#define XTOS_SYSTEM_H

#include "display.h"

typedef struct SystemPrefs {
    enum DisplayMode mode;
    enum DisplayPalette palette;
} SystemPrefs;

int SystemPrefsLoad(SystemPrefs *prefs);
int SystemPrefsSave(const SystemPrefs *prefs);
const SystemPrefs *SystemPrefsCurrent(void);
void SystemPrefsApply(const SystemPrefs *prefs);
int XtosScreenshotCga(const char *path);

#endif
