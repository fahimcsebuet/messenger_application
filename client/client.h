#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <string>
#include <unordered_map>
#include <condition_variable>
#include <mutex>

#include "file_handler.h"

class friend_info
{
public:
    friend_info(std::string user_name, std::string ip, int port)
    {
        this->user_name = user_name;
        this->ip = ip;
        this->port = port;
    }
    friend_info()
    {
        this->user_name = "";
        this->ip = "";
        this->port = -1;
    }

    std::string user_name;
    std::string ip;
    int port;
};

class client
{
public:
    int init(std::string configuration_file_path);
    int start();
    int send_data_to_server(std::string data);
    int send_location_info_to_server(std::string username);
    int _exit();
    std::vector<std::string> get_response_from_server()
    {
        return response_from_server;
    }
    std::string get_p2p_ip()
    {
        return p2p_ip;
    }
    int get_p2p_port()
    {
        return p2p_port;
    }
    std::unordered_map<std::string, friend_info> get_online_friends_list()
    {
        return online_friends_list;
    }
    int start_p2p();
private:
    std::string configuration_file_path;
    std::unordered_map<std::string, std::string> configuration_map;
    bool response_received;
    std::mutex response_mutex;
    std::condition_variable response_condition_variable;
    std::vector<std::string> response_from_server;
    std::string p2p_ip;
    int p2p_port;
    std::unordered_map<std::string, friend_info> online_friends_list;
    static int sockfd;
    static void * process_connection(void *arg);
    static void handle_command_from_server(int sockfd, std::string command);
    static void sigint_handler(int signal);
    static client *_client;
    static void * process_connection_p2p(void *arg);
    std::string get_fully_qualified_domain_name();
};

#endif
