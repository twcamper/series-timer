#! /usr/bin/env bash

ps x | grep $1 | grep -v grep | awk '{print $1}' | xargs kill
