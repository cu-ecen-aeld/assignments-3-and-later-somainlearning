#!/bin/bash

FILESDIR=$1
SEARCHSTR=$2

if [ -z "$FILESDIR" ] || [ -z "$SEARCHSTR" ]
then
    echo "Two arguments expected"
    exit 1
else
    if [ ! -d "$FILESDIR" ]; then
	echo "Failed: Directory $FILESDIR not found"
	exit 1
    fi
fi

NUMFILES=$(find "$FILESDIR" -mindepth 1 \( -type f -o -type d \) | wc -l)
MATCHCNT=$(grep -r "$SEARCHSTR" "$FILESDIR" | wc -l )

echo "The number of files are ${NUMFILES} and the number of matching lines are ${MATCHCNT}"
