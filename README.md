GBNframework
============
Authors: Luke Connors and Kevin Colin

This package includes code for a P2P file transfer system to run over localhost. The 
server manages, the list of clients and the master list of files for each client. The 
server distributes the master list of files to each client. The server will redistribute 
the master list of files every time there is an update. Each client registers itself with 
the P2P server and registers all the files in it's working directory. Each client can then 
use the ls command to view the most recent version of the master file list. Each client 
uses the get command along with a file name, host port, and host ip address to get a file 
from another client. Two clients exchanging their files will manage a seperate connection 
to transfer the file.

Functionality included:
- clients are able to connect to the server via TCP
- clients connect and disconnect from peers appropriately in order to retrieve files.
- clients can execute the commands 'ls', 'get <file_name> <host_port> <host_ip>' , and 'exit'

When running a client:
- the 'ls' command retrieves latest file list and prints to the console in the following
  format:
	<file_name> <file_size> <host_name> <host_ip> <host_port>
- the 'get <file_name> <host_port> <host_ip>' retrives file directly from peer
- the 'exit' command exits the client gracefully

Running clients:
Each client is designed to run in it's own directory where only it's files are located. In 
the terminal be sure to cd into the directory for a client before running the client. For 
demo purposes, this package includes 3 client directories, each with one test file. The 
make file will make executables into these directories. When each client connects to the 
server, each file in it's directory will be registered with the server.

Make:
	make clean
	make

Running:
	./server_PFS <port_number>
	./client_PFS <client_name> <server_ip> <server_port>

Example:
	./server_PFS 8080
	./client_PFS steve 127.0.0.1 8080
	./client_PFS alex 127.0.0.1 8080
	./client_PFS joey 127.0.0.1 8080



