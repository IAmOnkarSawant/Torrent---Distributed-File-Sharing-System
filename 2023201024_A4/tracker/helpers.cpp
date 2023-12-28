#include "libraries.h"
using namespace std;

// thread to contineously check for quit
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

// login credentials validation
int validate(vector<string> input)
{
    string username = input[1];
    string password = input[2];
    // account does not exists
    if (login_creds.find(username) == login_creds.end())
        return -2;

    // credentials do not match
    if (login_creds[username] != password)
        return -1;

    // check if already logged in
    if (is_logged_in.find(username) == is_logged_in.end())
        is_logged_in.insert({username, true});

    else if (is_logged_in[username] == 1)
        // already logged in
        return 1;
    else
        // now logging in
        is_logged_in[username] = 1;

    return 0;
}

// create user if not present
int create_user(vector<string> input)
{
    string username = input[1];
    string password = input[2];

    // user exists
    if (login_creds.find(username) != login_creds.end())
    {
        return -1;
    }
    login_creds.insert({username, password});
    return 0;
}

// crete groups
int create_group(string username, string gid)
{
    if (gid_to_mems.find(gid) != gid_to_mems.end())
        return -1;

    gid_to_admin[gid] = username;
    gid_to_mems[gid].push_back(username);
    return 0;
}

void list_requests(const vector<string> &input, int client_socket, const string &client_uid)
{
    // Check if the input has the correct number of arguments
    if (input.size() != 2)
    {
        write(client_socket, "Invalid argument count \nRequired: list_requests <group_id>", 59);
        return;
    }

    // Check if the client is the admin of the group
    if (gid_to_admin.find(input[1]) == gid_to_admin.end() || gid_to_admin[input[1]] != client_uid)
    {
        write(client_socket, "Admin of this group is someone else", 36);
        return;
    }

    // Check if there are pending requests for the group
    if (gid_to_pending.find(input[1]) == gid_to_pending.end() || gid_to_pending[input[1]].empty())
    {
        write(client_socket, "No pending requests for this group", 35);
        return;
    }

    // Send the list of pending requests to the client
    string reply = "";
    for (const string &request : gid_to_pending[input[1]])
    {
        reply += request + "+++";
    }

    write(client_socket, reply.c_str(), reply.length());
}

void accept_request(vector<string> input, int client_socket, string client_uid)
{
    if (input.size() != 3)
    {
        write(client_socket, "Invalid argument count \nRequired: accept_request <groupid> <username>", 22);
    }
    else
    {
        // Check if the group exists
        if (gid_to_mems.find(input[1]) == gid_to_mems.end())
        {
            write(client_socket, "This group doesn't exist", 24);
        }
        else
        {
            // Check if the client sending the request is the admin of the group
            if (client_uid != gid_to_admin[input[1]])
            {
                write(client_socket, "Admin of this group is someone else", 36);
            }
            else
            {
                // Check if the request is pending
                bool requestFound = false;
                for (string &pending : gid_to_pending[input[1]])
                {
                    if (pending == input[2])
                    {
                        requestFound = true;
                        break;
                    }
                }
                if (requestFound)
                {
                    // Add the user to the group's members
                    gid_to_mems[input[1]].push_back(input[2]);
                    // Remove the user from the pending requests
                    gid_to_pending[input[1]].erase(
                        remove(gid_to_pending[input[1]].begin(), gid_to_pending[input[1]].end(), input[2]),
                        gid_to_pending[input[1]].end());
                    write(client_socket, "Request accepted successfully", 29);
                }
                else
                {
                    write(client_socket, "Request not found or already accepted", 38);
                }
            }
        }
    }
}

void list_files(const vector<string> &inpt, int client_socket)
{
    if (inpt.size() != 2)
    {
        write(client_socket, "Invalid argument count \nRequired: list_files <group_id>", 56);

        return;
    }
    const string &group_id = inpt[1];

    write(client_socket, "Fetching files...", 17);

    // writeLog("Dummy read");

    if (gid_to_admin.find(group_id) == gid_to_admin.end())
    {
        write(client_socket, "Invalid group ID.", 18);
    }
    else if (seeder_list.find(group_id) == seeder_list.end() || seeder_list[group_id].empty())
    {
        write(client_socket, "No files found.", 16);
    }
    else
    {
        // writeLog("In the else block of list files");

        string reply = "";

        const auto &files = seeder_list[group_id];
        for (const auto &entry : files)
        {
            reply += entry.first + "$$";
        }
        reply = reply.substr(0, reply.length() - 2);
        // writeLog("List of files reply:" + reply);

        write(client_socket, reply.c_str(), reply.length());
    }
}

void stop_share(const vector<string> &inpt, int client_socket, const string &client_uid)
{
    if (inpt.size() != 3)
    {
        write(client_socket, "Invalid argument count \nRequired: stop_share <group_id> <file_name>", 68);
        return;
    }

    const string &group_id = inpt[1];
    const string &file_name = inpt[2];

    if (gid_to_admin.find(group_id) == gid_to_admin.end())
    {
        write(client_socket, "Invalid group ID.", 18);
    }
    else if (seeder_list.find(group_id) == seeder_list.end() || seeder_list[group_id].find(file_name) == seeder_list[group_id].end())
    {
        write(client_socket, "File not yet shared in the group", 33);
    }
    else
    {
        seeder_list[group_id][file_name].erase(client_uid);
        if (seeder_list[group_id][file_name].empty())
        {
            seeder_list[group_id].erase(file_name);
        }
        write(client_socket, "Stopped sharing the file", 25);
    }
}

void download_file(vector<string> input, int client_socket, string client_uid)
{
    struct stat buffer;
    bool path_exists = 0;
    if (stat(input[1].c_str(), &buffer) == 0)
        path_exists = 1;

    string gid = input[1];
    string file_name = input[2];
    string dest_path = input[3];

    bool is_member = 0;
    vector<string> mems;
    if (gid_to_mems.find(gid) == gid_to_mems.end())
        mems = gid_to_mems[gid];

    for (int i = 0; i < mems.size(); i++)
    {
        if (mems[i] == client_uid)
        {
            is_member = 1;
            break;
        }
    }

    if (input.size() != 4)
    {
        write(client_socket, "Invalid argument count \n Required: download_file <group_id> <file_name> <destination_path>", 91);
    }
    else if (gid_to_mems.find(gid) == gid_to_mems.end())
    {
        write(client_socket, "Error: Group not found.", 22);
    }
    else if (!is_member)
    {
        write(client_socket, "Error: You are not a member of this group.", 41);
    }
    else
    {
        if (!path_exists)
        {
            write(client_socket, "Error: The destination path does not exist.", 44);
            return;
        }

        char file_details[SEGMENT_SIZE] = {0};
        write(client_socket, "Downloading...", 13);

        if (read(client_socket, file_details, SEGMENT_SIZE))
        {
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

            string reply = "";
            if (seeder_list[gid].find(file_details_vec[0]) != seeder_list[gid].end())
            {
                for (auto i : seeder_list[gid][file_details_vec[0]])
                {
                    if (is_logged_in[i])
                    {
                        reply += uname_to_port[i] + "$$";
                    }
                }
                reply += file_size[file_details_vec[0]];
                write(client_socket, &reply[0], reply.length());

                write(client_socket, &piece_wise_hash[file_details_vec[0]][0], piece_wise_hash[file_details_vec[0]].length());

                seeder_list[gid][file_details_vec[0]].insert(client_uid);
            }
            else
            {
                write(client_socket, "Error: File not found in the group.", 34);
            }
        }
    }
}
