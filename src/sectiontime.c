#include "sectiontime.h"

#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

void section_table_init(struct section_table_t* s)
{
    FILE* pbfile = fopen("GoldST.txt", "r");

    // If file was found, load section times!
    if (pbfile)
    {
        for (size_t i = 0; i < SECTION_COUNT; ++i)
        {
            char str[16];

            // Read master pb
            if (fgets(str, sizeof(str), pbfile) != NULL)
                s->times[i].masterPB = atoi(str);

            // Read death pb
            if (fgets(str, sizeof(str), pbfile) != NULL)
                s->times[i].deathPB = atoi(str);

            s->times[i].current = 0;
        }

        fclose(pbfile);
    }
    else // Otherwise set the defaults
    {
        for (size_t i = 0; i < SECTION_COUNT; ++i)
        {
            s->times[i].masterPB = 3900; // 01:05:00
            s->times[i].deathPB = 2520;  // 00:42:00
            s->times[i].current = 0;
        }
    }
}

void section_table_terminate(struct section_table_t* s)
{
    printf("Writing Section PBs to GoldST.txt.");

    // Write PB file
    FILE* pbfile = fopen("GoldST.txt", "w");

    if (pbfile)
    {
        for (size_t i = 0; i < SECTION_COUNT; ++i)
        {
            fprintf(pbfile, "%d\n", s->times[i].masterPB);
            fprintf(pbfile, "%d\n", s->times[i].deathPB);
        }

        fclose(pbfile);
    }
}

struct section_table_t* section_table_create()
{
    struct section_table_t* s = (struct section_table_t*)malloc(sizeof(struct section_table_t));
    if (s)
    {
        section_table_init(s);
    }

    return s;
}

void section_table_destroy(struct section_table_t* s)
{
    section_table_terminate(s);
    free(s);
}

void setSectionTableTime(struct section_table_t* s,
                        unsigned int sectionIndex, int length)
{
    assert(sectionIndex < SECTION_COUNT);

    s->times[sectionIndex].current = length;
}

void updateSectionTime(struct section_table_t* s, unsigned int gameMode)
{
    for (size_t i = 0; i < SECTION_COUNT; ++i)
    {
        struct section_time_t* sectionTime = &s->times[i];
        if (gameMode == TAP_MODE_MASTER)
        {
            if (sectionTime->current> 0 &&
                sectionTime->masterPB > sectionTime->current)
                sectionTime->masterPB = sectionTime->current;
        }
        else if (gameMode == TAP_MODE_DEATH)
        {
            if (sectionTime->current> 0 &&
                sectionTime->deathPB > sectionTime->current)
                sectionTime->deathPB = sectionTime->current;
        }
    }
}
