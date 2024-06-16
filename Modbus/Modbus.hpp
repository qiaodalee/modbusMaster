#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>

using namespace std;

class Modbus {
    public:
        Modbus(const string host, const int port);
        ~Modbus();
        char* modbus_request( int transaction, int startAddr, int len, int functionCode, uint16_t* data);
        
    private:
        void initBuffer(int transaction, int functionCode, char* buffer);
        int modbus_read(int startAddr, int len, char* buffer);
        int modbus_write_single(int startAddr, uint16_t* data, char* buffer);
        int modbus_write_multiple(int startAddr, int len, uint16_t* data, char* buffer);
        void modbus_display_msg(char* buffer, size_t size);
        
        string host;
        int port;
        int sock_fd;
        struct sockaddr_in addr;
};