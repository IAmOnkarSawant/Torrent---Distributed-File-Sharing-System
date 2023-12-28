#include <bits/stdc++.h>
#include <sstream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <iomanip>
#include <pthread.h>
#define SIZE 99997
#define SEGMENT_SIZE 512000
using namespace std;

extern bool logged_in;
extern string peer_ip;
extern int peer_port;
extern bool isCorruptedFile;
                   
extern unordered_map<string, string> downloaded_files;
                // group id  file_name  uploaded_or_not
extern unordered_map<string, unordered_map<string, bool>> is_uploaded; 
                    // file  info of chuncks
extern unordered_map<string, vector<int>> file_chunk_info;
extern vector<vector<string>> cur_down_file_chunks;
extern unordered_map<string, string> file_to_file_path;
extern vector<string> cur_file_piece_wise_Hash;