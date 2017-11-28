#include <string>
#include <unordered_map>
#include <vector>

#include "data_structures.h"

class utility
{
public:
    static std::vector<std::string> split_string(const std::string& command_string, char delimiter);
    static void trim_string(std::string& splitted_command_string);
};

class user_info_file_handler
{
public:
    user_info_file_handler(std::string file_path)
    {
        this->file_path = file_path;
    }

    int load_user_info(std::unordered_map<std::string, user_info>& user_info_map);
    int save_user_info(std::unordered_map<std::string, user_info>& user_info_map);

private:
    std::string file_path;
};
