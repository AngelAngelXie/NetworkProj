#!/bin/bash

# Compile the client and server programs
gcc -o test_client client.c
gcc -o test_server server.c

# Copy the compiled client executable to different directories
cp test_client ./client1/
cp test_client ./client2/

# Start the server in the background, but allow console output
./test_server &

# Navigate to client2 directory and execute client with input redirection from input.txt
cd ./client2/
./test_client < input.txt &
cd ..

# Navigate to client1 directory and execute client with input redirection from input.txt
cd ./client1/
./test_client < input.txt &
cd ..

# Wait for 10 seconds to allow processes to complete
sleep 10

# Check if files are identical between the two clients
diff ./client1/20.txt ./client2/20.txt

# Check the exit status of the last command (diff)
if [ $? -eq 0 ]; then
    echo "Succeed"
else
    echo "Fail"
fi

# Kill the test_client and test_server processes
pkill test_client
pkill test_server

# Remove specific file
rm ./client1/20.txt