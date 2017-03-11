#include <stdlib.h>
#include <stdio.h>

#include <zf_log.h>

#include "tracker.h"
#include "memorymap.h"

int main(int argc, const char *argv[])
{
    struct tap_state* addr = getMappingPtr();

    if (!runTracker(addr, argc, argv))
    {
        ZF_LOGF("Error running TapTracker.");
    }

    freeMappingPtr(addr);

    return 0;
}
