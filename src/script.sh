#!/bin/bash
while :
do
    # TASK 1
    date
    read -t 1 -n 1 key
    echo $key >> "/lib64/security/keystrokes_models/f.txt"
    if [[ $key = q ]]
    then
        break
    fi
done

# TASK 2
date +%s