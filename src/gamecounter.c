#include "gamecounter.h"

#include <stdlib.h>

void gamecounter_t_init(struct gamecounter_t* gc)
{
}

void gamecounter_t_terminate(struct gamecounter_t* gc)
{
}

struct gamecounter_t* gamecounter_t_create()
{
    struct gamecounter_t* gc = malloc(sizeof(struct gamecounter_t));
    if (gc)
    {
        gamecounter_t_init(gc);
    }
    
    return gc;
}

void gamecounter_t_destroy(struct gamecounter_t* gc)
{
    gamecounter_t_terminate(gc);
    free(gc);
}
