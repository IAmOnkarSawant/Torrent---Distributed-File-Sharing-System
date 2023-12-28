#include "libraries.h"
//************************************************//
//                  STRUCTURES                    //
//************************************************//
struct Socket
{
    char *ip_add; 
    int port; 

    // Constructor
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

    ~Socket() {
    delete[] ip_add;
}
};