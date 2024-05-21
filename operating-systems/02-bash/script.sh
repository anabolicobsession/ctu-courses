#!/bin/bash

PATH_PREFIX="PATH "
ARG_HELP="-h"
ARG_ZIP="-z"
HELP_FILEPATH="help.txt"
FILES_TO_ZIP=()
DO_ZIP=false
WRONG_PATH_WAS_GIVEN=false

CODE_SUCCESS=0
CODE_ERROR=1
CODE_FATAL_ERROR=2

for ARG in "$@"; do
  if [[ "$ARG" = "$ARG_HELP" ]]; then
    cat "$HELP_FILEPATH"
    exit $CODE_SUCCESS
  elif [[ "$ARG" = "$ARG_ZIP" ]]; then
    DO_ZIP=true
  else
    exit $CODE_FATAL_ERROR
  fi
done

while IFS= read -r LINE; do
  if [[ "$LINE" = "$PATH_PREFIX"* ]]; then
    
    INPUT_PATH=${LINE#"$PATH_PREFIX"}
    if [[ -L "$INPUT_PATH" ]]; then
          echo "LINK '$INPUT_PATH' '$(readlink "$INPUT_PATH")'"
    elif [[ -f "$INPUT_PATH" ]]; then
      if [[ -r "$INPUT_PATH" ]]; then
        FILES_TO_ZIP+=("$INPUT_PATH")
        echo "FILE '$INPUT_PATH' $(wc -l < "$INPUT_PATH") '$(head -n 1 "$INPUT_PATH")'"
      else
        exit $CODE_FATAL_ERROR
      fi
    elif [[ -d "$INPUT_PATH" ]]; then
      echo "DIR '$INPUT_PATH'"
    else
      WRONG_PATH_WAS_GIVEN=true
      echo "ERROR '$INPUT_PATH'" >&2
    fi

  fi
done

if $DO_ZIP; then
  tar czf output.tgz "${FILES_TO_ZIP[@]}"
fi

if $WRONG_PATH_WAS_GIVEN; then
  exit $CODE_ERROR
fi

exit $CODE_SUCCESS
