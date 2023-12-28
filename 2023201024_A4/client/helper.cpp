#include "libraries.h"


long long file_size(char *path)
{
    FILE *f = fopen(path, "rb");
    long long s = -1;

    if (f)
    {
        fseek(f, 0, SEEK_END);
        s = ftell(f) + 1;
        fclose(f);
    }
    else
    {
        cout << "File not found .\n";
        return -1;
    }
    return s;
}

void get_string_hash(string s, string &hash)
{
    unsigned char a[20];
    if (!SHA1(reinterpret_cast<const unsigned char *>(&s[0]), s.length(), a))
    {
        printf("Error in hashing\n");
    }
    else
    {
        for (int i = 0; i < 20; i++)
        {
            char buffer[3];
            sprintf(buffer, "%02x", a[i] & 0xff);
            hash += string(buffer);
        }
    }
    hash += "$$";
}

string get_hash(char *path)
{
    FILE *fp;
    long long file_Size = file_size(path);

    if (file_Size == -1)
    {
        return "$";
    }

    int segments = file_Size / SEGMENT_SIZE + 1;
    char line[SIZE + 1];
    string hash = "";

    fp = fopen(path, "r");
    int i;
    int all;

    if (fp)
    {
        for (i = 0; i < segments; i++)
        {
            int rc;
            all = 0;
            string segment_string = "";

            while (all < SEGMENT_SIZE && (rc = fread(line, 1, min(SIZE - 1, SEGMENT_SIZE - all), fp))) // pb
            {
                line[rc] = '\0';
                all = all + strlen(line);

                segment_string = segment_string + line;
                memset(line, 0, sizeof(line));
            }

            get_string_hash(segment_string, hash);
        }
        fclose(fp);
    }
    else
    {
        printf("File not found.\n");
    }
    hash.pop_back();
    hash.pop_back();
    return hash;
}

// string getFileHash(char *path)
// {
//     ostringstream buf;
//     ifstream input(path);
//     buf << input.rdbuf();
//     string contents = buf.str(), hash;

//     unsigned char md[SHA256_DIGEST_LENGTH];
//     if (!SHA256(reinterpret_cast<const unsigned char *>(&contents[0]), contents.length(), md))
//     {
//         printf("Error in hashing\n");
//     }
//     else
//     {
//         for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
//         {
//             char buf[3];
//             sprintf(buf, "%02x", md[i] & 0xff);
//             hash += string(buf);
//         }
//     }
//     return hash;
// }

string get_file_Hash(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        cout << "File not found." << endl;
        return "";
    }

    ostringstream buffer;
    char tempBuffer[1024];
    while (size_t bytesRead = fread(tempBuffer, 1, sizeof(tempBuffer), file))
    {
        buffer.write(tempBuffer, bytesRead);
    }

    fclose(file);

    string contents = buffer.str();
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(contents.c_str()), contents.size(), md);

    stringstream hash;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        hash << hex << setw(2) << setfill('0') << static_cast<int>(md[i]);
    }

    return hash.str();
}


void splitString(const string &input, const string &delimiter, vector<string> &tokens)
{
    size_t start = 0, end = 0;
    while (end != string::npos)
    {
        end = input.find(delimiter, start);

        if (end == string::npos)
        {
            tokens.push_back(input.substr(start));
        }
        else
        {
            tokens.push_back(input.substr(start, end - start));
        }

        if (end != string::npos)
        {
            start = end + delimiter.length();
        }
    }
}

int print_groups(const string &server_reply)
{
    cout << "\n---------- List of Groups ----------\n";
    vector<string> split_vector;
    const string delimiter = "+++";

    splitString(server_reply, delimiter, split_vector);

    if (split_vector.empty() || split_vector[0] == "- No groups yet")
    {
        cout << "\n- No groups yet\n";
    }
    else
    {
        for (int i = 0; i < split_vector.size() - 1; i++)
        { // Update the loop condition here
            cout << i + 1 << ": " << split_vector[i] << "\n";
        }
    }
    cout << "------------------------------------\n";
    return 1;
}

int print_requests(const string &server_reply)
{
    if (string(server_reply) == "You are not the admin of this group")
    {
        cout << "You are not the admin of this group" << endl;
    }
    else if (string(server_reply) == "No pending requests for this group")
    {
        cout << "No pending requests" << endl;
    }
    else
    {
        vector<string> requests;
        string response(server_reply);
        splitString(response, "+++", requests);

        cout << "\n---------- Pending Requests ----------\n";
        for (size_t i = 0; i < requests.size() - 1; i++)
        {
            cout << i + 1 << ": " << requests[i] << "\n";
        }
        cout << "------------------------------------\n";
    }
    return 0;
}

void set_chunk_vector(string filename, long long l, long long r, bool isUpload)
{
    if (isUpload)
    {
        vector<int> tmp(r - l + 1, 1);
        file_chunk_info[filename] = tmp;
    }
    else
    {
        file_chunk_info[filename][l] = 1;
        // writeLog("chunk vector updated for " + filename + " at " + to_string(l));
    }
}

