#!/usr/bin/env python

import fumen
import mmap

import pyperclip

def enum(**enums):
    return type('Enum', (), enums)

TapState = enum(
    NONE=0,
    Start=1,
    Active=2,
    Locking=3,     # Cannot be influenced anymore
    Lineclear=4,   # Tetromino is being locked to the playfield.
    Entry=5,
    Gameover=7,    # "Game Over" is being shown on screen.
    Idle=10,       # No game has started just waiting...
    Fading=11,     # Blocks fading away when topping out (losing).
    Completion=13, # Blocks fading when completing the game
    Startup=71
)

TapMRollFlags = enum(
    M_FAIL_1   = 17,
    M_FAIL_2   = 19,
    M_FAIL_END = 31,

    M_NEUTRAL  = 48,
    M_PASS_1   = 49,
    M_PASS_2   = 51,
    M_SUCCESS  = 127,
)

# TGM2+ indexes its pieces slightly differently to fumen, so when encoding a
# diagram we gotta convert the index.
# 2 3 4 5 6 7 8
# I Z S J L O T
TapToFumenMapping = [0, 0, 1, 4, 7, 6, 2, 3, 5]

def fixCoordinates(block, rotation):
    """Given a fumen tetromino index and rotation state, output a tuple (x, y) that
    represents how far to offset TGM2+'s (x, y) location coordinate."""
    if block == 1:
        if rotation == 1 or rotation == 3:
            return (1, 0)
    elif block == 6:
        if rotation == 2:
            return (0, -1)
    elif block == 2:
        if rotation == 2:
            return (0, -1)
    elif block == 5:
        if rotation == 2:
            return (0, -1)
    return (0, 0)

def isInPlayingState(state):
    """Given the game's current state, determine whether or not we're in game."""
    return state != TapState.NONE and state != TapState.Idle and state != TapState.Startup

def testMasterConditions(flags):
    """Given TGM2+'s M-Roll flags, return true if all the requirements so far have
been met. If any condition has failed, return false."""
    return flags == M_NEUTRAL or flags == M_PASS_1 or flags == M_PASS_2 or flags == M_SUCCESS

def main():
    with open("/dev/shm/tgm2p_data", "r+b") as f:
        vSize = 4 * 13
        mm = mmap.mmap(f.fileno(), vSize)

        frameList = []
        frame = fumen.Frame()

        prevInCreditRoll = inCreditRoll = 0
        prevState = state = 0
        while True:
            prevState = state
            state = int(mm[0])
            prevInCreditRoll = inCreditRoll
            inCreditRoll = int(mm[6 * 4])
            mrollFlags = int(mm[5 * 4])
            currentBlock = TapToFumenMapping[int(mm[8 * 4])]
            currentX = int(mm[10 * 4])
            currentY = int(mm[11 * 4])
            rotState = int(mm[12 * 4])

            # When inspecting the game's memory, I found that currentX
            # underflows, so let's "fix" that.
            if currentX > 10:
                currentX = -1

            # Coordinates from TAP do not perfectly align with fumen's
            # coordinates.
            offsetX, offsetY = fixCoordinates(currentBlock, rotState)
            currentX += offsetX
            currentY += offsetY

            # Set the current frame's tetromino + location
            frame.willlock = True
            frame.piece.kind = currentBlock
            frame.piece.rot = rotState
            frame.piece.pos = 220 - currentY * 10 + currentX


            # If we've entered the M-Roll, clear the field. This doesn't test
            # for a specific mode yet, only if the M-Roll conditions have been
            # met.
            if not prevInCreditRoll and inCreditRoll and testMasterConditions(mrollFlags):
                frame.field = []

            # If a piece is locked in...
            if isInPlayingState(state) and prevState == TapState.Active and state == TapState.Locking:
                frameList.append(frame.copy())
                frame = frame.next()

            # If the game is over...
            if isInPlayingState(prevState) and not isInPlayingState(state):
                # Lock the previous piece and place the killing piece. It's
                # state is not set, so the above lock check will not be run.
                frameList.append(frame.copy())
                frame = frame.next()

                fumenURL = fumen.make(frameList, 0)
                print (fumenURL, '\n')
                pyperclip.copy(fumenURL)

                frameList = []
                frame = fumen.Frame()

if __name__ == '__main__':
    main()
