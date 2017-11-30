#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <list>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>

#include "server.h"

//const unsigned port = 5100;
const unsigned MAXBUFLEN = 512;

int server::init(std::string user_info_file_path, std::string configuration_file_path)
{
	// handle user info file
	this->user_info_file_path = user_info_file_path;
	user_info_file_handler _user_info_file_handler(user_info_file_path);
	_user_info_file_handler.load_user_info(user_info_map);

	// handle the configuration file
	this->configuration_file_path = configuration_file_path;
	configuration_file_handler _configuration_file_handler(configuration_file_path);
	_configuration_file_handler.load_configuration(configuration_map);
	return EXIT_SUCCESS;
}

int server::exit()
{
	user_info_file_handler _user_info_file_handler(user_info_file_path);
	_user_info_file_handler.save_user_info(user_info_map);

	configuration_file_handler _configuration_file_handler(configuration_file_path);
	_configuration_file_handler.save_configuration(configuration_map);
	return EXIT_SUCCESS;
}

int server::run()
{
	int _port = get_port_from_configuration_map();
    int serv_sockfd, cli_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t sock_len;
    ssize_t n;
    char buf[MAXBUFLEN];
    fd_set readfds, masters;
    int maxfd;
    std::list<int> sockfds;

    serv_sockfd = socket(PF_INET, SOCK_STREAM, 0);

    bzero((void*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(_port);

    bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(serv_sockfd, 5);

    sock_len = sizeof(cli_addr);
	getsockname(serv_sockfd, (struct sockaddr *)&serv_addr, &sock_len);
	std::cout << "Fully Qualified Domain Name = " << get_fully_qualified_domain_name() << std::endl;
    std::cout << "Port = " << ntohs(serv_addr.sin_port) << std::endl;

    FD_ZERO(&masters);
    FD_SET(serv_sockfd, &masters);
    maxfd = serv_sockfd;
    sockfds.push_back(serv_sockfd);

    for (; ;) {
	readfds = masters;
	select(maxfd+1, &readfds, NULL, NULL, NULL);

	std::list<int> l_tmp(sockfds);
	std::list<int>::iterator itr;
	for (std::list<int>::iterator itr = l_tmp.begin(); itr !=
		 l_tmp.end(); ++itr) {
	    int sock_tmp = *itr;

	    if (FD_ISSET(sock_tmp, &readfds)) {
		if (sock_tmp == serv_sockfd) {
		    // new connection request
		    cli_sockfd = accept(serv_sockfd, (struct sockaddr
						      *)&cli_addr, &sock_len);
		    std::cout << "New connection accepted" << std::endl;
		    FD_SET(cli_sockfd, &masters);
		    if (cli_sockfd > maxfd)
			maxfd = cli_sockfd;
		    sockfds.push_back(cli_sockfd);
		} else {
		    // data message
		    n = read(sock_tmp, buf, MAXBUFLEN);
		    if (n <= 0) {
			if (n == 0) 
			    std::cout << "connection closed" << std::endl;
			else
			    perror("something wrong");
			close(sock_tmp);
			FD_CLR(sock_tmp, &masters);
			sockfds.remove(sock_tmp);
		    } else {
			buf[n] = '\0';
			std::cout << buf << std::endl;
			write(sock_tmp, buf, strlen(buf));
		    }
		}
	    }
	}
    }
    close(serv_sockfd);
}

std::string server::get_fully_qualified_domain_name()
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

int server::get_port_from_configuration_map()
{
	int _port = 0;
	std::unordered_map<std::string, std::string>::iterator _config_map_itr = 
		configuration_map.find(configuration_keys::port);

	if(_config_map_itr == configuration_map.end())
	{
		return _port;
	}
	else
	{
		return std::stoi(_config_map_itr->second);
	}
}
