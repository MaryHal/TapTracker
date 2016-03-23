#include "sectiontable.h"

#include "game.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <assert.h>

void setDefaultPBTimes(struct pb_table_t* pb)
{
    for (int i = 0; i < SECTION_COUNT_LONG; ++i)
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
    for (int i = 0; i < SECTION_COUNT_LONG; ++i)
    {
        int begin = i * SECTION_LENGTH;
        int end   = (i + 1) * SECTION_LENGTH;

        if (end > LEVEL_MAX_LONG)
            end = LEVEL_MAX_LONG;

        snprintf(table->sections[i].label, 8,
                 "%03d-%03d", begin, end);
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
    for (size_t i = 0; i < SECTION_COUNT_LONG; ++i)
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
    assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT_LONG);

    struct section_t* section = &table->sections[game->currentSection];

    int gameMode = game->curState.gameMode;
    struct pb_table_t* pb     = _getPBTable(&table->pbHash, gameMode);

    // Special case for when we're at the end of the game (level 999). The
    // in-game timer seems to reset back to 00:00:00 on the same exact frame
    // that level 999 is reached. So when we're adding our data point, we must
    // use the previous frame's timer value.
    if (game->curState.level >= getModeEndLevel(gameMode))
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

            // Update Records
            updateGameTimeRecords(pb, table);

            // We update Gold STs when the game ends in any way, not just when
            // we complete the game.
            /* updateGoldSTRecords(pb, table); */
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

        assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT_LONG);
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

            for (size_t i = 0; i < SECTION_COUNT_LONG; ++i)
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

            for (size_t i = 0; i < SECTION_COUNT_LONG; ++i)
            {
                fprintf(pbfile, "%06d %06d\n",
                        pb->gameTime[i],
                        pb->goldST[i]);
            }
        }

        fclose(pbfile);
    }
}

bool shouldBlockRecordUpdate(struct pb_table_t* pb, struct section_table_t* table)
{
    const int NUM_SECTIONS = getModeSectionCount(pb->gameMode);
    for (int i = 0; i <= NUM_SECTIONS; ++i)
    {
        int sectionTime = table->sections[i].endTime - table->sections[i].startTime;

        if (sectionTime < 1200)
        {
            return true;
        }
    }

    return false;
}

void updateGoldSTRecords(struct pb_table_t* pb, struct section_table_t* table, int levelOfDeath)
{
    if (pb == NULL || table == NULL)
    {
        return;
    }

    printf("Updating Gold STs for mode %d.\n", pb->gameMode);

    if (shouldBlockRecordUpdate(pb, table))
    {
        printf("\tUnrealistic section times detected, Gold ST Update Blocked.\n");
    }

    // Update all new Gold STs.
    const int NUM_SECTIONS = getModeSectionCount(pb->gameMode);
    for (int i = 0; i < NUM_SECTIONS && i < levelOfDeath / 100; ++i)
    {
        const struct section_t* section = &table->sections[i];

        int sectionTime = section->endTime - section->startTime;
        if (sectionTime > 0 && pb->goldST[i] > sectionTime)
        {
            pb->goldST[i] = sectionTime;
        }
    }
}

void updateGameTimeRecords(struct pb_table_t* pb, struct section_table_t* table)
{
    if (pb == NULL || table == NULL)
    {
        return;
    }

    printf("Updating Game Time PBs for mode %d.\n", pb->gameMode);

    if (shouldBlockRecordUpdate(pb, table))
    {
        printf("\tUnrealistic section times detected, Game Time PB Update Blocked.\n");
    }

    // Update all PBs if necessary
    const int NUM_SECTIONS = getModeSectionCount(pb->gameMode);
    if (pb->gameTime[NUM_SECTIONS - 1] > table->sections[NUM_SECTIONS - 1].endTime)
    {
        for (int i = 0; i <= NUM_SECTIONS; ++i)
        {
            if (table->sections[i].endTime > 0)
                pb->gameTime[i] = table->sections[i].endTime;
        }
    }
}

int getModeEndLevel(int gameMode)
{
    if (gameMode == TAP_MODE_NORMAL ||
        gameMode == TAP_MODE_NORMAL_VERSUS ||
        gameMode == TAP_MODE_DOUBLES)
    {
        return LEVEL_MAX_SHORT;
    }

    return LEVEL_MAX_LONG;
}

int getModeSectionCount(int gameMode)
{
    if (gameMode == TAP_MODE_NORMAL ||
        gameMode == TAP_MODE_NORMAL_VERSUS ||
        gameMode == TAP_MODE_DOUBLES)
    {
        return SECTION_COUNT_SHORT;
    }

    return SECTION_COUNT_LONG;
}
