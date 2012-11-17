#! /usr/bin/env bash

~/timer/bin/say_time.sh $1
FLAC="`$HOME/timer/bin/get_random_tune.rb`"
open -a Audirvana "$FLAC"
