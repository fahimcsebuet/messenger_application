#include <string.h>

class server
{
public:
    int init(std::string user_info_file_path, std::string configuration_file_path);
    int exit();
    int run();

private:
    std::string user_info_file_path;
};
