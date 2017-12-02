#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <string>
#include <unordered_map>

#include "file_handler.h"

class client
{
public:
    int init(std::string configuration_file_path);
    int start();
    int send_data_to_server(std::string data);
    int _exit();
    bool get_command_error()
    {
        return command_error;
    }
private:
    std::string configuration_file_path;
    std::unordered_map<std::string, std::string> configuration_map;
    bool command_error;
    static int sockfd;
    static void * process_connection(void *arg);
    static void handle_command_from_server(int sockfd, std::string command);
    static void sigint_handler(int signal);
    static client *_client;
};

#endif
