#!/bin/bash

c++ -std=c++17 -o test *.cpp ../src/*.cpp
./test
rm test