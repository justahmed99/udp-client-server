#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
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
    char request[100], buffer[BUFF_SIZE], response[BUFF_SIZE];
    int byte_recv, fd;
    struct sockaddr_in address;

    int found_status = 0;
    struct frame_t frame;
    struct timeval t_out = {0, 0};
    struct stat st;
    off_t file_size;

    memset(request, 0, sizeof(request));
    memset(&address, 0, sizeof(address));

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

    while (1)
    {
        printf("Waiting ...\n");
        byte_recv = recvfrom(sockfd, request, sizeof(request), 0, (struct sockaddr *)&address, &address_len);
        FILE *file = fopen(request, "rb");
        if (file == NULL)
        {
            printf("[-] File not found!\n");

            found_status = 0;
            strcpy(response, "File not found!");
            sendto(sockfd, &(response), sizeof(response), 0, (struct sockaddr *)&address, sizeof(address));
            sendto(sockfd, &(found_status), sizeof(int), 0, (struct sockaddr *)&address, sizeof(address));
        }
        else
        {
            printf("[+] File found! Name : %s\n", request);

            found_status = 1;
            strcpy(response, "[+] File found!");
            sendto(sockfd, &(response), sizeof(response), 0, (struct sockaddr *)&address, sizeof(address));
            sendto(sockfd, &(found_status), sizeof(int), 0, (struct sockaddr *)&address, sizeof(address));

            stat(request, &st);
            file_size = st.st_size;

            t_out.tv_sec = 2;
            t_out.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));

            // fseek(file, 0L, SEEK_END);
            // long int file_size = ftell(file);
            long int total_frame = 0;
            long int ack_num = 0;
            long int drop_frame = 0, resend_frame = 0, t_out_flag = 0;

            if ((file_size % BUFF_SIZE) != 0)
                total_frame = (file_size / BUFF_SIZE) + 1;
            else
                total_frame = (file_size / BUFF_SIZE);
            printf("[+] Total frame : %ld\n", total_frame);

            sendto(sockfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *)&address, sizeof(address));
            recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&address, &address_len);
            printf("[+] ACK Number : %ld\n", ack_num);

            // Check for the acknowledgement
            while (ack_num != total_frame)
            {
                sendto(sockfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *)&address, sizeof(address));
                recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&address, &address_len);

                resend_frame++;

                if (resend_frame == 10)
                {
                    t_out_flag = 1;
                    break;
                }
            }

            for (long int i = 1; i <= total_frame; i++)
            {
                memset(&frame, 0, sizeof(frame));
                ack_num = 0;
                frame.id = i;
                frame.length = fread(frame.data, 1, BUFF_SIZE, file);

                sendto(sockfd, &(frame), sizeof(frame), 0, (struct sockaddr *)&address, sizeof(address));
                recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&address, (socklen_t *)&address_len);

                while (ack_num != frame.id)
                {
                    sendto(sockfd, &(frame), sizeof(frame), 0, (struct sockaddr *)&address, sizeof(address));
                    recvfrom(sockfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *)&address, (socklen_t *)&address_len);
                    printf("[-] Frame ===> %ld	dropped, %ld times\n", frame.id, ++drop_frame);

                    resend_frame++;

                    printf("[-] Frame ===> %ld	dropped, %ld times\n", frame.id, drop_frame);

                    if (resend_frame == 200)
                    {
                        t_out_flag = 1;
                        break;
                    }
                }

                resend_frame = 0;
                drop_frame = 0;

                if (t_out_flag == 1)
                {
                    printf("File not sent\n");
                    break;
                }

                printf("[+] Frame ===> %ld;	Acknowledge Number ===> %ld; Length ===> %ld; \n", frame.id, ack_num, frame.length);

                if (total_frame == ack_num)
                    printf("[+] File sent successfully\n");
            }

            fclose(file);

            t_out.tv_sec = 0;
            t_out.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));
        }
    }

    return 0;
}