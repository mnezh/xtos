#include "../runtime/client.h"
#include "../xtos/abi.h"

#define RUNTIME_EXE "runtime.exe"
#define LAUNCHER_EXE "launcher.exe"
#define EXEC_PATH_SIZE 64
#define MAX_LAUNCHES 64

int XtosDosExec(const char *path);
void XtosDosPrintLine(const char *message);
void XtosDosLogLine(const char *message);
void XtosDosLogLine2(const char *prefix, const char *value);

static void boot_log(const char *message)
{
    XtosDosLogLine(message);
}

static void boot_log_path(const char *prefix, const char *path)
{
    XtosDosLogLine2(prefix, path);
}

static void copy_path(char *dest, const char *source)
{
    unsigned int i;

    for (i = 0; i + 1 < EXEC_PATH_SIZE && source[i] != 0; ++i) {
        dest[i] = source[i];
    }

    dest[i] = 0;
}

static int is_launcher(const char *path)
{
    const char *launcher;

    launcher = LAUNCHER_EXE;

    while (*path != 0 && *launcher != 0) {
        if (*path != *launcher) {
            return 0;
        }
        ++path;
        ++launcher;
    }

    return *path == 0 && *launcher == 0;
}

int main(int argc, char **argv)
{
    int rc;
    char current_app[EXEC_PATH_SIZE];
    char next_app[EXEC_PATH_SIZE];
    int launcher_mode;
    unsigned int launch_count;
    u16 uninstall_reason;

    boot_log("[BOOT] supervisor start");
    boot_log("[BOOT] runtime install requested");
    XtosDosExec(RUNTIME_EXE);

    if (!XtosRuntimePresent()) {
        XtosDosPrintLine("XTOS runtime failed to install");
        boot_log("runtime install failed");
        return 1;
    }

    boot_log("[BOOT] runtime ready");
    launcher_mode = argc < 2 || argv[1] == 0;
    copy_path(current_app, launcher_mode ? LAUNCHER_EXE : argv[1]);
    rc = 0;
    launch_count = 0;

    do {
        if (launch_count >= MAX_LAUNCHES) {
            boot_log("[BOOT] launch guard exceeded");
            rc = 1;
            break;
        }

        ++launch_count;
        XtosExecClearNextApp();
        boot_log_path(is_launcher(current_app) ?
                      "[BOOT] launcher start" : "[BOOT] app start",
                      current_app);
        rc = XtosDosExec(current_app);
        boot_log_path(is_launcher(current_app) ?
                      "[BOOT] launcher exit" : "[BOOT] app exit",
                      current_app);

        if (!launcher_mode) {
            break;
        }

        if (XtosExecNextApp(next_app, sizeof(next_app))) {
            XtosExecClearNextApp();
            copy_path(current_app, next_app);
        } else if (!is_launcher(current_app)) {
            boot_log("[BOOT] relaunch launcher");
            copy_path(current_app, LAUNCHER_EXE);
        } else {
            boot_log("[BOOT] no exec request, exiting XTOS");
            break;
        }
    } while (1);

    boot_log("[RT] restore text mode");
    XtosRuntimeRestoreTextMode();
    boot_log("[RT] uninstall requested");
    uninstall_reason = 0;
    if (XtosRuntimeUninstall(&uninstall_reason)) {
        boot_log("[RT] uninstall ok");
    } else {
        boot_log("[RT] uninstall failed");
        if (uninstall_reason == XTOS_RESULT_UNSUPPORTED) {
            boot_log("[RT] uninstall failed reason=memory-release-unsupported");
        } else if (uninstall_reason == XTOS_RESULT_BUSY) {
            boot_log("[RT] uninstall failed reason=vector-owned-by-other");
        } else {
            boot_log("[RT] uninstall failed reason=unknown");
        }
    }
    boot_log("[BOOT] supervisor exit");

    return rc;
}