int upload_file(vector<string> input, int socket)
{
    if (input.size() != 3)
    {
        return 0;
    }
    string file_details = "";
    char *file_path = &input[1][0];
    string FILE_DELIMITER = "$$";
    string SERVER_ERROR_MSG = "error";
    vector<string> tokens;
    splitString(string(file_path), "/", tokens);
    string filename = tokens.back();

    if (is_uploaded[input[2]].find(filename) != is_uploaded[input[2]].end())
    {
        cout << "File already uploaded" << endl;
        if (send(socket, SERVER_ERROR_MSG.c_str(), strlen(SERVER_ERROR_MSG.c_str()), MSG_NOSIGNAL) == -1)
        {
            printf("Error: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }
    else
    {
        is_uploaded[input[2]][filename] = true;
        file_to_file_path[filename] = string(file_path);
    }
    
    string piecewise_hash = get_hash(file_path);

    if (piecewise_hash == "$")
    {
        return 0;
    }

    file_details += string(file_path) + FILE_DELIMITER;
    // cout<<"file hash: "<<file_hash<<"\n";

    long long fsize = file_size(file_path);
    string file_size = to_string(fsize);
    file_details += file_size + FILE_DELIMITER;

    // file_details += string(peer_ip) + ":" + to_string(peer_port) + FILE_DELIMITER;
    string file_hash = get_file_Hash(file_path);
    file_details += file_hash + FILE_DELIMITER;

    file_details += piecewise_hash;

    cout<<"file details: "<<file_details<<"\n";

    // writeLog("sending file details for upload: " + file_details);
    if (send(socket, &file_details[0], strlen(&file_details[0]), MSG_NOSIGNAL) == -1)
    {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }
    
    cout<<"file info sent to server\n";
    char server_reply[124000] = {0};
    read(socket, server_reply, 124000);
    cout << server_reply << endl;
    // writeLog("server reply for send file: " + string(server_reply));

    set_chunk_vector(filename, 0, stoll(file_size) / SEGMENT_SIZE + 1, true);

    return 0;
}

int upload_file_fun(const string &server_reply, vector<string> input, int socket)
{
    string sr = string(server_reply);
    if (sr == "Group not found")
    {
        return 0;
    }
    else if (sr == "Not a member of this group")
    {
        return 0;
    }
    else if (sr == "File does not exist")
    {
        return 0;
    }
    return upload_file(input, socket);
}

void handle_connection(int client_socket, sockaddr_in client_add)
{
    string client_uid = "";
    string client_gid = "";

    string client_ip = inet_ntoa(client_add.sin_addr);
    int client_port = ntohs(client_add.sin_port);
    string socket_client_add = client_ip + ":" + to_string(client_port);

    // listning to the commands sent by client
    while (true)
    {
        char buffer[1024] = {0};

        // socket not able to read
        if (read(client_socket, buffer, 1024) <= 0)
        {
            close(client_socket);
            break;
        }

        // valid socket
        string s;
        string in = string(buffer);
        stringstream ss(in);
        vector<string> input;

        while (ss >> s)
        {
            // cout<<s<<" ";
            input.push_back(s);
        } // cout<<"\n";

        cout << input[0] << " => \n";
    }
}

void *check_input(void *arg)
{
    while (true)
    {
        string buffer;
        getline(cin, buffer);
        if (buffer == "quit" || buffer == "Quit" || buffer == "QUIT")
        {
            cout<<"==========================================\n";
            exit(0);
        }
    }
}

void* run_as_server(void* arg)
{
    int tracer_fd, new_socket, o = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t is_exit_thread_m, is_exit_thread_s;

    if ((tracer_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("creation of socket file desciptor failed");
        exit(0);
    }

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr(peer_ip.c_str());
    address.sin_port = htons(peer_port); 

    if (setsockopt(tracer_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &o, sizeof(o))) { 
        perror("setsockopt: "); 
        exit(EXIT_FAILURE); 
    } 

    if(inet_pton(AF_INET, &peer_ip[0], &address.sin_addr)<=0)  { 
        printf("\nGiven address is invalid \n"); 
        return NULL; 
    } 
       
    if (bind(tracer_fd, (sockaddr *)&address,  sizeof(address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (listen(tracer_fd, 5) == -1)
    {
        perror("Failed to listen on clients");
        exit(0);
    }

    cout<<"==========================================\n";
    cout << "\nClient is Listening..........."
         << "\n\n";

    vector<thread> threads;
    if (pthread_create(&is_exit_thread_m, NULL, check_input, NULL) == -1)
    {
        perror("pthread");
        exit(0);
    }

    while (1)
    {
        sockaddr_in client_add;
        socklen_t client_add_size = sizeof(client_add);

        int client_socket;
        if ((client_socket = accept(tracer_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Error in accept connection");
            exit(0);
        }

        threads.push_back(thread(handle_connection, client_socket, address));
    }

    for (auto i = threads.begin(); i != threads.end(); i++)
    {
        if (i->joinable())
        {
            i->join();
        }
    }
    // close(server_socket);
}