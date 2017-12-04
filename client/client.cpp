#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "client.h"

const unsigned MAXBUFLEN = 512;
int client::sockfd = -1;
client* client::_client = NULL;

int client::init(std::string configuration_file_path)
{
    _client = this;
    is_peer_running = false;
    response_received = false;
    signal(SIGINT, sigint_handler);
    this->configuration_file_path = configuration_file_path;
    configuration_file_handler _configuration_file_handler(configuration_file_path);
    _configuration_file_handler.load_configuration(configuration_map);

    p2p_ip = get_fully_qualified_domain_name();
    p2p_port = 5100; // Port for p2p connection
    online_friends_list = {};
    response_from_server = {};
    return EXIT_SUCCESS;
}

int client::start()
{
    int rv, flag;
    struct addrinfo hints, *res, *ressave;
    pthread_t tid;

    const char * _servhost = 
        configuration_map.find(configuration_keys::server_host) != configuration_map.end() ? 
        configuration_map.find(configuration_keys::server_host)->second.c_str()
        : NULL;

    const char * _servport = configuration_map.find(configuration_keys::server_port) != configuration_map.end() ? 
        configuration_map.find(configuration_keys::server_port)->second.c_str()
        : NULL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(_servhost, _servport, &hints, &res)) != 0)
    {
        std::cout << "getaddrinfo wrong: " << gai_strerror(rv) << std::endl;
        return EXIT_FAILURE;
    }

    ressave = res;
    flag = 0;
    do 
    {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) 
            continue;
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
        {
            flag = 1;
            break;
        }
        close(sockfd);
    } while ((res = res->ai_next) != NULL);

    freeaddrinfo(ressave);

    if (flag == 0)
    {
        fprintf(stderr, "cannot connect\n");
        return EXIT_FAILURE;
    }

    pthread_create(&tid, NULL, &process_connection, NULL);
    return EXIT_SUCCESS;
}

int client::send_data_to_server(std::string data)
{
    utility::trim_string(data);
    if(data.empty())
    {
        return EXIT_FAILURE;
    }
    if(sockfd != -1)
    {
        response_from_server.clear();
        response_received = false;
        write(sockfd, data.c_str(), data.length());
        std::cout << "Processing..." << std::endl;
        std::unique_lock<std::mutex> _response_lock(response_mutex);
        while(!response_received)
        {
            response_condition_variable.wait(_response_lock);
        }
    }
    return EXIT_SUCCESS;
}

int client::send_data_to_peer(int in_sockfd, std::string data)
{
    utility::trim_string(data);
    if(data.empty())
    {
        return EXIT_FAILURE;
    }
    if(in_sockfd != -1)
    {
        write(in_sockfd, data.c_str(), data.length());
    }
    return EXIT_SUCCESS;
}

int client::send_location_info_to_server(std::string username)
{
    char _sentinel = -1;
    std::string _command = "loc";
    std::string _data_for_server = _command + _sentinel + username + 
        _sentinel + p2p_ip + _sentinel + std::to_string(p2p_port);
    return send_data_to_server(_data_for_server);
}

int client::_exit()
{
    if(sockfd != -1) close(sockfd);
    configuration_file_handler _configuration_file_handler(configuration_file_path);
    _configuration_file_handler.save_configuration(configuration_map);
    return EXIT_SUCCESS;
}

int client::start_p2p()
{
    is_peer_running = true;
    pthread_t tid;
    pthread_create(&tid, NULL, &process_start_p2p, NULL);
    return EXIT_SUCCESS;
}

int client::stop_p2p()
{
    is_peer_running = false;
    // stop p2p may be with condition variables
    return EXIT_SUCCESS;
}

