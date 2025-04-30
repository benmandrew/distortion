#!/bin/sh

find inc/ src/ -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
