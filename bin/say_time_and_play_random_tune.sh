#! /usr/bin/env bash

~/series-timer/bin/say_time.sh $1
FLAC="`$HOME/series-timer/bin/get_random_tune.rb`"
open -a Audirvana "$FLAC"
