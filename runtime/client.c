#include "client.h"
#include "int60.h"

int XtosRuntimePing(void)
{
    XtosPb pb;
    u16 int_out[2];

    if (!XtosInt60VectorPresent()) {
        return 0;
    }

    int_out[0] = 0;
    int_out[1] = 0;
    pb.opcode = XTOS_OP_PING;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    XtosInt60Call(&pb);

    return pb.result == XTOS_RESULT_OK &&
           int_out[0] == XTOS_RUNTIME_MAGIC &&
           int_out[1] == XTOS_ABI_VERSION;
}

int XtosRuntimePresent(void)
{
    return XtosRuntimePing();
}

int XtosRuntimeStatus(u16 *magic, u16 *version)
{
    XtosPb pb;
    u16 int_out[2];

    if (!XtosInt60VectorPresent()) {
        return 0;
    }

    int_out[0] = 0;
    int_out[1] = 0;
    pb.opcode = XTOS_OP_RUNTIME_STATUS;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    XtosInt60Call(&pb);

    if (magic != 0) {
        *magic = int_out[0];
    }

    if (version != 0) {
        *version = int_out[1];
    }

    return pb.result == XTOS_RESULT_OK &&
           int_out[0] == XTOS_RUNTIME_MAGIC &&
           int_out[1] == XTOS_ABI_VERSION;
}

int XtosRuntimeSelfTest(u16 *magic, u16 *version, u16 *status)
{
    XtosPb pb;
    u16 int_out[3];

    if (!XtosInt60VectorPresent()) {
        return 0;
    }

    int_out[0] = 0;
    int_out[1] = 0;
    int_out[2] = 0;
    pb.opcode = XTOS_OP_SELFTEST;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    XtosInt60Call(&pb);

    if (magic != 0) {
        *magic = int_out[0];
    }

    if (version != 0) {
        *version = int_out[1];
    }

    if (status != 0) {
        *status = int_out[2];
    }

    return pb.result == XTOS_RESULT_OK &&
           int_out[0] == XTOS_RUNTIME_MAGIC &&
           int_out[1] == XTOS_ABI_VERSION &&
           int_out[2] == XTOS_RUNTIME_STATUS_READY;
}

void XtosRuntimeLog(const char *message)
{
    XtosPb pb;

    if (message == 0 || !XtosInt60VectorPresent()) {
        return;
    }

    pb.opcode = XTOS_OP_LOG;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = (void XTOS_FAR *)message;
    pb.addr_out = 0;
    XtosInt60Call(&pb);
}

void XtosRuntimeRestoreTextMode(void)
{
    XtosPb pb;

    if (!XtosInt60VectorPresent()) {
        return;
    }

    pb.opcode = XTOS_OP_RESTORE_TEXT_MODE;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    XtosInt60Call(&pb);
}

int XtosRuntimeUninstall(u16 *reason)
{
    XtosPb pb;
    u16 int_out[1];

    if (!XtosInt60VectorPresent()) {
        if (reason != 0) {
            *reason = XTOS_RESULT_BUSY;
        }
        return 0;
    }

    int_out[0] = 0;
    pb.opcode = XTOS_OP_UNINSTALL;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = 0;
    XtosInt60Call(&pb);

    if (reason != 0) {
        *reason = int_out[0];
    }

    return pb.result == XTOS_RESULT_OK;
}

int XtosExecNextApp(char *path, u16 path_size)
{
    XtosPb pb;
    u16 int_in[1];
    u16 int_out[1];

    if (path == 0 || path_size == 0 || !XtosInt60VectorPresent()) {
        return 0;
    }

    path[0] = 0;
    int_in[0] = path_size;
    int_out[0] = 0;
    pb.opcode = XTOS_OP_EXEC_GET_NEXT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = int_in;
    pb.int_out = int_out;
    pb.addr_in = 0;
    pb.addr_out = (void XTOS_FAR *)path;
    XtosInt60Call(&pb);

    return pb.result == XTOS_RESULT_OK && int_out[0] != 0;
}

void XtosExecClearNextApp(void)
{
    XtosPb pb;

    if (!XtosInt60VectorPresent()) {
        return;
    }

    pb.opcode = XTOS_OP_EXEC_CLEAR_NEXT;
    pb.result = XTOS_RESULT_OK;
    pb.int_in = 0;
    pb.int_out = 0;
    pb.addr_in = 0;
    pb.addr_out = 0;
    XtosInt60Call(&pb);
}
