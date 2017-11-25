#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>

#include "file_handler.h"

void utility::trim_string(std::string& splitted_command_string)
{
    size_t _left_trim_first = splitted_command_string.find_first_not_of(' ');
    if(_left_trim_first == std::string::npos)
    {
        splitted_command_string =  "";
        return;
    }

    size_t _right_trim_last = splitted_command_string.find_last_not_of(' ');
    splitted_command_string = splitted_command_string.substr(_left_trim_first, (_right_trim_last -
        _left_trim_first)+1);
}

std::vector<std::string> utility::split_string(const std::string& command_string, char delimiter)
{
    std::vector<std::string> _splitted_string_vector;

    std::stringstream _command_string_stream(command_string);
    std::string _splitted_string;

    while(std::getline(_command_string_stream, _splitted_string, delimiter))
    {
        trim_string(_splitted_string);
        if(!_splitted_string.empty())
        {
            _splitted_string_vector.push_back(_splitted_string);
        }
    }

    return _splitted_string_vector;
}

int user_info_file_handler::load_user_info(std::unordered_map<std::string, user_info>& user_info_map)
{
    std::ifstream _user_info_file_stream(file_path);
    if(_user_info_file_stream.fail())
    {
        std::cout << "The user info file does not exist" << std::endl;
        return EXIT_FAILURE;
    }

    std::string _line = "";
    while(std::getline(_user_info_file_stream, _line))
    {
        std::string _user_name = "";
        std::string _password = "";
        std::vector<std::string> _contact_user_info_list;
        std::vector<std::string> _splitted_line = utility::split_string(_line, '|');
        if(_splitted_line.size() > 0)
        {
            _user_name = _splitted_line.at(0);
            if(_splitted_line.size() > 1)
            {
                _password = _splitted_line.at(1);
            }
            if(_splitted_line.size() > 2)
            {
                std::string _contacts_string = _splitted_line.at(2);
                _contact_user_info_list = utility::split_string(_contacts_string, ';');
            }
        }
        if(!_user_name.empty() && !_password.empty())
        {
            user_info _user_info(_user_name, _password, _contact_user_info_list);
            user_info_map[_user_name] = _user_info;
        }
    }

    return EXIT_SUCCESS;
}

int user_info_file_handler::save_user_info(const std::unordered_map<std::string, user_info>& user_info_map)
{
    return EXIT_SUCCESS;
}
