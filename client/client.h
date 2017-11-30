#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

class client
{
public:
    int run(int argc, char **argv);
private:
    static void * process_connection(void *arg);
};

#endif
