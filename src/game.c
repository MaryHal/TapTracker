#include "game.h"

#include "inputhistory.h"
#include "sectiontable.h"
#include "gamehistory.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

const char* DISPLAYED_GRADE[GRADE_COUNT] =
{
    "9", "8", "7", "6", "5", "4-", "4+", "3-", "3+", "2-", "2", "2+", "1-",
    "1", "1+", "S1-", "S1", "S1+", "S2", "S3", "S4-", "S4", "S4+", "S5-",
    "S5+", "S6-", "S6+", "S7-", "S7+", "S8-", "S8+", "S9"
};

const char* getModeName(int gameMode)
{
    switch (gameMode)
    {
    case TAP_MODE_NULL:
        return "NULL";
    case TAP_MODE_NORMAL:
        return "Normal";
    case TAP_MODE_MASTER:
        return "Master";
    case TAP_MODE_DOUBLES:
        return "Doubles";
    case TAP_MODE_NORMAL_VERSUS:
        return "Normal Versus";
    case TAP_MODE_MASTER_VERSUS:
        return "Master Versus";
    case TAP_MODE_MASTER_CREDITS:
        return "Master Credits";
    case TAP_MODE_TGMPLUS:
        return "TGM+";
    case TAP_MODE_TGMPLUS_VERSUS:
        return "TGM+ Versus";
    case TAP_MODE_MASTER_ITEM:
        return "Master Item";
    case TAP_MODE_TGMPLUS_ITEM:
        return "TGM+ Item";
    case TAP_MODE_DEATH:
        return "Death";
    case TAP_MODE_DEATH_VERSUS:
        return "Death Versus";
    default:
        return "???";
    }
}

void game_init(struct game_t* g)
{
    UT_icd tap_state_icd =
    {
        sizeof(struct tap_state),
        NULL, // Initializer
        NULL, // Copier
        NULL  // Destructor
    };
    utringbuffer_new(g->blockHistory, GAME_STATE_HISTORY_LENGTH, &tap_state_icd);

    resetGame(g);
}

void game_terminate(struct game_t* g)
{
    utringbuffer_free(g->blockHistory);
}

struct game_t* game_create()
{
    struct game_t* g = malloc(sizeof(struct game_t));
    if (g)
    {
        game_init(g);
    }

    return g;
}

void game_destroy(struct game_t* g)
{
    assert(g != NULL);

    game_terminate(g);
    free(g);
}

void resetGame(struct game_t* game)
{
    /* memset(game, 0, sizeof(struct game_t)); */

    game->currentSection = 0;

    memset(&game->curState,  0, sizeof(struct tap_state));
    memset(&game->prevState, 0, sizeof(struct tap_state));

    utringbuffer_clear(game->blockHistory);
}

bool isInPlayingState(char state)
{
    return state != TAP_NONE && state != TAP_IDLE && state != TAP_STARTUP;
}

void updateGameState(struct game_t* game,
                     struct input_history_t* inputHistory,
                     struct section_table_t* sectionTable,
                     struct game_history_t* gameHistory,
                     struct tap_state* dataPtr)
{
    assert(game != NULL);

    game->prevState = game->curState;

    // Copy the data that we set in the MAME process.
    game->curState = *dataPtr;

    if (isInPlayingState(game->curState.state) && game->curState.level < game->prevState.level)
    {
        printf("Internal State Error: ");
        printGameState(game);
    }

    if (sectionTable)
    {
        if (isInPlayingState(game->curState.state) && game->curState.level - game->prevState.level > 0)
        {
            // Push a data point based on the newly acquired game state.
            updateSectionTable(sectionTable, game);
        }
    }

    // Piece is locked in
    if (isInPlayingState(game->curState.state) &&
        game->prevState.state == TAP_ACTIVE &&
        (game->curState.state == TAP_LOCKING || game->curState.state == TAP_LINECLEAR))
    {
        if (gameHistory)
            utringbuffer_push_back(game->blockHistory, &game->prevState);

        if (inputHistory)
            pushInputHistoryElement(inputHistory, game->curState.level);
    }

    // Check if a game has completely ended
    if (isInPlayingState(game->prevState.state) && !isInPlayingState(game->curState.state))
    {
        // Update gold STs now that the game is over. There is technically a
        // "Credit Roll" game mode but it doesn't seem to interfere with normal
        // pb updates.
        struct pb_table_t* pb = _getPBTable(&sectionTable->pbHash, game->prevState.gameMode);
        updateGoldSTRecords(pb, sectionTable, game->prevState.level);

        pushStateToGameHistory(gameHistory, game->blockHistory);

        resetGame(game);

        if (inputHistory)
            resetInputHistory(inputHistory);
        if (sectionTable)
            resetSectionTable(sectionTable);
    }
}

void printGameState(struct game_t* game)
{
    printf("state: %d -> %d, level %d -> %d, time %d -> %d\n",
           game->prevState.state, game->curState.state,
           game->prevState.level, game->curState.level,
           game->prevState.timer, game->curState.timer);
}

bool testMasterConditions(struct game_t* game)
{
    // Of course we can also check the bit mask, but this works too.
    return
        game->curState.mrollFlags == M_NEUTRAL ||
        game->curState.mrollFlags == M_PASS_1  ||
        game->curState.mrollFlags == M_PASS_2  ||
        game->curState.mrollFlags == M_SUCCESS;
}

#if 0
bool calculateMasterConditions_(struct game_t* game)
{
    int sectionSum = 0;
    const int TETRIS_INDEX = 3;

    for (int i = 0; i < game->currentSection; i++)
    {
        struct section_t* section = &game->sections[i];

        // First 5 sections must be completed in 1:05:00 or less
        if (i < 5)
        {
            if (getSectionTime(section) > frameTime(65))
            {
                return false;
            }
            sectionSum += getSectionTime(section);

            // Two tetrises per section is required for the first 5 sections.
            if (section->lines[TETRIS_INDEX] < 2)
            {
                return false;
            }
        }
        // Sixth section (500-600) must be less than two seconds slower than the
        // average of the first 5 sections.
        else if (i == 5)
        {
            if (getSectionTime(section) > frameTime(sectionSum / 5 + 2))
            {
                return false;
            }

            // One tetris is required for the sixth section.
            if (section->lines[TETRIS_INDEX] < 1)
            {
                return false;
            }
        }
        // Last four sections must be less than two seconds slower than the
        // previous section.
        else
        {
            struct section_t* prevSection = &game->sections[i - 1];

            if (getSectionTime(section) > getSectionTime(prevSection) + frameTime(2))
            {
                return false;
            }

            // One tetris is required for the last four sections EXCEPT the last
            // one.
            if (section->lines[TETRIS_INDEX] < 1)
            {
                return false;
            }
        }
    }

    // Finally, an S9 grade is required at level 999 along with the same time
    // requirements as the eigth section.
    if (game->curState.level == LEVEL_MAX_LONG)
    {
        if (game->curState.grade < MASTER_S9_INTERNAL_GRADE)
        {
            return false;
        }

        // Hard time requirement over the entire game is 8:45:00
        if (game->sections[9].endTime - game->sections[0].startTime > frameTime(8 * 60 + 45))
        {
            return false;
        }

        // Test section time vs previous section
        struct section_t* section = &game->sections[9];
        struct section_t* prevSection = &game->sections[8];

        if (getSectionTime(section) > getSectionTime(prevSection) + frameTime(2))
        {
            return false;
        }
    }

    return true;
}
#endif
