#! /usr/bin/env bash

for p in `pgrep $1`;
do
  kill -9 $p;
done