int client::connect_to_peer(std::string peer_username)
{
    std::unique_lock<std::mutex> _lock(online_friends_list_mutex);
    std::unordered_map<std::string, friend_info>::iterator _online_friends_itr =
        online_friends_list.find(peer_username);

    if(_online_friends_itr == online_friends_list.end())
    {
        std::cout << "User not available or logged in" << std::endl;
        return EXIT_FAILURE;
    }

    friend_info _peer_info = _online_friends_itr->second;

    if(_peer_info.connected)
    {
        return EXIT_SUCCESS;
    }

    int rv, flag, *sock_ptr;
    struct addrinfo hints, *res, *ressave;
    pthread_t tid;

    const char * _servhost = _peer_info.ip.c_str();
    const char * _servport = std::to_string(_peer_info.port).c_str();

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(_servhost, _servport, &hints, &res)) != 0)
    {
        std::cout << "getaddrinfo wrong: " << gai_strerror(rv) << std::endl;
        return EXIT_FAILURE;
    }

    ressave = res;
    flag = 0;
    do 
    {
        _online_friends_itr->second.sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (_online_friends_itr->second.sockfd < 0) 
            continue;
        if (connect(_online_friends_itr->second.sockfd, res->ai_addr, res->ai_addrlen) == 0)
        {
            flag = 1;
            break;
        }
        close(_online_friends_itr->second.sockfd);
    } while ((res = res->ai_next) != NULL);

    freeaddrinfo(ressave);

    if (flag == 0)
    {
        fprintf(stderr, "cannot connect\n");
        return EXIT_FAILURE;
    }

    sock_ptr = (int *)malloc(sizeof(int));
    *sock_ptr = _online_friends_itr->second.sockfd;
    pthread_create(&tid, NULL, &process_connect_to_p2p, (void*)sock_ptr);
    _online_friends_itr->second.connected = true;

    return EXIT_SUCCESS;
}

void * client::process_connection(void *arg)
{
    int n;
    char buf[MAXBUFLEN];
    pthread_detach(pthread_self());
    while (true)
    {
        n = read(sockfd, buf, MAXBUFLEN);
        if (n <= 0)
        {
            if (n == 0)
            {
                std::cout << "server closed" << std::endl;
            }
            else
            {
                std::cout << "something wrong" << std::endl;
            }
            close(sockfd);
            exit(1);
        }
        buf[n] = '\0';
        handle_command_from_server(sockfd, std::string(buf));
    }
}

void * client::process_connect_to_p2p(void *arg)
{
    int _sockfd;
    _sockfd = *((int *)arg);
    free(arg);
    int n;
    char buf[MAXBUFLEN];
    pthread_detach(pthread_self());
    while (true)
    {
        n = read(_sockfd, buf, MAXBUFLEN);
        if (n <= 0)
        {
            if (n == 0)
            {
                std::cout << "server closed" << std::endl;
            }
            else
            {
                std::cout << "something wrong" << std::endl;
            }
            close(sockfd);
            exit(1);
        }
        buf[n] = '\0';
        handle_command_from_peer(_sockfd, std::string(buf));
    }
}

void client::handle_command_from_server(int sockfd, std::string command)
{
    std::unique_lock<std::mutex> _response_lock(_client->response_mutex);
    char _sentinel = -1;
    _client->response_from_server = utility::split_string(command, _sentinel);

    if(_client->response_from_server.size() <= 1)
    {
        std::cout << "Bad Response from Server" << std::endl;
        _client->response_received = true;
        _client->response_condition_variable.notify_all();
        return;
    }

    std::string _command_operator = _client->response_from_server.at(0);
    std::string _command_message = _client->response_from_server.at(1);
    if(_command_operator == "r")
    {
        if(_command_message == "500")
        {
            std::cout << "Username not available. Try Again!" << std::endl;
        }
        else if(_command_message == "200")
        {
            std::cout << "User registered" << std::endl;
        }
    }
    else if(_command_operator == "l")
    {
        if(_command_message == "500")
        {
            std::cout << "Login failure. Try Again!" << std::endl;        
        }
        else if(_command_message == "200")
        {
            _client->username = _client->response_from_server.at(2);
            std::cout << "User logged in" << std::endl;
        }
    }
    else if(_command_operator == "loc_friends" || _command_operator == "loc_friend")
    {
        int _number_of_online_friends = std::stoi(_client->response_from_server.at(1));
        int _friend_position = 1;
        std::string _friend_user_name = "";
        for(int _i=0; _i < _number_of_online_friends; _i++)
        {
            _friend_user_name = _client->response_from_server.at(++_friend_position);
            std::string _friend_ip = _client->response_from_server.at(++_friend_position);
            int _friend_port = std::stoi(_client->response_from_server.at(++_friend_position));
            friend_info _friend_info(_friend_user_name, _friend_ip, _friend_port);
            _client->online_friends_list[_friend_user_name] = _friend_info;
        }
        if(_command_operator == "loc_friend")
        {
            std::cout << _friend_user_name << " Logged in" << std::endl;
        }
        _client->print_online_friends();
    }
    else if(_command_operator == "rm_loc_friend")
    {
        std::string _username = _client->response_from_server.at(1);

        std::unordered_map<std::string, friend_info>::iterator _friend_info_itr = 
            _client->online_friends_list.find(_username);
        if(_friend_info_itr != _client->online_friends_list.end())
        {
            _client->online_friends_list.erase(_friend_info_itr);
        }
        std::cout << _username << " Logged out" << std::endl;
        _client->print_online_friends();
    }
    else if(_command_operator == "ir")
    {
        std::cout << "Friend Request from " << _client->response_from_server.at(1);
        if(_client->response_from_server.size() > 2)
        {
            std::cout << " >> " << _client->response_from_server.at(2);
        }
        std::cout << std::endl;
    }
    else if(_command_operator == "iar")
    {
        std::cout << "Approved Friend Request from " << _client->response_from_server.at(1);
        if(_client->response_from_server.size() > 2)
        {
            std::cout << " >> " << _client->response_from_server.at(2);
        }
        std::cout << std::endl;
    }
    else if(_command_operator == "idr")
    {
        std::cout << "Denied Friend Request from " << _client->response_from_server.at(1);
        if(_client->response_from_server.size() > 2)
        {
            std::cout << " >> " << _client->response_from_server.at(2);
        }
        std::cout << std::endl;
    }
    _client->response_received = true;
    _client->response_condition_variable.notify_all();
}

