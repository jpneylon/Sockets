#ifndef __SOCKET_CLASS_H__
#define __SOCKET_CLASS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <time.h>

#define MESSAGE_SIZE            32
#define MAX_DATA_PACKET_SIZE    16384
#define ALIEN_IP                "10.140.186.112"
#define HULK_IP                 "10.47.187.66"
#define AMAX_2_IP               "10.47.187.68"
#define PORTNO_BASE             51000


class CS_Socket
{
    public:
        CS_Socket()
        {
            message_size            = MESSAGE_SIZE;
            max_data_packet_size    = MAX_DATA_PACKET_SIZE;
            portno                  = PORTNO_BASE;
            compressed_allocated    = false;
            compressed_size         = 0;

            socket_flags[0] = "HULK";
            socket_flags[1] = "ALIEN";
            socket_flags[2] = "AMAX2";
        }
        ~CS_Socket()
        {
            free_compressed_arrays();
        }

        enum SocketServerEnum {
            HULK_SOCKET,
            ALIEN_SOCKET,
            AMAX_2_SOCKET };
        SocketServerEnum socket_server;
        char *socket_flags[3];

        void set_port_number( unsigned int n )
        {
            portno = PORTNO_BASE + n;
        }
        unsigned int get_port_number()
        {
            return portno;
        }
        void set_maximum_data_packet_size( unsigned int n )
        {
            max_data_packet_size = n;
        }
        unsigned int get_maximum_data_packet_size()
        {
            return max_data_packet_size;
        }
        void set_message_size( unsigned int n )
        {
            message_size = n;
        }
        unsigned int get_message_size()
        {
            return message_size;
        }
        void set_compressed_size( unsigned int n )
        {
            compressed_size = n;
        }
        unsigned int get_compressed_size()
        {
            return compressed_size;
        }
        void error(const char *msg)
        {
            perror(msg);
            exit(EXIT_FAILURE);
        }


