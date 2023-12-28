//************************************************//
//                 HEADERS                        //
//************************************************//
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
#include <pthread.h>
#include <bits/stdc++.h>
#define SEGMENT_SIZE 512000

using namespace std;

//************************************************//
//             DATA STRUCTURES                    //
//************************************************//
                 //username  password
extern unordered_map<string, string> login_creds;
                 //username  is_logged_in?
extern unordered_map<string, bool> is_logged_in;
                 //username  ip:port
extern unordered_map<string, string> uname_to_port;
                  //groupid  admin_username
extern unordered_map<string, string> gid_to_admin;
                  //groupid  group_members_usernames 
extern unordered_map<string, vector<string>>gid_to_mems;
                  //groupid  list_of_userid
extern unordered_map<string, vector<string>>gid_to_pending;
                  //groupid             file_name  list_of_users_has_file_chunk 
extern unordered_map<string, unordered_map<string, set<string>>> seeder_list;
                 //filename  hash_of_file_attributes
extern unordered_map<string, string> piece_wise_hash; 
                 
extern unordered_map<string, string> file_size;
