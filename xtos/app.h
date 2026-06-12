#ifndef XTOS_APP_H
#define XTOS_APP_H

#include "event.h"

typedef struct Application {
    void (*Init)(void);
    void (*HandleEvent)(Event *event);
    void (*Draw)(void);
    void (*Shutdown)(void);
    const char *first_draw_screenshot_path;
} Application;

int AppRun(Application *app);
void AppQuit(void);

#endif
