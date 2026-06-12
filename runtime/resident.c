#include <stdio.h>

#include "int60.h"
#include "log.h"
#include "client.h"

void XtosDosStayResident(u16 paragraphs);

int main(void)
{
    XtosInt60UseResidentDataSegment();
    XtosInt60Install();
    XTOS_LOG_PREFIX("[RT]", "resident_install");

    /* Keep the first 64 KiB allocation resident for the Phase 1A TSR. */
    XtosDosStayResident(0x1000);
    return 0;
}
