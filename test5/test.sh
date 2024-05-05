#!/bin/bash

# Compile the client program
gcc -o test_client client.c

# Copy the compiled client executable to different directories
cp test_client ./client1/
cp test_client ./client2/

# Start the server in the background, but allow console output
./server_loss &

# Execute test_client in client1 directory without output suppression
cd ./client1/
./test_client < input.txt &
cd ..

# Allow some time for the first client to process
sleep 3

# Execute test_client in client2 directory without output suppression
cd ./client2/
./test_client < input.txt &
cd ..

# Allow sufficient time for the second client to finish processing
sleep 10

# Compare output files in client1 and client2 directories
diff ./client1/01.txt ./client2/01.txt

# Analyze the result of file comparison
if [ $? -eq 0 ]; then
    echo "Succeed"
else
    echo "Fail"
fi

# Terminate all instances of test_client and server_loss
pkill test_client
pkill server_loss

# Clean up by removing a specific file
rm ./client2/01.txt