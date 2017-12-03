#include <iostream>
#include <unistd.h>
#include <sstream>

#include "client.h"

int handle_credential_commands_to_server(client* in_client)
{
    char _sentinel = -1;
    std::string _command = "";
    while (true)
	{
        std::vector<std::string> _response_from_server = in_client->get_response_from_server();
        if(!_response_from_server.empty())
        {
            if(_response_from_server.at(0) == "r" && _response_from_server.size() == 4)
            {
                if(_response_from_server.at(1) == "200")
                {
                    std::cout << "Automatic Login..." << std::endl;
                    std::string _username = _response_from_server.at(2);
                    std::string _password = _response_from_server.at(3);
                    _command = "l";
                    std::string _data_for_server = _command + _sentinel + _username + _sentinel + _password;
                    in_client->send_data_to_server(_data_for_server);
                    continue;
                }
                else
                {
                    std::cout << "Type command" << std::endl;
                    getline(std::cin, _command);
                }
            }
            else if(_response_from_server.at(0) == "l" && _response_from_server.size() == 4)
            {
                if(_response_from_server.at(1) == "200")
                {
                    std::string _username = _response_from_server.at(2);
                    in_client->send_location_info_to_server(_username);
                    continue;
                }
                else
                {
                    std::cout << "Type command" << std::endl;
                    getline(std::cin, _command);
                }
            }
            else if(_response_from_server.at(0) == "loc_friends" || _response_from_server.at(0) == "loc_friend")
            {
                break;
            }
            else
            {
                std::cout << "Type command" << std::endl;
                getline(std::cin, _command);
            }
        }
        else
        {
            std::cout << "Type command" << std::endl;
            getline(std::cin, _command);
        }

		if (_command == "exit")
		{
            in_client->_exit();
			exit(0);
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

int pre_process_command(std::string command, std::vector<std::string>& out_command_vector)
{
    out_command_vector = {};

    std::stringstream _command_string_stream(command);
    std::string _splitted_string;

    char _delimiter = ' ';
    while(std::getline(_command_string_stream, _splitted_string, _delimiter))
    {
        utility::trim_string(_splitted_string);
        if(!_splitted_string.empty())
        {
            out_command_vector.push_back(_splitted_string);
        }
        if(out_command_vector.size() == 2)
        {
            _delimiter = '\n';
        }
    }

    return EXIT_SUCCESS;
}

int handle_p2p_commands(client* in_client)
{
    char _sentinel = -1;
    std::string _command = "";
    while (true)
	{
        std::vector<std::string> _response_from_server = in_client->get_response_from_server();
        if(!_response_from_server.empty())
        {
            if(_response_from_server.at(0) == "i")
            {
                if(_response_from_server.at(1) == "200")
                {
                    std::cout << "Request Sent" << std::endl;
                }
                else if(_response_from_server.at(1) == "500")
                {
                    if(_response_from_server.size() > 2)
                        std::cout << _response_from_server.at(2) << std::endl;
                }
                std::cout << "Type command" << std::endl;
                getline(std::cin, _command);
            }
            else if(_response_from_server.at(0) == "ia" || _response_from_server.at(0) == "id")
            {
                std::string _message_type = (_response_from_server.at(0) == "ia")? "Apprval" : "Denial";
                if(_response_from_server.at(1) == "200")
                {
                    std::cout << _message_type << " Sent" << std::endl;
                }
                else if(_response_from_server.at(1) == "500")
                {
                    if(_response_from_server.size() > 2)
                        std::cout << _response_from_server.at(2) << std::endl;
                }
                std::cout << "Type command" << std::endl;
                getline(std::cin, _command);
            }
            else
            {
                std::cout << "Type command" << std::endl;
                getline(std::cin, _command);
            }
        }
        else
        {
            std::cout << "Type command" << std::endl;
            getline(std::cin, _command);
        }

		if (_command == "exit")
		{
			in_client->_exit();
			exit(0);
        }
        std::vector<std::string> _parsed_command;
        pre_process_command(_command, _parsed_command);
        if(!_parsed_command.empty())
        {
            std::string _command_operator = _parsed_command.at(0);
            if(_parsed_command.at(1) == in_client->get_username())
            {
                std::cout << "Can not be friend of self" << std::endl;
                std::cout << "Type command" << std::endl;
                getline(std::cin, _command);
            }
            std::string _data_for_server = _command_operator + _sentinel + in_client->get_username();
            if(_command_operator == "i" || _command_operator == "ia" || _command_operator == "id")
            {
                for(unsigned int i=1; i<_parsed_command.size(); i++)
                {
                    _data_for_server += (_sentinel + _parsed_command.at(i));
                }
            }

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
    handle_credential_commands_to_server(&_client);
    _client.start_p2p();
    handle_p2p_commands(&_client);
    _client._exit();
    return EXIT_SUCCESS;
}
