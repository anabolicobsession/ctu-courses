#!/bin/bash

ARG_HELP="-h"
ARG_PDF="-a"
ARG_NUMBERS="-b"
ARG_SENTENCES="-c"
HELP_TEXT="BASH SCRIPT FOR 'OSY' ASSIGNMENT 2
-h  help
-a  print all pdf files in current directory
-b  print all lines starting with number
-c  print sentences on separate lines"

for ARG in "$@"; do

  if [[ "$ARG" = "$ARG_HELP" ]]; then
    echo "$HELP_TEXT"
  elif [[ "$ARG" = "$ARG_PDF" ]]; then
    find . -iname "*.pdf" | cut -c 3- | sort
  elif [[ "$ARG" = "$ARG_NUMBERS" ]]; then
    while IFS= read -r LINE; do
      if [[ "$LINE" = [0-9]* ]] || [[ "$LINE" = [+-][0-9]* ]]; then
        echo "$LINE" | sed "s/^[+-]//" | sed "s/^[0-9]*//"
      fi
    done
  elif [[ "$ARG" = "$ARG_SENTENCES" ]]; then
    TEXT=$(</dev/stdin)
    echo "$TEXT" | tr '\n' ' ' | grep -Eo '[[:upper:]][^.!?]*[.!?]'
  else
    echo "$HELP_TEXT"
    exit 1
  fi

  exit 0
done
