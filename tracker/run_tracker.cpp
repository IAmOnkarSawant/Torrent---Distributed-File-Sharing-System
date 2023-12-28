#include "libraries.h"
#include "helpers.cpp"
#include "data_structures.h"

unordered_map<string, string> login_creds;
unordered_map<string, bool> is_logged_in;
unordered_map<string, string> uname_to_port;
unordered_map<string, string> gid_to_admin;
unordered_map<string, vector<string>> gid_to_mems;
unordered_map<string, vector<string>> gid_to_pending;
unordered_map<string, unordered_map<string, set<string>>> seeder_list;
unordered_map<string, string> piece_wise_hash;
unordered_map<string, string> file_size;

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
            is_logged_in[client_uid] = false;
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
        if (input[0] == "create_user")
        {
            if (input.size() != 3)
            {
                write(client_socket, "Invalid argument count \n Required: create_user <username> <password>", 69);
            }
            else
            {
                if (create_user(input) < 0)
                {
                    write(client_socket, "User exists", 11);
                }
                else
                {
                    write(client_socket, "Account created", 15);
                }
            }
        }
        else if (input[0] == "login")
        {
            if (input.size() != 3)
            {
                write(client_socket, "Invalid argument count \nRequired: create_user <username> <password>", 68);
            }
            else
            {
                int response = 5;
                response = validate(input);
                write(client_socket, to_string(response).c_str(), 1);
                if (response == -2)
                {
                    write(client_socket, "Account does not exists, please create account!", 47);
                }
                else if (response == -1)
                {
                    write(client_socket, "Username or Password do not match", 33);
                }
                else if (response == 1)
                {
                    write(client_socket, "You already have one active session", 35);
                }
                else
                {
                    write(client_socket, "Login Successful", 17);
                    cout << "login done ";
                    client_uid = input[1];

                    string peer_address = socket_client_add;
                    uname_to_port[client_uid] = peer_address;
                    cout << peer_address << "\n";
                }
            }
        }
        else if (input[0] == "logout")
        {
            if (input.size() != 1)
            {
                write(client_socket, "Invalid argument count \n Required: logout", 42);
            }
            else
            {
                is_logged_in[client_uid] = false;
                write(client_socket, "Logged Out", 11);
            }
        }
        else if(client_uid == "" || (is_logged_in.find(client_uid) != is_logged_in.end() && !is_logged_in[client_uid]))
        {
            write(client_socket, "please login to continue", 25);
        }
        else if (input[0] == "create_group")
        {
            if (input.size() != 2)
            {
                write(client_socket, "Invalid argument count \nRequired: Create_Group <groupID>", 57);
            }
            else
            {
                int res = create_group(client_uid, input[1]);
                if (res == -1)
                {
                    write(client_socket, "group already exists! ", 23);
                }
                else
                {
                    write(client_socket, "group created successfully! ", 29);
                }
            }
        }
        else if (input[0] == "list_groups")
        {
            if (input.size() != 1)
            {
                write(client_socket, "Invalid argument count \nRequired: list_groups", 46);
            }
            else
            {
                string groups = "";
                int groups_size = 0;
                for (auto i : gid_to_mems)
                    groups = (groups + i.first + "+++"); // delemeter (so we can split at client side)
                groups_size = groups.length();

                if (groups_size == 0)
                    write(client_socket, "- No groups yet", 16);
                else
                    write(client_socket, groups.c_str(), groups_size);
            }
        }
        else if (input[0] == "join_group")
        {
            if (input.size() != 2)
            {
                write(client_socket, "Invalid argument count \nRequired: join_group <groupid>", 55);
            }
            else
            {
                // Checking if gid exists
                if (gid_to_mems.find(input[1]) == gid_to_mems.end())
                {
                    write(client_socket, "This group doesn't exist", 24);
                }
                else
                {
                    // Check if the client is already a member
                    bool isMember = false;
                    // cout<<"member of gid: "<<input[1]<<"\n";
                    for (const string &member : gid_to_mems[input[1]])
                    {
                        // cout<<member<<" ";
                        if (member == client_uid)
                        {
                            isMember = true;
                            break;
                        }
                    } // cout<<"\n";

                    // Check if the client has already sent a request
                    bool hasPendingRequest = false;
                    for (const string &pending : gid_to_pending[input[1]])
                    {
                        if (pending == client_uid)
                        {
                            hasPendingRequest = true;
                            break;
                        }
                    }

                    if (isMember)
                    {
                        write(client_socket, "You are already a member of the group!", 37);
                    }
                    else if (hasPendingRequest)
                    {
                        write(client_socket, "You have already sent a request!", 31);
                    }
                    else
                    {
                        // Send a request to join the group
                        gid_to_pending[input[1]].push_back(client_uid);
                        write(client_socket, "Request sent successfully", 26);
                    }
                }
            }
        }
        else if (input[0] == "list_requests")
        {
            list_requests(input, client_socket, client_uid);
        }
        else if (input[0] == "accept_request")
        {
            accept_request(input, client_socket, client_uid);
        }
        else if (input[0] == "leave_group")
        {
            if (input.size() != 2)
            {
                // Invalid argument count
                write(client_socket, "Invalid argument count \nRequired: leave_group <group_id>", 57);
            }
            else
            {
                string group_id = input[1];
                // Check if the group exists
                if (gid_to_mems.find(group_id) != gid_to_mems.end())
                {
                    vector<string> &members = gid_to_mems[group_id];
                    // Check if the user is a member of the group
                    auto it = find(members.begin(), members.end(), client_uid);
                    if (it != members.end())
                    {
                        // User is a member, remove them from the group
                        members.erase(it);
                        write(client_socket, "You have left the group", 24);
                    }
                    else
                    {
                        // User is not a member of the group
                        write(client_socket, "You are not a member of this group", 35);
                    }
                }
                else
                {
                    // Group does not exist
                    write(client_socket, "The group does not exist", 25);
                }
            }
        }
        else if (input[0] == "upload_file")
        {
            if (input.size() != 3)
            {
                write(client_socket, "Invalid argument count \nRequired: upload_file <file_path> <group_id>", 69);
            }
            else
            {
                string group_id = input[2];
                vector<string> group_members = gid_to_mems[group_id];
                auto is_member = find(group_members.begin(), group_members.end(), client_uid);

                struct stat buffer;
                bool pathExists = (stat(input[1].c_str(), &buffer) == 0);

                if (gid_to_mems.find(group_id) == gid_to_mems.end())
                {
                    write(client_socket, "Group not found", 16);
                }
                else if (is_member == group_members.end())
                {
                    write(client_socket, "Not a member of this group", 27);
                }
                else if (!pathExists)
                {
                    write(client_socket, "File does not exist", 20);
                }
                else
                {
                    char file_details[SEGMENT_SIZE] = {0};
                    write(client_socket, "Uploading...", 12);

                    // Read the uploaded file data from the client socket
                    if (read(client_socket, file_details, SEGMENT_SIZE))
                    {
                        if (string(file_details) == "error")
                        {
                            return;
                        }

                        vector<string> file_details_vec;
                        int pos = 0;
                        string fd = file_details;

                        while ((pos = fd.find("$$")) != string::npos)
                        {
                            string t = fd.substr(0, pos);
                            file_details_vec.push_back(t);
                            fd.erase(0, pos + 2);
                        }

                        file_details_vec.push_back(fd);
                        file_details_vec.insert(file_details_vec.begin() + 1, socket_client_add);
                        for (int i = 0; i < file_details_vec.size(); i++)
                        {
                            cout << file_details_vec[i] << " ";
                        }
                        cout << "\n";

                        // file_details_vec = [file_path, peer_address, file_size, file_hash, piecewise_hash]
                        vector<string> file_path_vec;
                        pos = 0;
                        fd = string(file_details_vec[0]);
                        while ((pos = fd.find("/")) != string::npos)
                        {

                            string t = fd.substr(0, pos);
                            file_path_vec.push_back(t);
                            fd = fd.erase(0, pos + 1); // Update fd by reassigning the result of erase
                        }

                        file_path_vec.push_back(string(file_details_vec[0]));
                        string file_name = file_path_vec[file_path_vec.size() - 1];

                        string hash_of_pieces = "";
                        for (int i = 4; i < file_details_vec.size(); i++)
                        {

                            hash_of_pieces += file_details_vec[i];
                            if (i != file_details_vec.size() - 1)
                                hash_of_pieces += "$$";
                        }

                        piece_wise_hash[file_name] = hash_of_pieces;

                        // Add the client as a seeder for this file in the group
                        seeder_list[group_id][file_name].insert(client_uid);
                        file_size[file_name] = file_details_vec[2];

                        write(client_socket, "Uploaded", 8);

                        // cout<<"seeders list\n";
                        // set<string> se = seeder_list[group_id][file_name];
                        // for (auto it = se.begin(); it != se.end(); ++it)
                        // {
                        //     cout << *it << "  ";
                        // }
                    }
                }
            }
        }
        else if (input[0] == "download_file")
        {
            download_file(input, client_socket, client_uid);
        }
        else if (input[0] == "list_files")
        {
            list_files(input, client_socket);
        }
        else if (input[0] == "stop_share")
        {
            stop_share(input, client_socket, client_uid);
        }
        else
        {
            write(client_socket, "Invalid Command", 15);
        }
    }
    close(client_socket);
}

