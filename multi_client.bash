#!/bin/bash

for N in {1..1000}
do
    ./client 1 &
done
wait
