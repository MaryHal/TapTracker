#include "sectiontable.h"

#include "game.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <assert.h>

void setDefaultPBTimes(struct pb_table_t* pb)
{
    for (int i = 0; i < SECTION_COUNT; ++i)
    {
        pb->gameTime[i] = 3900 * (i + 1); // 01:05:00 for every section
        pb->goldST[i] = 3900;             // 01:05:00
    }
}

struct pb_table_t* _addPBTable(struct pb_table_t** map, int gameMode)
{
    struct pb_table_t* pb = _getPBTable(map, gameMode);

    if (pb == NULL)
    {
        pb = malloc(sizeof(struct pb_table_t));
        pb->gameMode = gameMode;

        setDefaultPBTimes(pb);

        printf("Adding pb table: %d\n", gameMode);

        HASH_ADD_INT(*map, gameMode, pb);
    }

    return pb;
}

void _deletePBTable(struct pb_table_t** map, int gameMode)
{
    struct pb_table_t* pb = _getPBTable(map, gameMode);

    if (pb != NULL)
    {
        HASH_DEL(*map, pb);
        free(pb);
    }
}

struct pb_table_t* _getPBTable(struct pb_table_t** map, int gameMode)
{
    struct pb_table_t* pb = NULL;

    HASH_FIND_INT(*map, &gameMode, pb); /* pb: output */
    return pb;
}

void section_table_init(struct section_table_t* table)
{
    for (int i = 0; i < SECTION_COUNT; ++i)
    {
        snprintf(table->sections[i].label, 8,
                 "%03d-%03d",
                 i * SECTION_LENGTH,
                 (i + 1) * SECTION_LENGTH - 1);
    }

    table->pbHash = NULL;

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

    struct pb_table_t* current = NULL;
    struct pb_table_t* tmp = NULL;
    HASH_ITER(hh, table->pbHash, current, tmp)
    {
        HASH_DEL(table->pbHash, current);
        free(current);
    }

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

    // Add an initial data point to the first section
    table->sections[0].data[0] = (struct datapoint_t){ 0, 0 };
    table->sections[0].size++;
}

void updateSectionTable(struct section_table_t* table, struct game_t* game)
{
    assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT);

    int gameMode = game->curState.gameMode;
    /* bool endOfGame = (gameMode == TAP_MODE_NORMAL || gameMode == TAP_MODE_NORMAL_VERSUS || */
    /*                   gameMode == TAP_MODE_ */

    struct section_t* section = &table->sections[game->currentSection];
    struct pb_table_t* pb     = _getPBTable(&table->pbHash, gameMode);

    // Special case for when we're at the end of the game (level 999). The
    // in-game timer seems to reset back to 00:00:00 on the same exact frame
    // that level 999 is reached. So when we're adding our data point, we must
    // use the previous frame's timer value.
    if (game->curState.level >= getGameEndLevel(gameMode))
    {
        // On the first frame reaching the end of the game
        if (section->endTime == 0)
        {
            section->endTime = game->prevState.timer;

            int tempTime = game->curState.timer;
            game->curState.timer = game->prevState.timer;

            addDataPointToSection(section, game);

            // For consistency, restore original time value.
            game->curState.timer = tempTime;

            // Update final section gold ST
            if (pb->goldST[game->currentSection] > section->endTime - section->startTime)
            {
                pb->goldST[game->currentSection] = section->endTime - section->startTime;
            }

            // Update all game time PBs if necessary
            if (pb->gameTime[game->currentSection] > table->sections[game->currentSection].endTime)
            {
                for (int i = 0; i <= game->currentSection; ++i)
                    pb->gameTime[i] = table->sections[i].endTime;
            }
        }

        return;
    }

    // If we've passed this section's boundary, add our last point and move to
    // the next section.
    while (game->curState.level >= (game->currentSection + 1) * SECTION_LENGTH)
    {
        section->endTime = game->curState.timer;
        addDataPointToSection(section, game);

        // Update this section's gold ST
        if (pb->goldST[game->currentSection] > section->endTime - section->startTime)
        {
            pb->goldST[game->currentSection] = section->endTime - section->startTime;
        }

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

void readSectionRecords(struct section_table_t* table, const char* filename)
{
    assert(table != NULL);

    FILE* pbfile = fopen(filename, "r");

    // If file was found, load section times!
    if (pbfile)
    {
        int mode = 0;
        while (fscanf(pbfile, "%d", &mode) == 1)
        {
            struct pb_table_t* pb = _addPBTable(&table->pbHash, mode);

            for (size_t i = 0; i < SECTION_COUNT; ++i)
            {
                fscanf(pbfile, "%6d %6d\n",
                       &pb->gameTime[i],
                       &pb->goldST[i]);
            }
        }

        fclose(pbfile);
    }
}

void writeSectionRecords(struct section_table_t* table)
{
    assert(table != NULL);

    // Write PB file
    FILE* pbfile = fopen("GoldST.txt", "w");

    if (pbfile)
    {
        for (struct pb_table_t* pb = table->pbHash; pb != NULL; pb = pb->hh.next)
        {
            fprintf(pbfile, "%d\n", pb->gameMode);

            for (size_t i = 0; i < SECTION_COUNT; ++i)
            {
                fprintf(pbfile, "%06d %06d\n",
                        pb->gameTime[i],
                        pb->goldST[i]);
            }
        }

        fclose(pbfile);
    }
}

int getGameEndLevel(int gameMode)
{
    if (gameMode == TAP_MODE_NORMAL ||
        gameMode == TAP_MODE_NORMAL_VERSUS ||
        gameMode == TAP_MODE_DOUBLES)
    {
        return LEVEL_MAX_SHORT;
    }

    return LEVEL_MAX_FULL;
}
