#!/bin/bash

WRITEFILE=$1
WRITESTR=$2

if [ -z "$WRITEFILE" ] || [ -z "$WRITESTR" ]
then
    echo "Two arguments expected"
    exit 1
fi

mkdir -p "$(dirname "$WRITEFILE")"
echo "$WRITESTR" > "$WRITEFILE"

if [ $? -ne 0 ]
then
    echo "File "$WRITEFILE" could not be created"
    exit 1
fi
