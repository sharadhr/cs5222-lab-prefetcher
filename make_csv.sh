#!/usr/bin/env bash

echo "trace,prefetcher,ipc" > results.csv

for output_file in output/* ; do
    file_name=$(basename "$output_file")
    echo "${file_name}"
    arr=("${file_name//;/ }")
    ipc=$(jq --raw-input < "${output_file}" | jq --slurp '.[-3]' | jq 'split(" ")' | jq --raw-output '.[-1]')
    echo "${arr[0]},${arr[1]}$ipc" >> results.csv
done
