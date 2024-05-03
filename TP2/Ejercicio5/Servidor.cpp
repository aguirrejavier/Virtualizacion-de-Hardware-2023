/*
#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Servidor.cpp 					#
#	APL Nro: 2											#
# 	Ejercicio Numero 5 - Reentrega									#
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Kevin, Sanchez				41173649			#
#		Baranda, Leonardo			36875068			#
#														#
#-------------------------------------------------------#
*/
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h> 
#include <stdio.h>
#include <getopt.h>
#include <map>
#include <vector>
#include <stack>
#include <queue>

struct StrucMessage {
    int numeroMensaje;
    char remitente[64];
    char destinatario[64];
    char titulo[128];
    char cuerpo[1024];
    time_t horario;
    bool leido;
    int tipoMensaje;
    struct tm *info_tiempo;
    char horaFormateada[20];
};

using namespace std;

#define TAM_BUFFER 1024
#define MAX_CLIENTS 10
#define SIGNAL_DESCONEXION "DESCONEXION_CLIENTE"
#define DISCONECT_CLIENT 1
#define DISCONECT_SERVER 2
#define SEND 3
#define LIST 4
#define READ 5
#define LISTA_VACIA 6
#define ID_MSJ_NO_ENCONTRADO 7
#define EXIT 8

int client_sockets[MAX_CLIENTS];
int num_clients = 0;
int conexionesMAX, puerto = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::map<int, std::string> clientesConectados;
std::map<std::string, std::map<int,StrucMessage>> diccionarioMensajesPorCliente;

