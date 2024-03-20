# Connectify
## Decription
- Connectify is a FTP (File Transfer Protocol) server that facilitates seamless file transfer between multiple clients and the server over a network.
- The server hosts files and allows multiple clients to connect, upload, and download files over TCP sockets. This code doesn't use the standard FTP protocol. This code can **only run on** `Linux (debian-based distros)`.

## Features
- `Upload and Download`: Allows users to upload files from the client to the server and download files from the server to the client.
- `Peek`: Allows client to view 1st **1kB** of his file on server
- `View & Remove`: Allows clients to see the names of his files on server and remove them.
- `Security`: Implements authentication to ensure the confidentiality of transferred files.
- `Consistency`: Server Maintains a **Log file** for every transaction


## FTP Server and Client `Setup` Guide

#### Building the Executables
1. Organize the server.c, common.c, and the Makefile in a folder [X] on PC1.
2. Organize client.c, common.c, and the Makefile in a folder [Y] on PC2.
3. Navigate to the respective folder (X/Y) and execute `make` to build server.out and client.out files.
   
#### Setting Up the Server.
```
make
```
1. `a.out` is created in the server folder.
2. Create a file named "id_passwd.txt" in folder [X], containing usernames and passwords of multiple clients.
3. Each entry in id_passwd.txt should follow the format:
```
[userName]/:[password]
```
4. In the same folder [X], create an empty directory with the same name as each client's username for every user.
5. Launch the server by running `./server.out [ServerPortNo]`.
```
./server.out [ServerPortNo]
```

#### Setting Up the Client

1. Only the client.out file is required in the client computer's folder [Y].
2. To run the client, navigate to folder [Y] and execute `./client.out [ServerIP] [ServerPort]`.
```
./client.out [ServerIP] [ServerPort]
```

Note: Replace terms inside [...] as per your system configuration.
