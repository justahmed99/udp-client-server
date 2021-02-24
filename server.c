#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 8081

int main (int argc, char *argv[])
{
    char request[100], buffer[1000];
    int byte_recv, fd;

    struct sockaddr_in address;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);
    address.sin_family = AF_INET;

    int address_len = sizeof(address);

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        printf("[-] Socket creation error!\n");
        return 1;
    }
    else 
        printf("[+] Socket creation success!\n");
    
    int server_bind = bind(sockfd, (struct sockaddr *)&address, sizeof(address));
    if (server_bind)
    {
        printf("[-] Server binding failed!\n");
        return 1;
    }
    else
        printf("[+] Server binding success!\n");
    
    while(1)
    {
        printf("Waiting ...\n");
        byte_recv = recvfrom(sockfd, request, sizeof(request), 0, (struct sockaddr *)&address, &address_len);
        FILE *file = fopen(request, "r");
        if (file == NULL)
        {
            printf("[-] File not found!\n");
            sendto(sockfd, "File not found!", sizeof("File not found!"), 0, (struct sockaddr *)&address, sizeof(address));
        }
        else
        {
            sendto(sockfd, "File found!", sizeof("File found!"), 0, (struct sockaddr *)&address, sizeof(address));
            fclose(file);
            fd = open(request, O_RDONLY);
            while ((byte_recv = read(fd, buffer, 1000)) > 0)
            {
                sendto(sockfd, buffer, byte_recv, 0, (struct sockaddr*)&address, sizeof(address));
            }
            printf("[+] File found!\n");
        }
    }

    return 0;
}