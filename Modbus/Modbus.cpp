#include "Modbus.hpp"

Modbus:: Modbus(const string host, const int port)
    : host(host), port(port), sock_fd(-1)
{
    // Create socket
    if ( ( sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror( "Socket create fail\n");
        exit(1);
    }
    else
        printf( " Socket create success\n");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock_fd);
        exit(1);
    }

    printf("[");
    for ( int i = 0; i< host.size(); i++){
        printf("%c", host[i]);
    }
    printf(":%d]", port);

    // Connect to the server
    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror(" Connect failed");
        close(sock_fd);
        exit(1);
    }
    else
        printf( " Connect success\n");
    
}

Modbus:: ~Modbus(){
    close(sock_fd);
}

void Modbus:: modbus_display_msg(char* buffer, size_t size){
    for ( int i = 0; i< size; i++){
        for ( int i = 0; i< size; i++){
            printf("buffer[%2d] = 0x%02x => ", i, static_cast<unsigned char>(buffer[i]));
            for ( int j = 7; j>= 4; j--) printf("%d", (buffer[i] >> j) & 1);
            printf(" ");
            for ( int j = 3; j>= 0; j--) printf("%d", (buffer[i] >> j) & 1);
            printf("\n");
        }
        return;
    }
}

/************************************************

-------------------------------------------------------------------------------------
|TCP Header	| Address | Function Code | Start register addr	| query length | data   |
-------------------------------------------------------------------------------------
|6bytes	    |1byte    |1byte          |2byte                |2bytes	       |N bytes |
-------------------------------------------------------------------------------------

************************************************/
char* Modbus:: modbus_request( int transaction, int startAddr, int len, int functionCode, uint16_t* data){
    char* buffer = new char[25];
    this->initBuffer(transaction, functionCode, buffer);
    size_t size;

    if ( functionCode <= 4){
        size = modbus_read(startAddr, len, buffer);
    }
    else if ( functionCode > 4 && functionCode <= 6){
        size = modbus_write_single(startAddr, data, buffer);
    }
    else if ( functionCode > 6){
        size = modbus_write_multiple(startAddr, len, data, buffer);
    }

    // send req to server
    printf("\nsend req size = %d\n", size);
    modbus_display_msg(buffer, size);

    send(sock_fd, buffer, size, 0);

    // recv res from server (MBAP header)
    size_t recv_size;

    // if server disconnect, recv package's size = 0
    if ( (recv_size = recv(sock_fd, buffer, sizeof(buffer), 0)) == 0){
        printf("Recv failed\n");
        this->~Modbus();
        return 0;
    }
    
    printf("\nrecv res size = %ld\n", recv_size);
    modbus_display_msg(buffer, size);

    // recv res from server (Data)
    if ( (recv_size = recv(sock_fd, buffer, sizeof(buffer), 0)) != 0){
        printf("\nrecv res size = %ld\n", recv_size);
        modbus_display_msg(buffer, recv_size);
    }

    return buffer;
}

/************************************************
*
* byte 0~1 為本次通訊的識別碼
* byte 2~4 通常為0
* byte 5   為資料長度 (從Address ~ data的總長度)
* byte 6   default 0xFF in Modbus tcp/ip
* byte 7   function code
*
************************************************/
void Modbus:: initBuffer(int transaction, int functionCode, char* buffer){
    buffer[0] = transaction >> 8;
    buffer[1] = transaction & 0xFF;

    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    
    buffer[5] = 0x06;

    buffer[6] = 0xFF;

    buffer[7] = functionCode;
}

/************************************************
*
* 01: Read coils
* 02: Read input status
* 03: Read holding register
* 04: Read input register
* 05: Write single coils
* 06: Write single holding register
* 15: Write multiple coils
* 16: Write multiple holding register
* 
************************************************/
int Modbus:: modbus_read(int startAddr, int len, char* buffer){
    
    buffer[8] = startAddr >> 8;
    buffer[9] = startAddr & 0xFF;

    buffer[10] = len >> 8;
    buffer[11] = len & 0xFF;

    return 12;
}

int Modbus:: modbus_write_single(int startAddr, uint16_t* data, char* buffer){

    buffer[8] = startAddr >> 8;
    buffer[9] = startAddr & 0xFF;

    buffer[10] = data[0] >> 8;
    buffer[11] = data[0] & 0xFF;
    
    return 12;
}

int Modbus:: modbus_write_multiple(int startAddr, int len, uint16_t* data, char* buffer){

    buffer[8] = startAddr >> 8;
    buffer[9] = startAddr & 0xFF;

    buffer[10] = len >> 8;
    buffer[11] = len & 0xFF;

    buffer[12] = len * 2;
    for ( int i = 0; i< len; i++){
        buffer[13+i*2] = data[i] >> 8;
        buffer[14+i*2] = data[i] & 0xFF;
    }


    return 12 + len*2;
}