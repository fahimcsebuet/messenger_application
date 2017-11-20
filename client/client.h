class client
{
public:
    int run(int argc, char **argv);
private:
    static void * process_connection(void *arg);
};
