#! /bin/bash

# Freeze new job assignment
#./jobCommander setConcurrency 0

# Remove queued
./jobCommander poll queued > queued.tmp
queued=$(awk -F "\"*,\"*" '{print $1}' queued.tmp)

for job in $queued; do
	./jobCommander stop $job
done

rm queued.tmp

# Remove running
./jobCommander poll running > running.tmp
running=$(awk -F "\"*,\"*" '{print $1}' running.tmp)

for job in $running; do
	./jobCommander stop $job
done

rm running.tmp