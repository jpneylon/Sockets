
#include "socket_class.h"

char message[32];
CS_Socket *server_socket;

/* SOCKET OPERATIONS
        unsigned short thresh = 655;
        unsigned int compressed_size = server_socket->calculate_compressed_data_size( dose, clientData->size_terma, thresh );

        sprintf(message, "%u", compressed_size );
        server_socket->send_message_server_socket( message );

        if (compressed_size > 0)
        {
            printf("\n Compressing data and sending to client...\n");
            unsigned short *compressed_dose = new unsigned short[compressed_size];
            memset( compressed_dose, 0, compressed_size * sizeof(unsigned short) );
            server_socket->allocate_compressed_arrays();

            server_socket->compress_data( dose, compressed_dose, clientData->size_terma, thresh );

            server_socket->send_compressed_data_server_socket( compressed_dose );

            server_socket->free_compressed_arrays();
        }
        else
        {
            printf("\n No advantage to compressing data, sending data to client...\n");
            server_socket->send_data_server_socket( dose, clientData->size_terma );
        }

        server_socket->close_new_socket();
*/



int main(int argc, char *argv[])
{
    server_socket = new CS_Socket;

    server_socket->set_port_number(0);
    if (checkCmdLineFlag( argc, (const char**)argv, "portno" ))
    {
        int p = getCmdLineArgumentInt( argc, (const char**)argv, "portno");
        server_socket->set_port_number(p);
    }

    server_socket->open_server_socket();

    while(true)
    {
        server_socket->listen_server_socket();

        int child_id = fork();
        if (child_id == 0)
        {
            server_socket->receive_message_server_socket( message );

            sprintf(message,"%d initiated",server_socket->get_port_number());
            server_socket->send_message_server_socket( message );

            //printf("\n ID %d performing convolution...\n",child_id);

            server_socket->close_socket();

            //run_server_convolution(argc,argv);

            printf("\n Child terminating...\n ||||||||||||||||||||||||||||||||||||||||||||||||||||||||| \n");

            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("\n ID %d waiting for child to exit...\n",child_id);
            server_socket->close_new_socket();
            int status;
            waitpid(child_id, &status, 0);
            //printf("\n Child process terminated with status %d\n",status);
            if ( !WIFEXITED(status) )
            {
                printf("\n Child process termination status failed.\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    delete [] message;
    server_socket->close_socket();
    delete server_socket;

    printf("\n\n");
    return 0;
}