void compileAndRunSubtracker(string ipandport)
{
    string sourceCode = "./subtracker.cpp ";
    string outputBinary = "subtracker ";
    string compileCommand = "g++ " + sourceCode + " -o " + outputBinary;
    int compileResult = system(compileCommand.c_str());

    cout << "compile done\n";
    if (compileResult != 0)
    {
        std::cerr << "Compilation failed." << std::endl;
        return;
    }

    string runCommand = "xdg-open  './" + outputBinary + " " + ipandport + "'";
    system(runCommand.c_str());
}

void runSubtracker(string Subtracker)
{
    cout << Subtracker << endl;

    compileAndRunSubtracker(Subtracker);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Please correct the format, please refer following";
        cerr << "\nUsage: \n"
             << argv[0] << "  <tracker info file name> <tracker_number>\n\n";
        return 1;
    }

    string tracker_file_name = argv[1];
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

    // for (int i = 0; i < lines.size(); i++)
    // {
    //     cout<<lines[i]<<" ";
    // }

    string soc = lines[0] + ":" + lines[1];
    string soc1 = lines[2] + ":" + lines[3];

    struct Socket Mastertracer, Subtracker;
    Mastertracer.setsocketdata(soc);
    Subtracker.setsocketdata(soc1);

    // thread subtracker_thread(runSubtracker, string(argv[1]));

    // creating socket to listen to client on port 8000
    int tracer_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t is_exit_thread_m, is_exit_thread_s;

    // Creating socket file descriptor
    if ((tracer_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("creation of socket file desciptor failed");
        exit(0);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(Mastertracer.ip_add);
    address.sin_port = htons(Mastertracer.port);

    // associate a socket with IP and Port
    if (bind(tracer_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("binding socket failed");
        exit(0);
    }

    // passive socket, which means it will be used to accept incoming connections upto backlog parameter = 5
    if (listen(tracer_fd, 5) == -1)
    {
        perror("Failed to listen on clients");
        exit(0);
    }

    cout<<"==========================================\n";
    cout << "\nTacker is Listening..........."
         << "\n\n";

    vector<thread> threads;
    // Creating 1 thread to detect if "quit" is written as input to exit.
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

        // char buffer[1024];
        // while (1)
        // {
        //     int valread = read(new_socket, buffer, sizeof(buffer));
        //     if (valread <= 0) {
        //         perror("Connection closed or error");
        //         break;
        //     }
        //     buffer[valread] = '\0';
        //     cout << "Client: " << buffer << endl;

        //     // Send a reply
        //     string reply;
        //     cout << "You: ";
        //     getline(cin, reply);
        //     send(new_socket, reply.c_str(), reply.size(), 0);
        // }
    }
    for (auto i = threads.begin(); i != threads.end(); i++)
    {
        if (i->joinable())
        {
            i->join();
        }
    }
    // subtracker_thread.join();
}
