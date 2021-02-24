#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define ADDR "127.0.0.1"
#define PORT 8081

int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("[!] input command : ./client [ip address] [filename]\n");
        return 1;
    }
    char request[100], buffer[20000];
    int fd;

    memset(buffer, '0', sizeof(buffer));

    struct sockaddr_in address;
    address.sin_addr.s_addr = inet_addr(argv[1]);
    address.sin_port = htons(PORT);
    address.sin_family = AF_INET;

    int address_len = sizeof(address);

    if (argc < 2)
    {
        printf("[-] Please input the argument!\n");
        return 1;
    }
    else
        strcpy(request, argv[2]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        printf("[-] Socket creation error!\n");
        return 1;
    }
    else 
        printf("[+] Socket creation success!\n");
    
    // int server_bind = bind(sockfd, (struct sockaddr *)&address, sizeof(address));
    // if (server_bind)
    // {
    //     printf("[-] Server binding failed!\n");
    //     return 1;
    // }
    // else
    //     printf("[+] Server binding success!\n");

    sendto(sockfd, request , sizeof(request), 0, (struct sockaddr*)&address, address_len);

    int n_bytes = recvfrom(sockfd, buffer, 2048, 0, (struct sockaddr*)&address, &address_len);
    buffer[n_bytes] = 0;

    fd = open(request, O_RDWR | O_CREAT, 0666);

    // FILE *file = fopen(request, "ab");

    while ((n_bytes = recvfrom(sockfd, buffer, 20000, 0, (struct sockaddr*)&address, &address_len)) > 0)
    {
        buffer[n_bytes] = 0;
        // fwrite(n_bytes, 1, n_bytes, file);
        write(fd, buffer, n_bytes);
        printf("%s\n", request);
    }

    close(sockfd);
    

    return 0;
}