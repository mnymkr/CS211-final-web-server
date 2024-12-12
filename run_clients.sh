#!/bin/bash
# Đo thời gian tổng cộng
start=$(date +%s.%N)

# Chạy 100 client đồng thời
for i in {1..100}
do
    ./client_time &
done

# Chờ tất cả các client chạy xong
wait

# Kết thúc đo thời gian
end=$(date +%s.%N)

# Tính thời gian thực thi
elapsed=$(echo "$end - $start" | bc)
echo "Total time for 100 clients: $elapsed seconds"

# Tính thông lượng
throughput=$(echo "100 / $elapsed" | bc -l)
echo "Throughput: $throughput requests per second"

