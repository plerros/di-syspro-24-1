#!/bin/bash

for file in "$@"; do
	while read line; do
		./jobCommander issueJob $line
	done <$file
done