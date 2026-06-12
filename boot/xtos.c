#include <stdio.h>

#include "../runtime/client.h"

#define RUNTIME_EXE "runtime.exe"

int XtosDosExec(const char *path);

static void boot_log(const char *message)
{
    (void)message;
}

int main(int argc, char **argv)
{
    int rc;

    if (argc < 2 || argv[1] == 0) {
        puts("Usage: XTOS.COM APP.EXE");
        return 1;
    }

    boot_log("xtos.com starting runtime.exe");
    XtosDosExec(RUNTIME_EXE);

    if (!XtosRuntimePresent()) {
        puts("XTOS runtime failed to install");
        boot_log("runtime install failed");
        return 1;
    }

    boot_log("launching app");
    XtosRuntimeLog("xtos.com verified runtime");
    rc = XtosDosExec(argv[1]);
    boot_log("app exited");

    boot_log("restoring text mode");
    XtosRuntimeRestoreTextMode();
    boot_log("xtos.com exiting");

    return rc;
}
