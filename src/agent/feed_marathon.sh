#!/usr/bin/env bash

MARATHON=$1

curl -X POST "${MARATHON}/v2/apps" -d @"./mesos_marathon_app.json" \
 -H "Content-type: application/json"