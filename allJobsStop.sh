#! /bin/bash

i=0

while [[ $(./jobCommander poll queued) != "" ]]; do
	./jobCommander stop job_$i
	i=$(( i + 1 ))
done

i=0

while [[ $(./jobCommander poll running) != "" ]]; do
	./jobCommander stop job_$i
	i=$(( i + 1 ))
done