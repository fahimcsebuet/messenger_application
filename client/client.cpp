#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
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
#include "file_handler.h"

const unsigned MAXBUFLEN = 512;
int client::sockfd = -1;
client client::_client;

int client::init(std::string configuration_file_path)
{
	signal(SIGINT, sigint_handler);
	this->configuration_file_path = configuration_file_path;
	configuration_file_handler _configuration_file_handler(configuration_file_path);
	_configuration_file_handler.load_configuration(configuration_map);
	_client = *this;
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

    std::cout << _servhost << " " << _servport << std::endl;

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

    std::string oneline;
    return EXIT_SUCCESS;
}

int client::send_data_to_server(std::string data)
{
	if(sockfd != -1)
	{
		write(sockfd, data.c_str(), data.length());
	}
	return EXIT_SUCCESS;
}

int client::_exit()
{
	if(sockfd != -1) close(sockfd);
	configuration_file_handler _configuration_file_handler(configuration_file_path);
	_configuration_file_handler.save_configuration(configuration_map);
	return EXIT_SUCCESS;
}

void * client::process_connection(void *arg)
{
    int n;
    char buf[MAXBUFLEN];
    pthread_detach(pthread_self());
	while (1)
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
			// we directly exit the whole process.
			exit(1);
		}
		buf[n] = '\0';
		std::cout << buf << std::endl;
    }
}

void client::sigint_handler(int signal)
{
	std::cout << "SIGINT handler" << std::endl;
	_client._exit();
	exit(0);
}
