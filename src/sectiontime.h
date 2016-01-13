#ifndef SECTIONTIME_H
#define SECTIONTIME_H

#include "game.h"

struct section_time_t
{
        int masterPB;
        int deathPB;
        int current; // Current section time (for completed sections only!)
};

struct section_table_t
{
        struct section_time_t times[SECTION_COUNT];
};

void section_table_init(struct section_table_t* s);
void section_table_terminate(struct section_table_t* s);

struct section_table_t* section_table_create();
void section_table_destroy(struct section_table_t* s);

void setSectionTableTime(struct section_table_t* s,
                         unsigned int sectionIndex, int length);

// Once the game is over, call this to update the PB table.
void updateSectionTime(struct section_table_t* s, unsigned int gameMode);

#endif /* SECTIONTIME_H */
