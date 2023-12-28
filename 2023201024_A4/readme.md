# Distributed File Sharing System

## Prerequisites

1. OpenSSL library

    ```bash
        sudo apt-get install openssl 
     ```

## Platform:
1. Linux <br/>

## How To Run The Program

1. Extract the files.
2. you can now see two folders Client and Server
3. Open a terminal and run following commands
```
cd client
g++ -o login login.cpp -lssl -lcrypto
./login 127.0.0.1:5050 tracker_info.txt
cd ..
```
4. Now open a new terminal and run following commands 
```
cd tracker
g++ -o tracker run_tracker.cpp
./tracker tracker_info.txt 1
cd ..
```

## Usage

### Tracker

1. Run Tracker:

```bash
cd tracker
./tracker​ <TRACKER INFO FILE> <TRACKER NUMBER>
ex: ./tracker tracker_info.txt 1
```

`<TRACKER INFO FILE>` contains the IP, Port details of all the trackers.

```bash
Ex:
127.0.0.1
5000
127.0.0.1
6000
```

2. Close Tracker:

```bash
quit || Quit || QUIT
```

### Client:

1. Run Client:

```bash
cd client
./client​ <IP>:<PORT> <TRACKER INFO FILE>
ex: ./client 127.0.0.1:18000 tracker_info.txt
```

2. Create user account:

```bash
create_user​ <user_id> <password>
```

3. Login:

```bash
login​ <user_id> <password>
```

4. Create Group:

```bash
create_group​ <group_id>
```

5. Join Group:

```bash
join_group​ <group_id>
```

6. Leave Group:

```bash
leave_group​ <group_id>
```

7. List pending requests:

```bash
list_requests ​<group_id>
```

8. Accept Group Joining Request:

```bash
accept_request​ <group_id> <user_id>
```

9. List All Group In Network:

```bash
list_groups
```

10. List All sharable Files In Group:

```bash
list_files​ <group_id>
```

11. Upload File:

```bash
​upload_file​ <file_path> <group_id​>
```

12. Stop sharing: ​

```bash
stop_share ​<group_id> <file_name>
```
13. Logout:​

```bash
logout
```

## Working

1. At Least one tracker will always be online.
2. Client needs to create an account (user_id and password) in order to be part of the
network.
3. Client can create any number of groups (group_id should be different) and hence will be
owner of those groups.
4. Client needs to be part of the group from which it wants to download the file
5. Client will send join request to join a group
6. Owner Client Will Accept/Reject the request.
7. After joining group, client can see list of all the shareable files in the group.
8. Client can share file in any group (as an owner or member; note: file will not get
uploaded to tracker but only the <ip>:<port> of the client for that file)
9. Client can send the download command to tracker with the group_id and filename, and
tracker will send the details of the group members which are currently sharing that
particular file.
10. After fetching the peer info from the tracker, client will communicate with peers about
the portions of the file they contain and hence accordingly decide which part of file to
take from which peer (You need to design your own Piece Selection Algorithm)
11. As soon as a chunk of file gets downloaded it should be available for sharing (the client
becomes a ‘leecher’)
12. After logout, the client should temporarily stop sharing their own currently shared
files/file chunks till the next login.
13. All trackers need to be in sync with each other, so that any seeding/sharing information is
available to all.

## Assumptions

1. Only one tracker is implemented and that tracker should always be online.
2. The peer can login from different IP addresses, but the details of his downloads/uploads will not be persistent across sessions.
3. SHA1 integrity checking doesn't work correctly for binary files.
4. File paths should be absolute.(whole path)
5. segment size is 512kb


In this part of code Download file functionality has not coded.