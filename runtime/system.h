#ifndef RUNTIME_SYSTEM_H
#define RUNTIME_SYSTEM_H

#include "../xtos/system.h"

int RuntimeSystemPrefsLoad(SystemPrefs *prefs);
int RuntimeSystemPrefsSave(const SystemPrefs *prefs);
const SystemPrefs *RuntimeSystemPrefsCurrent(void);
void RuntimeSystemPrefsApply(const SystemPrefs *prefs);

#endif
