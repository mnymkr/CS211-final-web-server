#!/bin/bash

for N in {1..10000}
do
    ./client 1 &
done
wait