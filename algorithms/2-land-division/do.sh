#!/bin/bash

ARG_TEST="test"
TEST_DIR="datapub"
EXEC_DIR="cmake-build-debug"
EXEC="hw2"

ARG_MAKE="make"

ARG_ZIP="zip"
ZIP_TARGET="alg01"
ZIP_FILES=("main.cpp" "CMakeLists.txt") # use double quotes if any file contains whitespaces
ZIP_EXTENSION=".tgz"

ARG_CLEAN="clean"
CLEAN_FILES=(*"$ZIP_EXTENSION")

add_leading_zeros() {
  ZEROS=""
  (( N_ZEROS = "$2" - "${#1}" ))
  for i in $(seq 1 "$N_ZEROS"); do
    ZEROS+="0"
  done
  echo "$ZEROS$1"
}

for ARG_IDX in $(seq 1 "$#"); do
  ARG="${*:$ARG_IDX:1}"

  if [[ "$ARG" = "$ARG_TEST" ]]; then
    (( N_TESTS = $(ls -1q "$TEST_DIR" | wc -l) / 2 ))

    if [[ "$ARG_IDX" = "$#" ]]; then
      ALL_TESTS_PASSED=1
      LOOP_START=1
      LOOP_END="$N_TESTS"
    else
      (( NEXT_ARG_IDX = ARG_IDX + 1 ))
      LOOP_START="${*:$NEXT_ARG_IDX:1}"
      LOOP_END="$LOOP_START"
    fi

    for i in $(seq "$LOOP_START" "$LOOP_END"); do
      NUM=$(add_leading_zeros "$i" "${#N_TESTS}")
      TEST="pub$NUM"

      echo "Running $TEST..."
      START_TIME=$(date +%s.%N)
      RECEIVED="$(./"$EXEC_DIR/$EXEC" < "$TEST_DIR/$TEST.in")"
      TIME=$(echo "$(date +%s.%N) - $START_TIME" | bc)
      TOTAL_TIME+=("$TIME")
      EXPECTED="$(cat "$TEST_DIR/$TEST.out" | tr -d '\r')"

      echo "$RECEIVED"
      echo "-"
      echo "$EXPECTED"

      if [[ "$RECEIVED" = "$EXPECTED" ]]
      then
        printf "Passed: %.2fs\n" "$TIME"
      else
        printf "Failed: %.2fs\n" "$TIME"
        ALL_TESTS_PASSED=0
        break
      fi

      if [[ "$i" -lt "$LOOP_END" ]]
      then
        echo "---"
      fi
    done

    if [[ "$ALL_TESTS_PASSED" = 1 ]]
    then
      echo ""
      echo "All tests passed"
    fi

  elif [[ "$ARG" = "$ARG_MAKE" ]]; then
    cd "$EXEC_DIR" && make && cd - || return

  elif [[ "$ARG" = "$ARG_ZIP" ]]; then
    tar czf "$ZIP_TARGET$ZIP_EXTENSION" "${ZIP_FILES[@]}"

  elif [[ "$ARG" = "$ARG_CLEAN" ]]; then
    rm -rfv "${CLEAN_FILES[@]}"
  fi
done
