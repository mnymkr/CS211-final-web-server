#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <number_of_runs> <command>"
    exit 1
fi

# Read parameters
runs=$1
command=$2

# Initialize variables to store cumulative values for calculations
total_time_sum=0
throughput_sum=0

# File to store intermediate results (optional, for logging purposes)
log_file="client_IO_results.log"

# Clear or create the log file
> "$log_file"

# Loop to run the command specified number of times
for ((i=1; i<=runs; i++)); do
    echo "Run #$i:" >> "$log_file"

    # Execute the command and capture its output
    output=$($command)

    # Extract total time and throughput using grep and awk
    total_time=$(echo "$output" | grep -oP 'Total time taken for all files: \K[0-9.]+')
    throughput=$(echo "$output" | grep -oP 'Throughput: \K[0-9.]+')

    # Log the results
    echo "$output" >> "$log_file"

    # Add extracted values to cumulative sums
    total_time_sum=$(echo "$total_time_sum + $total_time" | bc)
    throughput_sum=$(echo "$throughput_sum + $throughput" | bc)

done

# Calculate averages
average_total_time=$(echo "scale=6; $total_time_sum / $runs" | bc)
average_throughput=$(echo "scale=6; $throughput_sum / $runs" | bc)

# Print the results
cat << EOF
After $runs runs of '$command':
Average Total Time Taken: $average_total_time ms
Average Throughput: $average_throughput requests per second
EOF

