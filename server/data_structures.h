#include <string>
#include <vector>
#include <unordered_map>

class user_info
{
public:
    user_info(std::string user_name, std::string password, 
        std::vector<std::string>& contact_user_name_list)
    {
        this->user_name = user_name;
        this->password = password;
        this->contact_user_name_list = contact_user_name_list;
    }

private:
    std::string user_name;
    std::string password;
    std::vector<std::string> contact_user_name_list;
};

class configuration_info
{
public:
    configuration_info(int port)
    {
        this->port = port;
    }
private:
    int port;
};
