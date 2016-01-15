#include "sectiontable.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <assert.h>

void section_table_init(struct section_table_t* table)
{
    for (int i = 0; i < SECTION_COUNT; ++i)
    {
        snprintf(table->sections[i].label, 8,
                 "%03d-%03d",
                 i * SECTION_LENGTH,
                 (i + 1) * SECTION_LENGTH - 1);
    }

    readSectionRecords(table, "GoldST.txt");

    resetSectionTable(table);
}

void section_table_terminate(struct section_table_t* table)
{
    writeSectionRecords(table);
}

struct section_table_t* section_table_create()
{
    struct section_table_t* s = malloc(sizeof(struct section_table_t));
    if (s)
        section_table_init(s);

    return s;
}

void section_table_destroy(struct section_table_t* table)
{
    section_table_terminate(table);
    free(table);
}

void resetSectionTable(struct section_table_t* table)
{
    for (size_t i = 0; i < SECTION_COUNT; ++i)
    {
        struct section_t* section = &table->sections[i];

        section->size = 0;
        section->startTime = 0;
        section->endTime = 0;

        for (int j = 0; j < 4; ++j)
            section->lines[j] = 0;
    }

    // Add an initial data point to the first section section
    table->sections[0].data[0] = (struct datapoint_t){ 0, 0 };
    table->sections[0].size++;
}

void updateSectionTable(struct section_table_t* table, struct game_t* game)
{
    assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT);

    struct section_t* section = &table->sections[game->currentSection];

    // Special case for when we're at the end of the game (level 999). The
    // in-game timer seems to reset back to 00:00:00 on the same exact frame
    // that level 999 is reached. So when we're adding our data point, we must
    // use the previous frame's timer value.
    if (game->curState.level >= LEVEL_MAX)
    {
        if (section->endTime == 0)
        {
            section->endTime = game->prevState.timer;

            int tempTime = game->curState.timer;
            game->curState.timer = game->prevState.timer;

            addDataPointToSection(section, game);

            // For consistency, restore original time value.
            game->curState.timer = tempTime;
        }

        return;
    }

    // If we've passed this section's boundary, add our last point and move to
    // the next section.
    while (game->curState.level >= (game->currentSection + 1) * SECTION_LENGTH)
    {
        section->endTime = game->curState.timer;
        addDataPointToSection(section, game);

        // Section advance!
        game->currentSection++;
        section = &table->sections[game->currentSection];

        assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT);
    }

    addDataPointToSection(section, game);
}

void addDataPointToSection(struct section_t* section, struct game_t* game)
{
    assert(section->size < SECTION_MAX - 1);

    // Section just began
    if (section->size == 0)
    {
        section->startTime = game->curState.timer;

        // Push datapoint to the end of the section.
        section->data[section->size] = (struct datapoint_t) { game->curState.level, game->curState.timer };
        section->size++;
    }
    else
    {
        // Only push the data point if level has been incremented.
        int levelDifference = levelDifference = game->curState.level - section->data[section->size - 1].level;

        if (levelDifference > 0)
        {
            if (levelDifference > 4)
            {
                fprintf(stderr,
                        "Abnormally large level jump: %d -> %d\n",
                        section->data[section->size - 1].level,
                        game->curState.level);
            }

            // Push datapoint to the end of the section.
            section->data[section->size] = (struct datapoint_t) { game->curState.level, game->curState.timer };
            section->size++;

            // Check if we scored some phat lines.
            if (section->size >= 2 &&
                (game->prevState.state == TAP_LOCKING || game->curState.state == TAP_LINECLEAR))
            {
                section->lines[levelDifference - 1]++;
            }
        }
    }

    assert(section->size <= SECTION_MAX);
}

void updateRecords(struct section_table_t* table, unsigned int currentLevel, unsigned int gameMode)
{
    for (size_t i = 0; i < SECTION_COUNT; ++i)
    {
        // For PBs, we have to check if the player actually completed the section.
        bool gameComplete = (currentLevel == LEVEL_MAX);
        bool sectionComplete = gameComplete || (currentLevel > (i + 1) * SECTION_LENGTH);

        struct section_t* section = &table->sections[i];
        int sectionTime = section->endTime - section->startTime;

        if (gameMode == TAP_MODE_MASTER)
        {
            // Update section PB
            if (sectionComplete &&
                sectionTime > 0 &&
                section->masterST > sectionTime)
            {
                section->masterST = sectionTime;
            }

            // Update overall time PB
            if (gameComplete &&
                section->endTime > 0 &&
                section->masterTime > section->endTime)
            {
                section->masterTime = section->endTime;
            }
        }
        else if (gameMode == TAP_MODE_DEATH)
        {
            // Update section PB
            if (sectionComplete &&
                sectionTime > 0 &&
                section->deathST > sectionTime)
            {
                section->deathST = sectionTime;
            }

            // Update overall time PB
            if (gameComplete &&
                section->endTime > 0 &&
                section->deathTime > section->endTime)
            {
                section->deathTime = section->endTime;
            }
        }
    }
}

void readSectionRecords(struct section_table_t* table, const char* filename)
{
    assert(table != NULL);

    FILE* pbfile = fopen(filename, "r");

    // If file was found, load section times!
    if (pbfile)
    {
        for (size_t i = 0; i < SECTION_COUNT; ++i)
        {
            struct section_t* section = &table->sections[i];

            fscanf(pbfile, "%6d %6d %6d %6d\n",
                   &section->masterTime,
                   &section->masterST,
                   &section->deathTime,
                   &section->deathST);
        }

        fclose(pbfile);
    }
    else // Otherwise set the defaults
    {
        for (size_t i = 0; i < SECTION_COUNT; ++i)
        {
            struct section_t* section = &table->sections[i];

            section->masterST = 3900; // 01:05:00
            section->deathST = 2520;  // 00:42:00

            section->masterTime = 3900 * (i + 1); // 01:05:00
            section->deathTime = 2520 * (i + 1);  // 00:42:00
        }
    }
}

void writeSectionRecords(struct section_table_t* table)
{
    assert(table != NULL);

    // Write PB file
    FILE* pbfile = fopen("GoldST.txt", "w");

    if (pbfile)
    {
        for (size_t i = 0; i < SECTION_COUNT; ++i)
        {
            fprintf(pbfile, "%06d %06d %06d %06d\n",
                    table->sections[i].masterTime,
                    table->sections[i].masterST,
                    table->sections[i].deathTime,
                    table->sections[i].deathST);
        }

        fclose(pbfile);
    }
}
