#!/bin/bash

# Compile the client and server programs
gcc -o test_client client.c
gcc -o test_server server.c

# Copy the compiled client executable to different directories
cp test_client ./client1/
cp test_client ./client2/
cp test_client ./client3/

# Start the server in the background without suppressing its output
./test_server &

# Execute client in the background in different directories without suppressing output
cd ./client1/
./test_client < input.txt &
cd ..

cd ./client2/
./test_client < input.txt &
cd ..

cd ./client3/
./test_client < input.txt &
cd ..

# Wait for 10 seconds to allow processes to complete
sleep 10

# Check if files are identical between different clients
diff ./client1/01.txt ./client2/01.txt
res1=$?

diff ./client2/22.txt ./client3/22.txt
res2=$?

# Report the results based on file comparison
if [ $res1 -eq 0 -a $res2 -eq 0 ]; then
    echo "Succeed"
else
    echo "Fail"
fi

# Kill the test_client and test_server processes
pkill test_client
pkill test_server

# Remove specific files
rm ./client2/01.txt
rm ./client3/22.txt