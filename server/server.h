#include <string.h>

class server
{
public:
    int init(std::string user_info_file, std::string configuration_file);
    int run();
};
