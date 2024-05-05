#!/bin/bash

# Compile the server program
gcc -o test_server server.c

# Start the server in the background, but allow console output
./test_server &

# Execute client_loss in client1 directory without output suppression
cd ./client1/
./client_loss < input.txt &
cd ..

# Pause to allow client1 to process
sleep 3

# Execute client_loss in client2 directory without output suppression
cd ./client2/
./client_loss < input.txt &
cd ..

# Allow sufficient time for client2 to finish processing
sleep 10

# Check if files are identical between the two clients
diff ./client1/01.txt ./client2/01.txt

# Check the exit status of the last command (diff)
if [ $? -eq 0 ]; then
    echo "Succeed"
else
    echo "Fail"
fi

# Kill the client_loss and test_server processes
pkill client_loss
pkill test_server

# Remove specific file
rm ./client2/01.txt