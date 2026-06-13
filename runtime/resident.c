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

    /* Keep enough paragraphs resident for the growing Phase 1B runtime. */
    XtosDosStayResident(0x1400);
    return 0;
}
