#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <string>
#include <unordered_map>

class client
{
public:
    int init(std::string configuration_file_path);
    int start();
    int send_data_to_server(std::string data);
    int _exit();
private:
    std::string configuration_file_path;
    std::unordered_map<std::string, std::string> configuration_map;
    static int sockfd;
    static void * process_connection(void *arg);
    static void sigint_handler(int signal);
    static client _client;
};

#endif
