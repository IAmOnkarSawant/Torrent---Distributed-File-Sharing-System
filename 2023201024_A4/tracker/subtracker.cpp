#include "libraries.h"
#include "helpers.cpp"
#include "data_structures.h"

unordered_map<string, string> login_creds;
unordered_map<string, bool> is_logged_in;
unordered_map<string, string> uname_to_port;


void handle_connection(int client_socket)
{
    string client_uid = "";
    string client_gid = "";

    // listning to the commands sent by client
    while(true)
    {
        char buffer[1024] = {0}; 

        // socket not able to read
        if(read(client_socket , buffer, 1024) <= 0)
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

        while(ss >> s)
        {
            input.push_back(s);
        }

        if(input[0] == "create_user")
        {
            if(input.size() != 3)
            {
                write(client_socket, "Invalid argument count", 22);
                write(client_socket, "create_user <username> <password>", 33);
            }
            else{
                if(create_user(input) < 0){
                    write(client_socket, "User exists", 11);
                }
                else{
                    write(client_socket, "Account created", 15);
                }
            }
        }
        else if(input[0] == "login")
        {
            if(input.size() != 3)
            {
                write(client_socket, "Please provide proper arguments", 31);
                write(client_socket, "create_user <username> <password>", 33);
            }
            else
            {
                int response = 5; 
                response = validate(input);
                write(client_socket, to_string(response).c_str(), 1);
                if(response == -2)
                {
                    write(client_socket, "Account does not exists, please create account!", 47);
                }
                else if(response == -1)
                {
                    write(client_socket, "Username or Password do not match", 33);
                }
                else if(response == 1)
                {
                    write(client_socket, "You already have one active session", 35);
                }
                else
                {
                    write(client_socket, "login successful", 16);
                    client_uid = input[1];
                    char buffer[96];
                    read(client_socket, buffer, 96);
                    string peer_address = string(buffer);
                    uname_to_port[client_uid] = peer_address;
                }
            }            
        }
        else if(input[0] == "logout")
        {
            if(input.size() != 1)
            {
                write(client_socket, "Please provide proper arguments", 31);
                write(client_socket, "logout", 6);
            }
            else
            {
                is_logged_in[client_uid] = false;
                write(client_socket, "Logged Out", 10);
            }
        }
    }
    close(client_socket);
}

int main(int argc, char *argv[])
{
    if (argc != 2) 
    {
        cerr << "Please correct the format, please refer following";
        cerr << "\nUsage: \n" << argv[0] << "<Subtracker_ip>:<Subtracker_port>\n\n";
        return 1;
    }

    struct Socket Subtracker;
    Subtracker.setsocketdata(string(argv[1]));

    // creating socket to listen to client on port 8000
    int tracer_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t is_exit_thread_s;

    // Creating socket file descriptor
    if ((tracer_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("creation of socket file desciptor failed");
        exit(0);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(Subtracker.ip_add);
    address.sin_port = htons(Subtracker.port);

    //associate a socket with IP and Port
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

    cout<<"Tacker is Listening..........."<<"\n";

    vector<thread> threads;
    // Creating 1 thread to detect if "quit" is written as input to exit.
    if(pthread_create(&is_exit_thread_s, NULL, check_input, NULL) == -1){
        perror("pthread"); 
        exit(0); 
    }

    while (1)
    {
        int client_socket;
        if ((client_socket = accept(tracer_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Error in accept connection");
            exit(0);
        }
        else
            cout<<"Client accepted!!";

        threads.push_back(thread(handle_connection, client_socket));
        
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
    for(auto i=threads.begin(); i!=threads.end(); i++)
    {
        if(i->joinable()) 
        {
            i->join();
        }
    }

}

