#!/bin/bash

if [ $# -eq 0 ]; then
    echo "No arguments supplied"
elif [ $# -gt 1 ]; then
    echo "Too many arguments"
elif [ "$1" = "sender" ] || [ "$1" = "receiver" ]; then
    gcc -o "$1" "$1".c
    if [ $? -eq 0 ]; then
        ./"$1"
    fi
else
    echo "Invalid program name"
fi
