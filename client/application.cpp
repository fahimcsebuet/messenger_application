#include <iostream>

#include "client.h"

void sigint_handler(int signal)
{

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
    _client.run();
    _client._exit();
    return EXIT_SUCCESS;
}
