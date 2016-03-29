#include "sectiontable.h"

#include "game.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <assert.h>

const char* DEFAULT_GOLD_ST_FILENAME = "GoldST.txt";

void setDefaultPBTimes(struct pb_table_t* pb, int gameMode)
{
    int defaultSectionTime = 3900; // 01:05:00

    if (getBaseMode(gameMode) == TAP_MODE_DEATH)
    {
        defaultSectionTime = 2700; // 00:45:00
    }

    for (int i = 0; i < SECTION_COUNT_LONG; ++i)
    {
        pb->gameTime[i] = defaultSectionTime * (i + 1);
        pb->goldST[i] = defaultSectionTime;
    }
}

struct pb_table_t* _addPBTable(struct pb_table_t** map, int gameMode)
{
    struct pb_table_t* pb = _getPBTable(map, gameMode);

    if (pb == NULL)
    {
        pb = malloc(sizeof(struct pb_table_t));
        pb->gameMode = gameMode;

        setDefaultPBTimes(pb, gameMode);

        char modeName[32];
        getModeName(modeName, 32, pb->gameMode);

        printf("Adding pb table: %d (%s).\n", gameMode, modeName);

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

void section_table_init(struct section_table_t* table, const char* pbfile)
{
    for (int i = 0; i < SECTION_COUNT_LONG; ++i)
    {
        int begin = i * SECTION_LENGTH;
        int end   = (i + 1) * SECTION_LENGTH;

        if (end > LEVEL_MAX_LONG)
            end = LEVEL_MAX_LONG;

        struct section_t* section = &table->sections[i];
        snprintf(section->label, 8, "%03d-%03d", begin, end);
    }

    table->pbHash = NULL;

    readSectionRecords(table, pbfile);
    resetSectionTable(table);
}

void section_table_terminate(struct section_table_t* table)
{
    writeSectionRecords(table);
}

struct section_table_t* section_table_create(const char* pbfile)
{
    struct section_table_t* s = malloc(sizeof(struct section_table_t));
    if (s)
        section_table_init(s, pbfile);

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

        section->complete = false;
    }

    // Add an initial data point to the first section
    table->sections[0].data[0] = (struct datapoint_t){ 0, 0 };
    table->sections[0].size++;
}

void updateSectionTable(struct section_table_t* table, struct game_t* game)
{
    assert(game->currentSection >= 0 && game->currentSection < SECTION_COUNT_LONG);

    struct section_t* section = &table->sections[game->currentSection];

    int gameMode = game->originalGameMode;
    struct pb_table_t* pb     = _getPBTable(&table->pbHash, gameMode);

    // Special case for when we're at the end of the game (level 999). The
    // in-game timer seems to reset back to 00:00:00 on the same exact frame
    // that level 999 is reached. So when we're adding our data point, we must
    // use the previous frame's timer value.
    if (game->curState.level >= getModeEndLevel(gameMode))
    {
        // On the first frame reaching the end of the game, this block should
        // only be run once.
        if (!section->complete && section->endTime == 0)
        {
            section->endTime = game->prevState.timer;

            int tempTime = game->curState.timer;
            game->curState.timer = game->prevState.timer;

            addDataPointToSection(section, game);

            // For consistency, restore original time value.
            game->curState.timer = tempTime;

            section->complete = true;

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

        section->complete = true;

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
        printf("Reading PB times @ \"%s\".\n", filename);

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

void updateGoldSTRecords(struct pb_table_t* pb, struct section_table_t* table)
{
    if (pb == NULL || table == NULL)
    {
        return;
    }

    char modeName[32];
    getModeName(modeName, 32, pb->gameMode);

    printf("Updating Gold STs for mode %d (%s).\n", pb->gameMode, modeName);

    // Update all new Gold STs.
    const int NUM_SECTIONS = getModeSectionCount(pb->gameMode);
    for (int i = 0; i < NUM_SECTIONS; ++i)
    {
        const struct section_t* section = &table->sections[i];

        int sectionTime = section->endTime - section->startTime;
        if (section->complete && sectionTime > 0 && pb->goldST[i] > sectionTime)
        {
            // If a section time is less than 20 seconds, block the update.
            // Pretty arbitrary choice of time, but if a section is completed,
            // realistically it won't be less than 20 seconds long. The Death WR
            // run has fastest STs of about ~23 seconds.
            if (sectionTime < 1200)
            {
                continue;
            }

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

    const int NUM_SECTIONS = getModeSectionCount(pb->gameMode);

    // Test if all sections have been completed.
    for (int i = 0; i < NUM_SECTIONS; ++i)
    {
        if (!table->sections[i].complete)
        {
            printf("All sections have not been completed\n");
            return;
        }
    }

    // Update all PBs if necessary
    if (pb->gameTime[NUM_SECTIONS - 1] > table->sections[NUM_SECTIONS - 1].endTime)
    {
        for (int i = 0; i < NUM_SECTIONS; ++i)
        {
            if (table->sections[i].endTime > 0)
                pb->gameTime[i] = table->sections[i].endTime;
        }
    }
}

int getModeEndLevel(int gameMode)
{
    if (gameMode == TAP_MODE_NORMAL ||
        gameMode == TAP_MODE_DOUBLES)
    {
        return LEVEL_MAX_SHORT;
    }

    return LEVEL_MAX_LONG;
}

int getModeSectionCount(int gameMode)
{
    if (gameMode == TAP_MODE_NORMAL ||
        gameMode == TAP_MODE_DOUBLES)
    {
        return SECTION_COUNT_SHORT;
    }

    return SECTION_COUNT_LONG;
}
