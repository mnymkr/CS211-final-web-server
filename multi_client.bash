#!/bin/bash

for N in {1..10}
do
    ./client 1 &
done
wait
