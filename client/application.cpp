#include <iostream>
#include <unistd.h>
#include <sstream>

#include "client.h"

int handle_credential_commands_to_server(client* in_client)
{
    std::cout << "Register or Login" << std::endl;
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
                in_client->start_p2p();
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
            utility::trim_string(_command);
            if(!_command.empty())
                std::cout << "Command not available or possible" << std::endl;
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
    while (true)
	{
        std::string _command = "";
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
            else if(_response_from_server.at(0) == "m")
            {
                if(_response_from_server.at(1) == "500")
                {
                    if(_response_from_server.size() > 2)
                        std::cout << _response_from_server.at(2) << std::endl;
                }
                std::cout << "Type command" << std::endl;
                getline(std::cin, _command);
            }
            else if(_response_from_server.at(0) == "ia" || _response_from_server.at(0) == "id")
            {
                std::string _message_type = (_response_from_server.at(0) == "ia")? "Approval" : "Denial";
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
            else if(_response_from_server.at(0) == "logout")
            {
                in_client->stop_p2p();
                std::cout << "User logged out" << std::endl;
                handle_credential_commands_to_server(in_client);
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
        if (_command == "logout")
		{
            utility::trim_string(_command);
            std::string _data_for_server = _command + _sentinel + in_client->get_username();
            in_client->send_data_to_server(_data_for_server);
			continue;
        }
        std::vector<std::string> _parsed_command;
        pre_process_command(_command, _parsed_command);
        if(!_parsed_command.empty())
        {
            std::string _command_operator = _parsed_command.at(0);
            if(_command_operator == "i" || _command_operator == "ia" || _command_operator == "id")
            {
                std::string _data_for_server = _command_operator + _sentinel + in_client->get_username();
                if(_parsed_command.at(1) == in_client->get_username())
                {
                    std::cout << "Can not be friend of self" << std::endl;
                    std::cout << "Type command" << std::endl;
                    getline(std::cin, _command);
                }
                for(unsigned int i=1; i<_parsed_command.size(); i++)
                {
                    _data_for_server += (_sentinel + _parsed_command.at(i));
                }

                in_client->send_data_to_server(_data_for_server);
            }
            else if(_command_operator == "m")
            {
                std::string _data_for_server = _command_operator + _sentinel + in_client->get_username();
                if(_parsed_command.at(1) == in_client->get_username())
                {
                    std::cout << "Can not send message to self" << std::endl;
                    std::cout << "Type command" << std::endl;
                    getline(std::cin, _command);
                }
                for(unsigned int i=1; i<_parsed_command.size(); i++)
                {
                    _data_for_server += (_sentinel + _parsed_command.at(i));
                }

                in_client->send_data_to_server(_data_for_server);
            }
            else if(_command_operator == "message")
            {
                std::string _data_for_peer = _command_operator + _sentinel + in_client->get_username();
                if(_parsed_command.at(1) == in_client->get_username())
                {
                    std::cout << "Can not send message to self" << std::endl;
                    std::cout << "Type command" << std::endl;
                    getline(std::cin, _command);
                }
                for(unsigned int i=1; i<_parsed_command.size(); i++)
                {
                    _data_for_peer += (_sentinel + _parsed_command.at(i));
                }
                std::string _peer_username = _parsed_command.at(1);
                int _res = in_client->connect_to_peer(_peer_username);
                std::unordered_map<std::string, friend_info>::iterator _friend_info_itr =
                    in_client->get_online_friends_list().find(_peer_username);
                if(_res == EXIT_SUCCESS && _friend_info_itr->second.connected && _friend_info_itr->second.sockfd != -1)
                    in_client->send_data_to_peer(_friend_info_itr->second.sockfd, _data_for_peer);
                else
                {
                    std::cout << "Connection Problem" << std::endl;
                    std::cout << "Type command" << std::endl;
                    getline(std::cin, _command);
                }
            }
        }
		else
		{
			utility::trim_string(_command);
            if(!_command.empty())
                std::cout << "Command not available or possible" << std::endl;
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
    handle_p2p_commands(&_client);
    _client._exit();
    return EXIT_SUCCESS;
}
