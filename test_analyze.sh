#!/bin/bash
# The arguments are paths to stdout ($1) and stderr ($2).
# This script assumes that nothing is in stderr.

# Using a more complex while loop to avoid side effects such as trimming leading
# whitespace, interpreting backslash sequences, and skipping the trailing line
# if it's missing a terminating linefeed. If these are concerns, you can do:
while IFS="" read -r L || [ -n "$L" ]; do
    echo "$L"|grep -h "(Failed)" > /dev/null
    if [[ $? -eq 0  ]]; then
        echo "failed:"$(echo "$L"|sed -e 's/.*(Failed)\(.*\)]].*/\1/')
    fi
    echo "$L"|grep -h "(Passed)" > /dev/null
    if [[ $? -eq 0  ]]; then
        echo "passed:"$(echo "$L"|sed -e 's/.*(Passed)\(.*\)]].*/\1/')
    fi
done < $1