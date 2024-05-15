#! /bin/bash

# Freeze new job assignment
#./jobCommander setConcurrency 0

# Remove queued
rm -f queued.tmp
./jobCommander poll queued 1> queued.tmp
queued=$(awk -F "\"*,\"*" '{print $1}' queued.tmp)

for job in $queued; do
	./jobCommander stop $job
done

rm queued.tmp

# Remove running
rm -f running.tmp
./jobCommander poll running 1> running.tmp
running=$(awk -F "\"*,\"*" '{print $1}' running.tmp)

for job in $running; do
	./jobCommander stop $job
done

rm running.tmp
