#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <string>
#include <unordered_map>

class client
{
public:
    int init(std::string configuration_file_path);
    int run();
    int _exit();
private:
    std::string configuration_file_path;
    std::unordered_map<std::string, std::string> configuration_map;
    static void * process_connection(void *arg);
};

#endif
