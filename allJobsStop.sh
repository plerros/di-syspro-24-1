#! /bin/bash

i=1

while [[ $(./jobCommander poll queued) != "" ]]; do
	./jobCommander stop job_$i
	i=$(( i + 1 ))
done

i=1

while [[ $(./jobCommander poll running) != "" ]]; do
	./jobCommander stop job_$i
	i=$(( i + 1 ))
done