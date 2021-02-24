# udp-client-server
This program is an experimental program of UDP Client-Server file transfer.

There are two files in this repo :
* client.c , a client file
* server.c , a server file

## Scenario
The scenario of this program is the client (with client.c script) sends a request to the server (with server.c script) with a name + format of a certain file.

## How To run the program
To run this program, you need to compile them first:
```
gcc client.c -o client
gcc server.c -o server
```
The to run the program, you can use this command :
```
./server
./client [ip address of server] [filename.format]
```

## BUGS !!
In this code, there is still a bug on how the to indicate that the requested sending is finished.
So, the client program always run even if the file is sent
