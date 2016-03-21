#!/usr/bin/env python

import simpleaudio

import mmap

import struct
import time

DATA_BLOCK_SIZE = 2

def unpack_mmap_block(mm, n):
    """Decode the nth 2-byte long byte string from mapped memory. This works with
both python 2 and python 3"""
    return struct.unpack("@h", mm[n*DATA_BLOCK_SIZE:(n+1)*DATA_BLOCK_SIZE])[0]

def main():
    with open("/dev/shm/taptracker_data", "r+b") as f:
        mm = mmap.mmap(f.fileno(), DATA_BLOCK_SIZE * 2)

        urgentObj = simpleaudio.WaveObject.from_wave_file('urgent.wav')
        panicObj  = simpleaudio.WaveObject.from_wave_file('panic.wav')

        prevLevel = level = 0
        while True:
            # # Let's read the current game level and current timer value:
            # timer = unpack_mmap_block(mm, 1)

            # In Python 3, we can do something as simple as:
            prevLevel = level;
            level = unpack_mmap_block(mm, 0)

            if prevLevel != level:
                sectionLevel = level % 100
                if sectionLevel >= 90 and sectionLevel < 96:
                    urgentObj.play()
                elif sectionLevel >= 96:
                    panicObj.play()

            # Sleepy-time may be a good idea.
            time.sleep(0.01)

        mm.close()

if __name__ == '__main__':
    main()
