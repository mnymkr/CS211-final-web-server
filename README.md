# CS211 Final Web Server Benchmarking

This repository benchmarks the thread-based and event-based server for both non-IO and IO-bound tasks.

## Getting Started

### Clone the Repository

```bash
git clone [<repository-url>](https://github.com/mnymkr/CS211-final-web-server.git)
cd CS211-final-web-server
```

## Benchmarking Non-IO Tasks

To benchmark the servers for non-IO tasks, follow these steps:

1. **Compile and Run the Threaded Server:**
   ```bash
   gcc server_thread.c -o server_thread && ./server_thread
   ```

2. **Compile the Client Code:**
   ```bash
   gcc client.c -o client
   ```

3. **Run the Client Benchmark:**
   Execute the following command to run the client with a specified number of requests (e.g., 50):
   ```bash
   ./run_client.sh 50 "./client"
   ```
   You can replace `50` with any other number as needed.

For Event-based server, replace step 1 by:
   ```bash
   gcc server_event.c -o server_event && ./server_event
   ```

## Benchmarking IO-Bound Tasks

For benchmarking servers handling IO-bound tasks, use these commands:

1. **Compile and Run the Threaded Server for IO:**
   ```bash
   gcc server_thread_IO.c -o server_thread_IO && ./server_thread_IO
   ```

2. **Compile and Run the Event-Based Server for IO:**
   ```bash
   gcc server_event_IO.c -o server_event_IO && ./server_event_IO
   ```

3. **Compile the Client Code for IO:**
   ```bash
   gcc client_IO.c -o client_IO
   ```

4. **Run Client for Returning 1000 Small Files:**
   ```bash
   ./run_client.sh 50 "./client_IO 1000_small"
   ```

5. **Run Client for Returning 999 Small Files and 1 Large File:**
   ```bash
   ./run_client.sh 50 "./client_IO 999_small_1_large"
   ```