        void open_client_socket()
        {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
                error("ERROR opening socket");

            switch (socket_server)
            {
                case HULK_SOCKET:
                    server = gethostbyname(HULK_IP);
                    break;
                case ALIEN_SOCKET:
                    server = gethostbyname(ALIEN_IP);
                    break;
                case AMAX_2_SOCKET:
                    server = gethostbyname(AMAX_2_IP);
                    break;
            }

            if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(EXIT_FAILURE);
            }

            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr,
                 (char *)&serv_addr.sin_addr.s_addr,
                 server->h_length);
            serv_addr.sin_port = htons(portno);

            if (connect( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR connecting");
        }
        void open_server_socket()
        {
             sockfd = socket(AF_INET, SOCK_STREAM, 0);
             if (sockfd < 0)
                error("ERROR opening socket");
             bzero((char *) &serv_addr, sizeof(serv_addr));

             serv_addr.sin_family = AF_INET;
             serv_addr.sin_addr.s_addr = INADDR_ANY;
             serv_addr.sin_port = htons(portno);
             if (bind(sockfd, (struct sockaddr *) &serv_addr,
                      sizeof(serv_addr)) < 0)
                      error("ERROR on binding");

             if (listen(sockfd,5) < 0)
                error("ERROR upon listen");
        }


        void send_message_client_socket( char *message )
        {
            n = write( sockfd, message, message_size);
            if (n < 0)
                 error("ERROR writing to client socket");
            printf("\n Send To Server: %s", message);
        }
        void send_message_server_socket( char *message )
        {
            n = write( newsockfd, message, message_size);
            if (n < 0)
                 error("ERROR writing to server socket");
            printf("\n Send To Client: %s", message);
        }
        void receive_message_client_socket( char *message )
        {
             n = read( sockfd, message, message_size );
             if (n < 0)
                error("ERROR reading from socket");
             printf("\n Received from Server: %s", message);
        }
        void receive_message_server_socket( char *message )
        {
             n = read( newsockfd, message, message_size );
             if (n < 0)
                error("ERROR reading from socket");
             printf("\n Received from Client: %s", message);
        }


        void listen_server_socket()
        {
             printf("\n Waiting for client...\n");

             struct sockaddr_in cli_addr;
             socklen_t clilen = sizeof(cli_addr);

             newsockfd = accept(sockfd,
                         (struct sockaddr *) &cli_addr,
                         &clilen);
             if (newsockfd < 0)
             {
                error("ERROR upon accept");
             }

             time_t rawtime;
             struct tm * timeinfo;
             char buffer[64];
             time (&rawtime);
             timeinfo = localtime (&rawtime);
             strftime (buffer,32,"%m.%d.%y-%H.%M.%S",timeinfo);
             printf("\n %s :\n",buffer);
        }

        void allocate_compressed_arrays()
        {
            if (compressed_size > 0)
            {
                if (compressed_allocated)
                    delete [] compressed_index;
                compressed_index = new unsigned int[compressed_size];
                compressed_allocated = true;
            }
        }
        void free_compressed_arrays()
        {
            if (compressed_allocated)
            {
                compressed_allocated = false;
                compressed_size = 0;
                delete [] compressed_index;
            }
        }


        void close_socket()
        {
             close(sockfd);
        }
        void close_new_socket()
        {
             close(newsockfd);
        }


        template <typename T> unsigned int calculate_compressed_data_size( T *data, unsigned int size_data, T data_threshold )
        {
            compressed_size = 0;
            for (unsigned int i=0; i<size_data; i++)
                if (data[i] > data_threshold)
                    compressed_size++;

            printf("\n Data Can Be Compressed From %lu bytes to %lu bytes", size_data * sizeof(T), compressed_size * sizeof(T) );
            unsigned int uncompressed_bytes = size_data * sizeof(T);
            unsigned int total_compressed_bytes = compressed_size * ( sizeof(T) + sizeof(unsigned int) );
            printf("\n Total Data Transferred Changes From %u bytes to %u bytes\n", uncompressed_bytes, total_compressed_bytes);

            if (total_compressed_bytes < uncompressed_bytes)
                return compressed_size;
            else
                return 0;
        }

        template <typename T> void compress_data( T *data, T *compressed_data, unsigned int size_data, T data_threshold)
        {
            unsigned int pos = 0;
            for (unsigned int i=0; i<size_data; i++)
                if (data[i] > data_threshold)
                {
                    compressed_data[pos] = data[i];
                    compressed_index[pos++] = i;
                }
        }

        template <typename T> void uncompress_data( T *data, T *compressed_data, unsigned int size_data )
        {
            memset(data, 0, size_data * sizeof(T));
		
	    unsigned int count = 0;
            for (int i=0; i<compressed_size; i++)
            {
                unsigned int pos = compressed_index[i];
                data[pos] = compressed_data[i];
		count++;
            }
	    printf("\n Uncompressed %lu bytes",count*sizeof(T));
        }

        template <typename T> void send_data_server_socket( T *data, unsigned int size_data )
        {
            T *buffer = new T[max_data_packet_size/sizeof(T)];
            unsigned int remaining_bytes = size_data * sizeof(T);
            unsigned int loops = 0;
            unsigned int bytes = 0;

            while(remaining_bytes > 0)
            {
                if (remaining_bytes > max_data_packet_size)
                {
                    memcpy(buffer, data + (bytes/sizeof(T)), max_data_packet_size);
                    n = write( newsockfd, (const void *)buffer, max_data_packet_size );
                    if (n < 0)
                        error("ERROR writing to socket");
                    remaining_bytes -= n;
                    bytes += n;
                    if (n < max_data_packet_size)
                        printf("\n %d Copy - %d bytes sent | %u bytes remaining", loops, n, remaining_bytes);
                }
                else
                {
                    memcpy(buffer, data + (bytes/sizeof(T)), remaining_bytes);
                    n = write( newsockfd, (const void *)buffer, remaining_bytes );
                    if (n < 0)
                        error("ERROR writing to socket");
                    bytes += n;
                    remaining_bytes -= n;
                    if (n < max_data_packet_size)
                        printf("\n %d Copy - %d bytes sent | %u bytes remaining", loops, n, remaining_bytes);
                }
                loops++;
            }
            printf("\n %d Copies - %u Bytes Sent.\n",loops,bytes);
            delete [] buffer;
        }

        template <typename T> void send_compressed_data_server_socket( T *compressed_data )
        {
            if (compressed_size > 0)
            {
                send_data_server_socket( compressed_data, compressed_size );
                send_data_server_socket( compressed_index, compressed_size );
            }
        }

        template <typename T> void receive_data_client_socket( T *data, unsigned int size_data )
        {
            T *buffer = new T[max_data_packet_size/sizeof(T)];
            unsigned int remaining_bytes = size_data * sizeof(T);
            unsigned int loops = 0;
            unsigned int bytes = 0;

            while(remaining_bytes > 0)
            {
                if (remaining_bytes > max_data_packet_size)
                {
                    n = read( sockfd, (void *)buffer, max_data_packet_size );
                    if (n < 0)
                        error("ERROR reading from socket");
                    memcpy(data + (bytes/sizeof(T)), buffer, max_data_packet_size);
                    remaining_bytes -= n;
                    bytes += n;
                }
                else
                {
                    n = read( sockfd, (void *)buffer, remaining_bytes );
                    memcpy(data + (bytes/sizeof(T)), buffer, remaining_bytes);
                    if (n < 0)
                        error("ERROR reading from socket");
                    bytes += n;
                    remaining_bytes -= n;
                }
                loops++;
            }
            printf("\n %d Copies - %lu Bytes Received.\n",loops,bytes);
            delete [] buffer;
        }

        template <typename T> void receive_compressed_data_client_socket( T *compressed_data )
        {
            if (compressed_size > 0)
            {
                receive_data_client_socket( compressed_data, compressed_size );
                receive_data_client_socket( compressed_index, compressed_size );
            }
        }


    protected:
        int n;
        int sockfd;
        int newsockfd;
        unsigned int portno;

        struct sockaddr_in cli_addr;
        socklen_t clilen;

        struct sockaddr_in serv_addr;
        struct hostent *server;

        unsigned int message_size;
        unsigned int max_data_packet_size;

        bool compressed_allocated;
        unsigned int compressed_size;
        unsigned int *compressed_index;
};

#endif // __SOCKET_CLASS_H__
