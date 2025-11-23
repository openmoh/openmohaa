#!/bin/bash

/usr/local/games/openmohaa/lib/openmohaa/omohaaded +set fs_homepath home +set dedicated 2 +set net_port ${GAME_PORT:-12203} +set net_gamespy_port ${GAMESPY_PORT:-12300} $@
