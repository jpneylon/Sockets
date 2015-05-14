
#include "socket_class.h"

#define DEFAULT_PID             0


int main(int argc, char *argv[])
{
    int   input_port = 0;

    if (checkCmdLineFlag( argc, (const char**)argv, "portno" ))
    {
        input_port = getCmdLineArgumentInt( argc, (const char**)argv, "portno");
    }

/////////////////////////// OPEN SOCKETS
    CS_Socket *client_socket;
    client_socket = new CS_Socket();

    client_socket->socket_server = CS_Socket::ALIEN_SOCKET;
    char *socket_string;

    if (checkCmdLineFlag( argc, (const char**)argv, "socket-server" ))
    {
       getCmdLineArgumentString( argc, (const char**)argv, "socket-server", &socket_string );
    }

    if ( strcmp(socket_string,"HULK")==0 || strcmp(socket_string,"Hulk")==0 || strcmp(socket_string,"hulk")==0 )
        client_socket->socket_server = CS_Socket::HULK_SOCKET;
    else if ( strcmp(socket_string,"ALIEN")==0 || strcmp(socket_string,"Alien")==0 || strcmp(socket_string,"alien")==0 )
        client_socket->socket_server = CS_Socket::ALIEN_SOCKET;
    else if ( strcmp(socket_string,"AMAX2")==0 || strcmp(socket_string,"Amax2")==0 || strcmp(socket_string,"amax2")==0 )
        client_socket->socket_server = CS_Socket::AMAX_2_SOCKET;
    printf("\n %d - SOCKET SERVER: %d - %s", index, client_socket->socket_server, client_socket->socket_flags[ client_socket->socket_server ] );

    if (checkCmdLineFlag( argc, (const char**)argv, "socket-size" ))
    {
        int m = getCmdLineArgumentInt( argc, (const char**)argv, "socket-size");
        //client_socket->set_maximum_data_packet_size(m);
    }

///////////////////////// SUM SERVER DOSE AND NORMALIZE
    client_socket->set_port_number(index + input_port);
    printf("\n ||| %d - Port Number: %d |||", index, client_socket->get_port_number());

    client_socket->open_client_socket();

    char message[32];
    bool branch = false;
    if (index == 1) branch = true;
    sprintf(message, "%d", branch );
    client_socket->send_message_client_socket( message );

    memset(message, 0, 32);
    client_socket->receive_message_client_socket( message );

    client_socket->receive_message_client_socket( message );
    client_socket->set_compressed_size( atoi(message) );

    unsigned int total_data_size;
    if ( client_socket->get_compressed_size() > 0)
    {
        printf("\n ||| %d - Receiving compressed data from server... |||\n", index);
        client_socket->allocate_compressed_arrays();

        unsigned short *compressed_dose = new unsigned short[ client_socket->get_compressed_size() ];
        client_socket->receive_compressed_data_client_socket( compressed_dose );

        client_socket->uncompress_data( map_dose + index*shmData->size_terma, compressed_dose, shmData->size_terma );

        client_socket->free_compressed_arrays();

        total_data_size = client_socket->get_compressed_size() * (sizeof(unsigned short) + sizeof(unsigned long));
    }
    else
    {
        printf("\n ||| %d - Receiving uncompressed data from server... |||\n", index);
        client_socket->receive_data_client_socket( map_dose + index*shmData->size_terma, shmData->size_terma );
        total_data_size = shmData->size_terma * sizeof(unsigned short);
    }
    client_socket->close_socket();
    delete client_socket;

    return 0;
}



