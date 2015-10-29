#ifndef FUMEN_H
#define FUMEN_H

#include <stdbool.h>

#define FIELD_SIZE 220

// Adapted from raincomplex's fumen.py

const char encodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char asciiTable[]  = " !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

const char pieceData[] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,1,1,1,2,1,3,1,1,0,1,1,1,2,1,3,0,1,1,1,2,1,3,1,1,0,1,1,1,2,1,3,
	0,1,1,1,2,1,0,2,1,0,1,1,1,2,2,2,2,0,0,1,1,1,2,1,0,0,1,0,1,1,1,2,
	1,1,2,1,1,2,2,2,1,1,2,1,1,2,2,2,1,1,2,1,1,2,2,2,1,1,2,1,1,2,2,2,
	0,1,1,1,1,2,2,2,2,0,1,1,2,1,1,2,0,1,1,1,1,2,2,2,2,0,1,1,2,1,1,2,
	0,1,1,1,2,1,1,2,1,0,1,1,2,1,1,2,1,0,0,1,1,1,2,1,1,0,0,1,1,1,1,2,
	0,1,1,1,2,1,2,2,1,0,2,0,1,1,1,2,0,0,0,1,1,1,2,1,1,0,1,1,0,2,1,2,
	1,1,2,1,0,2,1,2,0,0,0,1,1,1,1,2,1,1,2,1,0,2,1,2,0,0,0,1,1,1,1,2
};

struct piece_t
{
        char kind;
        char rot;
        char pos;
};

struct frame_t
{
        // Playfield of blocks, with two extra rows on top.
        char field[FIELD_SIZE];
        bool willLock;

        bool rise;
        bool mirror;

        struct piece_t piece;
        const char* comment;
};

struct frame_t createFrame();
bool isRep(struct frame_t* frame, struct frame_t* other);
void lockPiece(struct frame_t* frame);
struct frame_t copyFrame(struct frame_t* frame);
struct frame_t nextFrame(struct frame_t* frame);

void writeData(char* data, int n);

// Clear completed lines from a frame's playfield.
void clearLines(struct frame_t* frame);

// Return the difference between two playfields
const char* getDiff(const char* a, const char* b);

// Build our fumen link
const char* makeFumen(struct frame_t frames[], int rotMode, const char* baseURL);

#endif /* FUMEN_H */
