#ifndef XTOS_APP_H
#define XTOS_APP_H

#include "event.h"

typedef struct Application {
    void (*Init)(void);
    void (*HandleEvent)(Event *event);
    void (*Draw)(void);
    void (*Shutdown)(void);
} Application;

int AppRun(Application *app);
void AppQuit(void);

#endif
