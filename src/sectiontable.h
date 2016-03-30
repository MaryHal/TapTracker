#ifndef SECTIONTIME_H
#define SECTIONTIME_H

#include <stdbool.h>
#include <uthash.h>

#define LEVEL_MAX_SHORT 300
#define LEVEL_MAX_LONG  999

#define SECTION_COUNT_SHORT 3
#define SECTION_COUNT_LONG  10

#define SECTION_LENGTH 100
#define SECTION_MAX    128

extern const char* DEFAULT_GOLD_ST_FILENAME;

struct datapoint_t
{
    int level;
    int time;
};

struct section_t
{
    char label[8];

    struct datapoint_t data[SECTION_MAX];
    size_t size;

    int startTime; // Frame count for when this section began.
    int endTime;

    int lines[4];

    bool complete;
    bool mTest;
};

struct pb_table_t
{
    int gameMode; // Key

    int gameTime[SECTION_COUNT_LONG];
    int goldST[SECTION_COUNT_LONG];

    UT_hash_handle hh;
};

struct pb_table_t* addPBTable(struct pb_table_t** map, int gameMode);
void deletePBTable(struct pb_table_t** map, int gameMode);
struct pb_table_t* getPBTable(struct pb_table_t** map, int gameMode);

struct section_table_t
{
    struct section_t sections[SECTION_COUNT_LONG];
    struct pb_table_t* pbHash;

    const char* filename;
};

struct game_t;

void section_table_init(struct section_table_t* table, const char* pbfile);
void section_table_terminate(struct section_table_t* table);

struct section_table_t* section_table_create(const char* pbfile);
void section_table_destroy(struct section_table_t* table);

void resetSectionTable(struct section_table_t* table);

// Should be called after a block a placed.
void updateSectionTable(struct section_table_t* table, struct game_t* game);
void addDataPointToSection(struct section_t* section, struct game_t* game);

void readSectionRecords(struct section_table_t* table, const char* filename);
void writeSectionRecords(struct section_table_t* table, const char* filename);

void updateGoldSTRecords(struct pb_table_t* pb, struct section_table_t* table);
void updateGameTimeRecords(struct pb_table_t* pb, struct section_table_t* table);

int getModeEndLevel(int gameMode);
int getModeSectionCount(int gameMode);

#endif /* SECTIONTIME_H */
