#!/bin/bash

sox radio_waves.wav -r 16k -b 32 -L -c 1 -e unsigned --norm music.raw gain -h +10 dither
