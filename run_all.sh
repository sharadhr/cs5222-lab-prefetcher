#!/usr/bin/env bash

built_executables="build/*"
traces="traces/*.dpc"
for trace_path in $traces ; do
    for sim_path in $built_executables ; do
        prefetcher=$(basename "$sim_path")
        trace=$(basename "$trace_path" "_trace2.dpc")
        $sim_path < "$trace_path" > "output/$prefetcher;$trace" &
    done
done

wait
