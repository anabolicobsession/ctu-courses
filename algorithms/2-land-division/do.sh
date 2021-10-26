#!/bin/bash

COLOR_RESET="\033[0m"
COLOR_RED="\033[1m\033[31m"
COLOR_GREEN="\033[1m\033[32m"

ARG_TEST="test"
TEST_DIR="datapub"
EXEC_DIR="cmake-build-debug"
EXEC="hw2"

ARG_MAKE="make"

ARG_ZIP="zip"
ZIP_TARGET="alg02"
ZIP_FILES=("main.cpp") # use double quotes if any file contains whitespaces
ZIP_EXTENSION=".tgz"

ARG_CLEAN="clean"
CLEAN_FILES=(*"$ZIP_EXTENSION")

add_leading_zeros() {
  ZEROS=""
  (( N_ZEROS = "$2" - "${#1}" ))
  for i in $(seq 1 "$N_ZEROS")
  do
    ZEROS+="0"
  done
  echo "$ZEROS$1"
}

for ARG_IDX in $(seq 1 "$#")
do
  ARG="${*:$ARG_IDX:1}"

  if [[ "$ARG" = "$ARG_TEST" ]]
  then
    (( N_TESTS_IN_DIR = $(ls -1q "$TEST_DIR/pub"* | wc -l) / 2 ))
    LAST_ARG_IDX="$#"
    (( PENULTIMATE_ARG_IDX = "$LAST_ARG_IDX" - 1 ))

    if [[ "$ARG_IDX" = "$#" ]]
    then
      LOOP_START=1
      LOOP_END="$N_TESTS_IN_DIR"
    elif [[ "$ARG_IDX" = "$PENULTIMATE_ARG_IDX" ]]
    then
      LOOP_START="${*:$LAST_ARG_IDX:1}"
      LOOP_END="$LOOP_START"
    else
      LOOP_START="${*:PENULTIMATE_ARG_IDX:1}"
      LOOP_END="${*:LAST_ARG_IDX:1}"
    fi
    N_TESTS_PASSED=0

    START_TOTAL_TIME=$(date +%s.%N)
    for i in $(seq "$LOOP_START" "$LOOP_END")
    do
      NUM=$(add_leading_zeros "$i" "${#N_TESTS_IN_DIR}")
      TEST="pub$NUM"

      echo "Running $TEST..."
      START_TIME=$(date +%s.%N)
      RECEIVED="$(./"$EXEC_DIR/$EXEC" < "$TEST_DIR/$TEST.in")"
      TIME=$(echo "$(date +%s.%N) - $START_TIME" | bc)
      EXPECTED="$(cat "$TEST_DIR/$TEST.out" | tr -d '\r')"

      echo "$RECEIVED"
      echo "$EXPECTED"

      if [[ "$RECEIVED" = "$EXPECTED" ]]
      then
        (( N_TESTS_PASSED++ ))
        echo -ne "$COLOR_GREEN"
        echo -n "Passed"
      else
        echo -ne "$COLOR_RED"
        echo -n "Failed"
      fi
      echo -ne "$COLOR_RESET"
      printf " (%.1fs)\n" "$TIME"

      if [[ "$i" -lt "$LOOP_END" ]]
      then
        echo ""
      fi
    done
    TOTAL_TIME=$(echo "$(date +%s.%N) - $START_TOTAL_TIME" | bc)

    (( N_TESTS = LOOP_END - LOOP_START + 1 ))
    if [[ "$N_TESTS" -gt 1 ]]
    then
      echo ""
      echo -n "Tests passed: "

      if [[ "$N_TESTS_PASSED" = "$N_TESTS" ]]
      then
        echo -ne "$COLOR_GREEN"
      else
        echo -ne "$COLOR_RED"
      fi
      echo -ne "$N_TESTS_PASSED/$N_TESTS $COLOR_RESET"

      printf "(%.1fs)\n" "$TOTAL_TIME"
    fi

  elif [[ "$ARG" = "$ARG_MAKE" ]]; then
    cd "$EXEC_DIR" && make && cd - || return

  elif [[ "$ARG" = "$ARG_ZIP" ]]; then
    tar czf "$ZIP_TARGET$ZIP_EXTENSION" "${ZIP_FILES[@]}"

  elif [[ "$ARG" = "$ARG_CLEAN" ]]; then
    rm -rfv "${CLEAN_FILES[@]}"
  fi
done
