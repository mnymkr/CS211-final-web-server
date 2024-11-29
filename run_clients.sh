#!/bin/bash
# Chạy 100 client đồng thời
for i in {1..100}
do
    ./client_event &
done

# Chờ tất cả các client chạy xong
wait
