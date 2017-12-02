#include <iostream>
#include <unistd.h>

#include "client.h"

int handle_commands_to_server(client* in_client)
{
    char _sentinel = -1;
    std::string _command;
    while (in_client->get_command_error())
	{
        std::cout << "Type command" << std::endl;
        getline(std::cin, _command);
		if (_command == "exit")
		{
			in_client->_exit();
			break;
        }
        else if (_command == "r")
        {
            std::cout << "Register" << std::endl;
            std::string _username = "";
            std::string _password = "";
            std::cout << "Please enter username: ";
            getline(std::cin, _username);
            utility::trim_string(_username);
            while(_username.empty())
            {
                std::cout << "User name cannot be empty" << std::endl;
                std::cout << "Please enter username: ";
                getline(std::cin, _username);
            }
            std::cout << "Please enter password: ";
            getline(std::cin, _password);
            utility::trim_string(_password);
            while(_password.empty())
            {
                std::cout << "Password cannot be empty" << std::endl;
                std::cout << "Please enter password: ";
                getline(std::cin, _password);
            }
            std::string _data_for_server = _command + _sentinel + _username + _sentinel + _password;
            in_client->send_data_to_server(_data_for_server);
        }
        else if (_command == "l")
        {
            std::cout << "Login" << std::endl;
            std::string _username = "";
            std::string _password = "";
            std::cout << "Please enter username: ";
            getline(std::cin, _username);
            utility::trim_string(_username);
            while(_username.empty())
            {
                std::cout << "User name cannot be empty" << std::endl;
                std::cout << "Please enter username: ";
                getline(std::cin, _username);
            }
            std::cout << "Please enter password: ";
            getline(std::cin, _password);
            utility::trim_string(_password);
            while(_password.empty())
            {
                std::cout << "Password cannot be empty" << std::endl;
                std::cout << "Please enter password: ";
                getline(std::cin, _password);
            }
            std::string _data_for_server = _command + _sentinel + _username + _sentinel + _password;
            in_client->send_data_to_server(_data_for_server);
        }

		else
		{
			in_client->send_data_to_server(_command);
		}
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cout << "Invalid number of parameters!" << std::endl;
        return EXIT_FAILURE;
    }
    std::string _configuration_file(argv[1]);
    client _client;
    _client.init(_configuration_file);
    _client.start();
    handle_commands_to_server(&_client);
    _client._exit();
    return EXIT_SUCCESS;
}
