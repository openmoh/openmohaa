#!/bin/bash

header=$'\xff\xff\xff\xff\x01disconnect'
# to avoid spamming the log file with getinfo/getstatus
# simply send an unsupported message
# so the server will return the disconnect message
message=$'none'

query_port=${GAME_PORT:-12203}
data=""

while [ -z "$data" ]
do
    data=$(echo "$message" | socat - UDP:0.0.0.0:$query_port 2>/dev/null)
    ret=$?

    if [ $ret != 0 ]
    then
        echo "Fail (socat returned $ret)"
        exit 1
    fi

    if [ -n "$data" ]
    then
        break
    fi
done

if [ "$data" != "$header" ]
then
    echo "Fail (not maching header)"
    echo $line
    exit 1
fi

# Success
exit 0
