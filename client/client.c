#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 8081
#define BUFF_SIZE 1024

struct frame_t
{
    long int id;
    long int length;
    char data[BUFF_SIZE];
};

int main(int argc, char *argv[])
{

    int found_status = 0;
    long int total_frame = 0;

    struct timeval t_out = {0, 0};
    struct frame_t frame;

    if (argc < 2)
    {
        printf("[!] input command : ./client [ip address] [filename]\n");
        return 1;
    }
    char request[100], buffer[BUFF_SIZE], response[BUFF_SIZE];
    int fd;

    memset(buffer, '0', sizeof(buffer));

    struct sockaddr_in address;

    memset(&address, 0, sizeof(address));

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

    sendto(sockfd, request, sizeof(request), 0, (struct sockaddr *)&address, address_len);

    recvfrom(sockfd, &(response), sizeof(response), 0, (struct sockaddr *)&address, &address_len);
    printf("response : %s\n", response);

    recvfrom(sockfd, &(found_status), sizeof(int), 0, (struct sockaddr *)&address, &address_len);

    if (found_status != 0)
    {
        t_out.tv_sec = 2;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));

        recvfrom(sockfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *)&address, (socklen_t *)&address_len);

        t_out.tv_sec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));

        if (total_frame > 0)
        {
            sendto(sockfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *)&address, address_len);
            printf("[+] Total Frame : %ld\n", total_frame);

            long int bytes_rec = 0;
            FILE *file_target = fopen(request, "wb");

            for (long int i = 1; i <= total_frame; i++)
            {
                memset(&(frame), 0, sizeof(frame));

                recvfrom(sockfd, &(frame), sizeof(frame), 0, (struct sockaddr *)&address, (socklen_t *)&address_len);
                sendto(sockfd, &(frame.id), sizeof(frame.id), 0, (struct sockaddr *)&address, address_len);

                if ((frame.id < i) || (frame.id > i))
                {
                    i--;
                }
                else
                {
                    fwrite(frame.data, 1, frame.length, file_target);
                    printf("[+] ID Frame ===> %ld,	Frame Length ===> %ld\n", frame.id, frame.length);
                    bytes_rec += frame.length;
                }

                if (i == total_frame)
                {
                    printf("[+] File recieved\n");
                }
            }
            printf("[+] Total data : %ld bytes\n", bytes_rec);
            fclose(file_target);
        }
        else
        {
            printf("[-] File is empty.\n");
        }
    }

    // printf("total frame : %ld\n", total_frame);
    // buffer[n_bytes] = 0;

    // fd = open(request, O_RDWR | O_CREAT, 0666);

    // while ((n_bytes = recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr*)&address, &address_len)) > 0)
    // {
    //     buffer[n_bytes] = 0;
    //     write(fd, buffer, n_bytes);
    //     printf("%s\n", request);
    // }

    close(sockfd);

    return 0;
}