void client::handle_command_from_peer(int sockfd, std::string command)
{
    std::cout << command << std::endl;
}

void client::sigint_handler(int signal)
{
    _client->_exit();
    exit(0);
}

void * client::process_connection_p2p(void *arg) {
    int _sockfd;
    ssize_t n;
    char buf[MAXBUFLEN];

    _sockfd = *((int *)arg);
    free(arg);

    pthread_detach(pthread_self());
	while ((n = read(_sockfd, buf, MAXBUFLEN)) > 0)
	{
        buf[n] = '\0';
        handle_command_from_peer(_sockfd, std::string(buf));
    }
	if (n == 0)
	{
        std::cout << "peer closed" << std::endl;
	}
	else
	{
        std::cout << "something wrong" << std::endl;
    }
    close(_sockfd);
    return(NULL);
}

void client::print_online_friends()
{
    if(online_friends_list.empty())
    {
        std::cout << "No friend online" << std::endl;
        return;
    }

    std::cout << "Number of online friends: " << online_friends_list.size() << std::endl;
    std::unordered_map<std::string, friend_info>::iterator _online_friends_itr =
        online_friends_list.begin();
    while(_online_friends_itr != online_friends_list.end())
    {
        friend_info _friend_info = _online_friends_itr->second;
        std::cout << "Username: " << _friend_info.user_name << "; IP: " <<
            _friend_info.ip << "; Port: " << _friend_info.port << std::endl;
        _online_friends_itr++;
    }
}

void * client::process_start_p2p(void *arg)
{
    int serv_sockfd, cli_sockfd, *sock_ptr;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t sock_len;
    pthread_t tid;
    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    bzero((void*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(_client->p2p_port);

    bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(serv_sockfd, 10); // 10 is the number of backlogs in the queue

    while(true) // _client->is_peer_running
    {
        sock_len = sizeof(cli_addr);
        cli_sockfd = accept(serv_sockfd, (struct sockaddr *)&cli_addr, &sock_len);

        std::cout << "remote client IP: " << inet_ntoa(cli_addr.sin_addr);
        std::cout << ", port: " << ntohs(cli_addr.sin_port) << std::endl;

        sock_ptr = (int *)malloc(sizeof(int));
        *sock_ptr = cli_sockfd;

        pthread_create(&tid, NULL, &process_connection_p2p, (void*)sock_ptr);
    }

    close(serv_sockfd);
    return(NULL);
}

std::string client::get_fully_qualified_domain_name()
{
	std::string _fully_qualified_domain_name = "";
	struct addrinfo _hints, *_info, *_info_itr;
	int gai_result;

	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	bzero(&_hints, sizeof(_hints));
	_hints.ai_family = AF_INET; /*either IPV4 or IPV6*/
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_flags = AI_CANONNAME;

	if ((gai_result = getaddrinfo(hostname, "http", &_hints, &_info)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
		return _fully_qualified_domain_name;
	}

	for(_info_itr = _info; _info_itr != NULL; _info_itr = _info_itr->ai_next)
	{
		_fully_qualified_domain_name += _info_itr->ai_canonname;
	}

	freeaddrinfo(_info);

	return _fully_qualified_domain_name;
}
