#!/bin/bash

green='\033[0;32m'
red='\033[0;31m'
clear='\033[0m'

echo "Compiling ESP test files..."
g++ -DUNIT_TEST -I./lib/unity -I./lib -I./ESP32 -I./tests \
    Tests/ESPTests.cpp lib/unity/unity.c lib/ArduinoMock.cpp \
    -o Tests/testRunner

if [ $? -ne 0 ]; then
  echo "Compilation failed!"
  exit 1
fi

echo "Running ESP tests..."
./Tests/testRunner

if [ $? -eq 0 ]; then
  printf "${green}All ESP tests passed!${clear}\n"
else
  printf "${red}Some ESP tests failed.${clear}\n"
  exit 1
fi

rm -rf Tests/testRunner