void mostrarAyuda()
{
    printf("\n---------------------------------- HELP SERVIDOR ------------------------------------\n");
    printf("\n DESCRIPCION:\n");
    printf("\t Esta aplicacion brinda el servicio de un servidor local que controla\n");
    printf("\t el trafico de mensajeria entre los clientes que se conecten al mismo. \n");
    printf("\n SINTAXIS:\n");
    printf("\t ./Servidor -h\n");
    printf("\t ./Servidor -p valor -c valor\n");
    printf("\t -p Se define un puerto para la comunicacion. OBLIGATORIO \n");
    printf("\t -c cantidad de clientes que acepta el servidor. OPCIONAL, en su defento toma 2. \n");
    printf("\t NOTA: Cantidad maxima de clientes en simultaneo: 10 \n");
    printf("\n Ejemplos: \n");
    printf("\t ./Servidor -h\n");
    printf("\t ./Seridor -p 5000\n");
    printf("\t ./Seridor -p 5000 -c 4\n");
    printf("\n-------------------------------------------------------------------------------------\n");
    exit(EXIT_SUCCESS);
}
void validarParametros(int argc,char* argv[])
{
    char *endptr;
    if (argc != 2 && argc != 3 && argc != 5){
        printf("Error en la cantidad de parametros!\n");
        mostrarAyuda();
        exit(3);
    }
    if (argc == 2) {
        if (strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"-h") == 0 ){
            mostrarAyuda();
            exit(0);
        }
        else{
            printf("Error en el uso de parametros!\n");
            mostrarAyuda();
            exit(4);
        }
    }
    if (argc == 3)
    {
        if (strcmp(argv[1],"--puerto") != 0 && strcmp(argv[1],"-p") != 0 ){
            printf("Error en el uso de parametros!\n");
            mostrarAyuda();
            exit(1);
        }
    }
    if (argc == 5)
    {
        int option;
        struct option long_options[] = {
                    {"puerto", required_argument, NULL, 'p'},
                    {"conexiones", required_argument, NULL, 'c'},
                    {0, 0, 0, 0}
        };

        while ((option = getopt_long(argc, argv, "p:c:", long_options, NULL)) != -1) {
            switch (option) {
                case 'p':
                    if (strcmp(argv[optind - 2], "--puerto") != 0 && strcmp(argv[optind - 2], "-p") != 0) {
                        fprintf(stderr, "Error: Se requiere la opcion --puertos o -p.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }

                    puerto = strtol(optarg, &endptr, 10);

                    if ((errno == ERANGE) || (errno != 0 && puerto == 0) || (*endptr != '\0')) {
                        fprintf(stderr, "Error: El valor de --puerto no es un número válido.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'c':
                    if (strcmp(argv[optind - 2], "--conexiones") != 0 && strcmp(argv[optind - 2], "-c") != 0) {
                        fprintf(stderr, "Error: Se requiere la opcion --conexiones o -c.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }
                    conexionesMAX = strtol(optarg, &endptr, 10);

                    if ((errno == ERANGE) || (errno != 0 && conexionesMAX == 0) || (*endptr != '\0')) {
                        fprintf(stderr, "Error: El valor de --conexiones no es un número válido.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }
                    break;
                default:
                    mostrarAyuda();
                    exit(EXIT_FAILURE);
            }
        }

        fflush(stdout);
    }
        
}
int obtenerClaveXUsuario(const std::map<int, std::string>& mapa, const std::string& valor) {
    for (const auto& par : mapa) {
        if (par.second == valor) {
            return par.first; 
        }
    }
    return -1; 
}
bool existe_colaMsj_xUsuario(std::string user){
    auto busqueda = diccionarioMensajesPorCliente.find(user);
    if (busqueda != diccionarioMensajesPorCliente.end()) 
        return true;
    return false;
    
}

void handle_client(int client_socket) {
    char buffer[TAM_BUFFER];
    struct StrucMessage structMensaje;
    while (1) {
        memset(&structMensaje, 0, sizeof(structMensaje));
        int bytes_received = recv(client_socket, &structMensaje, sizeof(structMensaje), 0);
        if (bytes_received <= 0) {
            printf("Cliente desconectado\n");
            close(client_socket);
            return;
        }
        if(structMensaje.tipoMensaje == 1){
            printf("Se desconecto el cliente: %d\n",client_socket);
            clientesConectados.erase(client_socket);
            strncpy(structMensaje.cuerpo, "Un nuevo usuario acaba abandonar la sala..\n", sizeof(structMensaje.cuerpo));
            
            pthread_mutex_lock(&mutex);
            for (const auto& cliente : clientesConectados) {
                if (cliente.first != client_socket) {
                     send(cliente.first, &structMensaje, sizeof(structMensaje), 0);
                }
            }
            pthread_mutex_unlock(&mutex);
            close(client_socket);
            num_clients = num_clients - 1;
            continue;
        }
        switch (structMensaje.tipoMensaje) {
            case 3:
                if(existe_colaMsj_xUsuario(structMensaje.destinatario)){
                    std::map<int,StrucMessage> copiaColaMsj= diccionarioMensajesPorCliente[structMensaje.destinatario];
                    auto iteradorExterno = copiaColaMsj.rbegin();
                    StrucMessage ultimoMensaje = iteradorExterno->second;
                    structMensaje.numeroMensaje = ultimoMensaje.numeroMensaje + 1;
                }else{
                    structMensaje.numeroMensaje = 1;
                }
                diccionarioMensajesPorCliente[structMensaje.destinatario][structMensaje.numeroMensaje] = structMensaje;
                break;
            case 4:
                if(existe_colaMsj_xUsuario(structMensaje.remitente)){
                    //auto iterador = diccionarioMensajesPorCliente.find(structMensaje.remitente);
                    pthread_mutex_lock(&mutex);
                    std::map<int,StrucMessage> copiaColaMsj= diccionarioMensajesPorCliente[structMensaje.remitente];
                    pthread_mutex_unlock(&mutex);
                    // while (!copiaColaMsj.empty()) {
                    //     StrucMessage mensajeActual = copiaColaMsj.front();
                    //     if(mensajeActual.leido == false){
                    //         mensajeActual.tipoMensaje = 4;
                    //         send(client_socket, &mensajeActual, sizeof(mensajeActual), 0); 
                    //     }
                    //     copiaColaMsj.pop();
                    // }
                    int contador = 0;
                    for (const auto& mensajeRecibido : copiaColaMsj) {
                        
                        StrucMessage mensajeActual = mensajeRecibido.second;
                        if(mensajeActual.leido == false){
                            mensajeActual.tipoMensaje = 4;
                            send(client_socket, &mensajeActual, sizeof(mensajeActual), 0); 
                            contador ++;
                        }
                    }
                    StrucMessage mensajeActual;
                    if(contador == 0){
                        mensajeActual.tipoMensaje = 6;
                    }else{
                        strcpy(mensajeActual.titulo,"END_LIST");
                    }
                    send(client_socket, &mensajeActual, sizeof(mensajeActual), 0);
                }
                else{
                    StrucMessage mensajeActual;
                    mensajeActual.tipoMensaje = 6;
                    send(client_socket, &mensajeActual, sizeof(mensajeActual), 0);
                }
                break;
            case 5:
                if(existe_colaMsj_xUsuario(structMensaje.remitente)){
                    //auto iterador = diccionarioMensajesPorCliente.find(structMensaje.remitente);
                    pthread_mutex_lock(&mutex);
                    std::map<int,StrucMessage> copiaColaMsj= diccionarioMensajesPorCliente[structMensaje.remitente];
                    pthread_mutex_unlock(&mutex);
                    auto iterador = copiaColaMsj.find(structMensaje.numeroMensaje);
                    if (iterador != copiaColaMsj.end()) {
                        StrucMessage mensajeActual = iterador->second;
                        if(mensajeActual.leido == false){
                             mensajeActual.tipoMensaje = 5;
                            //MANDO EL MENSAJE
                            send(client_socket, &mensajeActual, sizeof(mensajeActual), 0);
                            //MODIFICO EL MENSAJE
                            mensajeActual.leido = true;
                            diccionarioMensajesPorCliente[structMensaje.remitente][structMensaje.numeroMensaje] = mensajeActual;
                        }else{
                            mensajeActual.tipoMensaje = 7;
                            send(client_socket, &mensajeActual, sizeof(mensajeActual), 0);
                        }
                    }
                    else{
                        StrucMessage mensajeActual;
                        mensajeActual.tipoMensaje = 7;
                        send(client_socket, &mensajeActual, sizeof(mensajeActual), 0);
                    }
                }
                else{
                        StrucMessage mensajeActual;
                        mensajeActual.tipoMensaje = 7;
                        send(client_socket, &mensajeActual, sizeof(mensajeActual), 0);
                }
                break;
            case 8:
                  clientesConectados.erase(client_socket);
                  close(client_socket);
                  num_clients = num_clients - 1;
        }
    }
}
void limpiarConexiones() {
    char mensaje_desconexion[] = "DESCONEXION_SERVIDOR";
    struct StrucMessage structMensaje;
    structMensaje.tipoMensaje = 2;
    for (const auto& cliente : clientesConectados) {
        send(cliente.first, &structMensaje, sizeof(structMensaje), 0);
        std::cout << "usuario desconectado: " << cliente.second << std::endl;
        close(cliente.first);
    }
}
void manejador_signals(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("\nSe recibio la señal SIGINT o SIGTERM! \nCerrando servidor y las conexiones de los clientes...\n");
        limpiarConexiones();
        exit(0);
    }
}

bool existe_usuario(std::string user){

    auto iter = clientesConectados.begin();
    for (; iter != clientesConectados.end(); ++iter) {
        if (iter->second == user) {
            break;  
        }
    }
    if (iter != clientesConectados.end()) {
        return true;
    } else {
        return false;
    }
}
void iniciarDemonio() {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    umask(0);
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main (int argc, char* argv[]) 
{
    validarParametros(argc,argv);
    //VALIDAR PARAMETROS
    int conexiones;
    if(argc == 3){ 
        conexionesMAX = 2;
        puerto = atoi(argv[2]); 
    }

    if(puerto <= 0 || (conexionesMAX < 2 || conexionesMAX > MAX_CLIENTS)){
        printf("ERROR! Puerto debe ser mayor a 0 y conexiones mayor o igual a 2\n");
        mostrarAyuda();
        exit(6);
    }
    // Configurar manejador de señales
    signal(SIGINT, manejador_signals);
    signal(SIGTERM, manejador_signals); 

    struct sockaddr_in serverConfiguracion;
    memset(&serverConfiguracion, '0', sizeof(serverConfiguracion));

    serverConfiguracion.sin_family = AF_INET; // 127.0.0.1
    serverConfiguracion.sin_addr.s_addr = htonl(INADDR_ANY);
    serverConfiguracion.sin_port = htons(puerto); // Mayor 1023

    int socketEscucha = socket(AF_INET, SOCK_STREAM, 0);
    int resultadoServidor = bind(socketEscucha, (struct sockaddr *)&serverConfiguracion, sizeof(serverConfiguracion));
    if(resultadoServidor == -1) { 
        perror("Error al querer configurar la comunicacion\n");
        close(socketEscucha);
        exit(1);
    } 
    // cant de conexines que acepta el servidor
    if (listen(socketEscucha, conexionesMAX) == -1) {
        perror("Error al configurar la escucha del servidor\n");
        exit(1);
    }
    char buffer[TAM_BUFFER];
    iniciarDemonio();
    while (true)  
    {
        int socketComunicacion = accept(socketEscucha, (struct sockaddr *)NULL, NULL);
         if (socketComunicacion == -1) {
            perror("Error al crear el socket del servidor");
            exit(1);
        }
        if (num_clients < conexionesMAX) {

            int bytesReceived = recv(socketComunicacion, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string username(buffer);

                if(existe_usuario(username)){
                    StrucMessage structMensaje;
                    structMensaje.tipoMensaje = 97;
                    send(socketComunicacion, &structMensaje, sizeof(structMensaje), 0);
                    close(socketComunicacion);
                    continue;
                }

                client_sockets[num_clients] = socketComunicacion; 
                clientesConectados[socketComunicacion] = username;
                num_clients++;
                printf("Nuevo cliente conectado en el socket %d\n", socketComunicacion);
            }
            // char bienvenida_cliente[] = "Un nuevo usuario se acaba de unir a la sala...\n";
            // pthread_mutex_lock(&mutex);
            // for (const auto& cliente : clientesConectados) {
            //     if (cliente.first != socketComunicacion) {
            //          send(cliente.first, bienvenida_cliente, strlen(bienvenida_cliente), 0);
            //     }
            // }
            // pthread_mutex_unlock(&mutex);
            StrucMessage structMensaje;
            structMensaje.tipoMensaje = 98;
            send(socketComunicacion, &structMensaje, sizeof(structMensaje), 0);
        } else {
            printf("Máximo de clientes alcanzado. No se aceptan nuevas conexiones.\n");
            StrucMessage structMensaje;
            structMensaje.tipoMensaje = 99;
            send(socketComunicacion, &structMensaje, sizeof(structMensaje), 0);
            close(socketComunicacion);
        }
        thread th(handle_client, socketComunicacion);
        th.detach();        
    }
    close(socketEscucha);
    return EXIT_SUCCESS; 
} 