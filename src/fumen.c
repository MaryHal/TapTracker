#include "fumen.h"

#include <string.h>
#include <stdlib.h>

struct frame_t createFrame()
{
    return (struct frame_t)
    {
        .field = {0},
            .willLock = false,
                 .rise = false,
                 .mirror = false,
                 .piece = (struct piece_t){0, 0, 0},
                 .comment = NULL
                      };
}

bool isRep(struct frame_t* frame, struct frame_t* other)
{
    for (int i = 0; i < FIELD_SIZE; ++i)
    {
        if (frame->field[i] != other->field[i])
        {
            return false;
        }
    }

    return true;
}

void lockPiece(struct frame_t* frame)
{
    if (frame->piece.kind != 0)
    {
        for (int i = 0; i < 4; ++i)
        {
            frame->field[frame->piece.pos + pieceData[frame->piece.kind * 32 + frame->piece.rot * 8 + i * 2 + 1] * 10 + pieceData[frame->piece.kind * 32 + frame->piece.rot * 8 + i * 2] - 11] = frame->piece.kind;
        }
    }
}

struct frame_t copyFrame(struct frame_t* frame)
{
    struct frame_t copy;
    copy.willLock = frame->willLock;
    copy.rise     = frame->rise;
    copy.mirror   = frame->mirror;
    copy.piece    = frame->piece;
    copy.comment  = frame->comment;

    for (int i = 0; i < FIELD_SIZE; i++)
    {
        copy.field[i] = frame->field[i];
    }

    return copy;
}

struct frame_t nextFrame(struct frame_t* frame)
{
    struct frame_t copy = copyFrame(frame);

    if (copy.willLock)
    {
        lockPiece(&copy);
    }

    clearLines(&copy);

    return copy;
}

void writeData(char* data, int n)
{
    (void) data, (void) n;
}

void clearLines(struct frame_t* frame)
{
    char output[FIELD_SIZE];
    int m = 21;

    for (int i = 21; i > 0; --i)
    {
        int c = 0;
        for (int k = 0; k < 10; ++k)
        {
            if (frame->field[i * 10 + k] != 0)
            {
                c++;
            }
        }

        if (c < 10)
        {
            for (int k = 0; k < 10; ++k)
            {
                output[m * 10 + k] = frame->field[i * 10 + k];
            }
            m -= 1;
        }
    }

    // Copy the new playfield over.
    memcpy(frame->field, output, sizeof(char) * FIELD_SIZE);
}

const char* getDiff(const char* a, const char* b)
{
    (void) a, (void) b;
    return NULL;
}

const char* makeFumen(struct frame_t frames[], int rotMode, const char* baseURL)
{
    (void) frames, (void) rotMode, (void) baseURL;
    return NULL;
}
