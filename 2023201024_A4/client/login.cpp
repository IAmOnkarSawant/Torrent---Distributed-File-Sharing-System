#include "helper.cpp"

bool logged_in;
string peer_ip;
int peer_port;
bool isCorruptedFile;
unordered_map<string, string> downloaded_files;
unordered_map<string, unordered_map<string, bool>> is_uploaded;
unordered_map<string, vector<int>> file_chunk_info;
vector<vector<string>> cur_down_file_chunks;
unordered_map<string, string> file_to_file_path;
vector<string> cur_file_piece_wise_Hash;

struct Socket
{
    char *ip_add;
    int port;

    Socket()
    {
        ip_add = nullptr;
        port = 0;
    }

    void setsocketdata(string socket)
    {
        string temp;
        vector<string> tokens;
        stringstream parse(socket);

        while (getline(parse, temp, ':'))
        {
            tokens.push_back(temp);
        }

        string ip = tokens[0];
        ip_add = new char[ip.length() + 1];

        strcpy(ip_add, ip.c_str());
        port = stoi(tokens[1]);
    }

    ~Socket()
    {
        delete[] ip_add;
    }
};

int processCMD(vector<string> inpt, int socket)
{
    char server_reply[10240];
    bzero(server_reply, 10240);
    read(socket, server_reply, 10240);
    cout << server_reply << "\n";

    string s1 = string(server_reply);
    s1 = s1.substr(0, 23);

    if (s1 == "Invalid argument count")
    {
        cout << server_reply << endl;
        return 0;
    }
    if (string(server_reply) == "Invalid Command")
    {
        cout << "Received: Invalid Command" << endl;
        return 0;
    }
    if (string(server_reply) == "please login to continue")
    {
        cout << "Received: please login to continue" << endl;
        return 0;
    }

    if (inpt[0] == "login")
    {
        if (string(server_reply) == "0Login Successful")
        {
            logged_in = true;
        }
    }
    else if (inpt[0] == "logout")
    {
        logged_in = false;
    }
    else if (inpt[0] == "list_groups")
    {
        return print_groups(server_reply);
    }
    else if (inpt[0] == "list_requests")
    {
        return print_requests(server_reply);
    }
    else if (inpt[0] == "upload_file")
    {
        return upload_file_fun(server_reply, inpt, socket);
    }
    else
    {
        cout << "\n";
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <IP>:<PORT> tracker_info.txt" << endl;
        return 1;
    }
    string tracker_file_name = argv[2];
    FILE *file = fopen(tracker_file_name.c_str(), "r");
    vector<string> lines;

    if (file != nullptr)
    {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file) != nullptr)
        {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
            {
                buffer[len - 1] = '\0';
            }
            lines.emplace_back(buffer);
        }
        fclose(file);
    }
    else
    {
        cerr << "File not found or cannot be opened: " << tracker_file_name << "\n";
        exit(0);
    }

    string soc = lines[0] + ":" + lines[1];
    string soc1 = lines[2] + ":" + lines[3];

    struct Socket Mastertracer, Subtracker;
    Mastertracer.setsocketdata(soc);
    Subtracker.setsocketdata(soc1);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(Mastertracer.port);

    if (inet_pton(AF_INET, Mastertracer.ip_add, &(server_address.sin_addr)) <= 0)
    {
        perror("Invalid server IP address");
        return 1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Connection failed");
        return 1;
    }

    cout << "Connected to the server." << endl;

    while (true)
    {
        cout << ">> ";
        string inputline, s;
        getline(cin, inputline);
        if (inputline.length() < 1)
            continue;

        stringstream ss(inputline);
        vector<string> input;
        while (ss >> s)
        {
            input.push_back(s);
        }

        if (input[0] == "quit" || input[0] == "QUIT" || input[0] == "Quit")
            break;
        if (send(client_socket, &inputline[0], strlen(&inputline[0]), MSG_NOSIGNAL) == -1)
        {
            printf("Error: %s\n", strerror(errno));
            return -1;
        }

        processCMD(input, client_socket);
    }

    close(client_socket);

    return 0;
}
