#!/usr/bin/env python

import fumen
import mmap

import pyperclip

from enum import Enum

class TapState(Enum):
    NONE         = 0
    Start        = 1
    Active       = 2
    Locking      = 3  # Cannot be influenced anymore
    Lineclear    = 4  # Tetromino is being locked to the playfield.
    Entry        = 5
    Gameover     = 7  # "Game Over" is being shown on screen.
    Idle         = 10 # No game has started just waiting...
    Fading       = 11 # Blocks fading away when topping out (losing).
    Completion   = 13 # Blocks fading when completing the game
    Startup      = 71

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
    return state != TapState.NONE.value and state != TapState.Idle.value and state != TapState.Startup.value

def main():
    with open("/dev/shm/tgm2p_data", "r+b") as f:
        vSize = 4 * 13
        mm = mmap.mmap(f.fileno(), vSize)

        frameList = []
        frame = fumen.Frame()

        prevState = state = 0
        while True:
            prevState = state
            state = int(mm[0])
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

            # If a piece is locked in...
            if isInPlayingState(state) and prevState == TapState.Active.value and state == TapState.Locking.value:
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
