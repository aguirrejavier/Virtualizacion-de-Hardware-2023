/*
#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Cliente.cpp   					#
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
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <getopt.h>

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

#define SIGNAL_DESCONEXION "DESCONEXION_SERVIDOR"
#define TAM_BUFFER 1024

char usuario[20];
char SERVIDOR[20];
pthread_t recv_thread; 
int socketServidor, PUERTO;

void mostrarAyuda()
{
    printf("\n--------------------------------------- HELP CLIENTE -----------------------------------------\n");
    printf("\n DESCRIPCION:\n");
    printf("\t Esta aplicacion brinda el servicio de un Cliente que se conecta al servidor\n");
    printf("\t para realizar en intercambio de mensajes con otos clientes. \n");
    printf("\n SINTAXIS:\n");
    printf("\t ./Cliente -h\n");
    printf("\t ./Cliente -u user -p puerto -s servidor\n");
    printf("\t -u | --user Usuario del cliente que se quiere comunicar. OBLIGATORIO \n");
    printf("\t -p | --puerto Se define un puerto para la comunicacioncon el servidor. OBLIGATORIO \n");
    printf("\t -s | --servidor Servidor a donde se quiere conectar para iniciar el servicio de mensajeria. OBLIGATORIO \n");
    printf("\n Ejemplos: \n");
    printf("\t ./Cliente -h\n");
    printf("\t ./Cliente -u cliente2 -p 5000 -s 127.0.0.1\n");
    printf("\t ./Cliente ./Cliente -u carlos -p 5000 -s 127.0.0.1\n");
    printf("\t ./Cliente -u cliente2 -p 5056 -s 127.0.0.1\n");
    printf("\n----------------------------------------------------------------------------------------------\n");
    exit(EXIT_SUCCESS);
}
int esAlfanumerico(const char *cadena) {
    if (strlen(cadena) == 0) {
        return 0; 
    }

    for (size_t i = 0; i < strlen(cadena); i++) {
        if (!isalnum((unsigned char)cadena[i])) {
            return 0;
        }
    }
    return 1; 
}
void validarParametros(int argc,char* argv[])
{
    if (argc != 2 && argc != 7){
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
    if (argc == 7)
    {
        int option;
        char *endptr;
        struct option long_options[] = {
                    {"user", required_argument, NULL, 'u'},
                    {"puerto", required_argument, NULL, 'p'},
                    {"servidor", required_argument, NULL, 's'},
                    {0, 0, 0, 0}
        };

        while ((option = getopt_long(argc, argv, "u:p:s:", long_options, NULL)) != -1) {
            switch (option) {
                case 'u':
                    if (strcmp(argv[optind - 2], "--user") != 0 && strcmp(argv[optind - 2], "-u") != 0) {
                        fprintf(stderr, "Error: Se requiere la opcion --user o -u.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }
                    if(esAlfanumerico(optarg)){
                        strcpy(usuario, optarg);
                        break;
                    }
                    else{
                        printf("\n EL nombre de usuario debe ser Alfanumerico!\n");
                        exit(10);
                    }
                case 'p':
                    if (strcmp(argv[optind - 2], "--puerto") != 0 && strcmp(argv[optind - 2], "-p") != 0) {
                        fprintf(stderr, "Error: Se requiere la opcion --puertos o -p.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }
                    PUERTO = strtol(optarg, &endptr, 10);
                    if ((errno == ERANGE) || (errno != 0 && PUERTO == 0) || (*endptr != '\0')) {
                        fprintf(stderr, "Error: El valor de --puerto/-p no es un número válido.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 's':
                    if (strcmp(argv[optind - 2], "--servidor") != 0 && strcmp(argv[optind - 2], "-s") != 0) {
                        fprintf(stderr, "Error: Se requiere la opcion --servidor o -s.\n");
                        mostrarAyuda();
                        exit(EXIT_FAILURE);
                    }
                    strcpy(SERVIDOR, optarg);
                    break;
                default:
                    mostrarAyuda();
                    exit(EXIT_FAILURE);
            }
        }
        fflush(stdout);
    }
}
void manejador_signals(int signal) {
    char mensaje_desconexion[] = "DESCONEXION_CLIENTE";
    struct StrucMessage structMensaje;
    structMensaje.tipoMensaje = 1;

    send(socketServidor, &structMensaje, sizeof(structMensaje), 0);
    close(socketServidor);
    exit(0);
}

void *receive_messages(void *arg) {
    struct StrucMessage structMensaje;
    while (1) {
        fflush(stdout);
        memset(&structMensaje, 0, sizeof(structMensaje));
        int bytes_received = recv(socketServidor, &structMensaje, sizeof(structMensaje), 0);
        if (bytes_received <= 0) {
            printf("El servidor se desconecto.\n");
            close(socketServidor);
            exit(1);
        }
        if(structMensaje.tipoMensaje == 99){
            printf("Cantidad maxima de clientes conectados en simultaneo!\n");
            close(socketServidor);
            exit(99);
        }
        if(structMensaje.tipoMensaje == 98){
            printf("Conexion exitosa!\n");
            memset(&structMensaje, 0, sizeof(structMensaje));
        }
        if(structMensaje.tipoMensaje == 97){
            printf("Error al conectarse al servidor! Ya existe un usuario conectado con ese nombre!\n");
            close(socketServidor);
            exit(99);
        }
        if(structMensaje.tipoMensaje == 2){
            printf("\nSe cerro el Servidor! Desconectando cliente..\n");
            close(socketServidor);
            exit(10);
        }
        if(structMensaje.tipoMensaje == 4){
            fflush(stdout);
            printf("Nro\tRemitente\tEnviado\t\t\tTitulo\n");
            do {
                printf("%d\t%-10s\t%-20s\t%s\n", structMensaje.numeroMensaje, structMensaje.remitente, structMensaje.horaFormateada, structMensaje.titulo);
                bytes_received = recv(socketServidor, &structMensaje, sizeof(structMensaje), 0);
            } while (strcmp(structMensaje.titulo,"END_LIST") != 0 );
            memset(&structMensaje, 0, sizeof(structMensaje));
            fflush(stdout);
        }
         if(structMensaje.tipoMensaje == 5){
             fflush(stdout);
             printf("Remitente: %s\n", structMensaje.remitente);
             printf("Enviado: %s\n", structMensaje.horaFormateada);
             printf("Titulo: %s\n", structMensaje.titulo);
             printf("Mensaje: %s\n", structMensaje.cuerpo);
             fflush(stdout);
             memset(&structMensaje, 0, sizeof(structMensaje));
         }
        if(structMensaje.tipoMensaje == 6){
            fflush(stdout);
            printf("No tiene mensajes recibidos!\n");
            fflush(stdout);
        }
        if(structMensaje.tipoMensaje == 7){
            printf("No se encontro mensaje con ese ID\n");
        }
    }
    close(socketServidor);
    return NULL;
}
int esCadenaVacia(const char *cadena) {
    int len = strlen(cadena);
    for (int i = 0; i < len; i++) {
        if (cadena[i] != ' ') {
            return 0;
        }
    }
    return 1; 
}

int main (int argc, char* argv[]) 
{   
    validarParametros(argc,argv);
    
    // Configurar manejador de señales
    signal(SIGINT, manejador_signals);
    struct StrucMessage structMensaje;
    struct sockaddr_in clientConfiguracion;
    memset(&clientConfiguracion, '0', sizeof(clientConfiguracion));
    clientConfiguracion.sin_family = AF_INET;
    clientConfiguracion.sin_port = htons(PUERTO);
    inet_pton(AF_INET, SERVIDOR, &clientConfiguracion.sin_addr);

    socketServidor = socket(AF_INET, SOCK_STREAM, 0);
    printf("Conectadose al servidor...\n");
    int resultadoConexion = connect(socketServidor,
        (struct sockaddr *)&clientConfiguracion, sizeof(clientConfiguracion));
    
    if(resultadoConexion == -1) { 
        perror("Error al conectarse al servidor!\n");
        close(socketServidor);
        exit(1);
    } 
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("Error al crear el hilo de recepción\n");
        close(socketServidor);
        exit(1);
    } 

    struct StrucMessage structSEND;
    send(socketServidor, usuario, strlen(usuario), 0);
    int bytesRecibidos = 0;
    char cadena[256]; 
    char mensaje[1000];
    char input[256];
    time_t tiempo_actual;

    while (true) {
        fflush(stdout);
        fgets(input, sizeof(input), stdin);
        strcpy(cadena,input);
        input[strcspn(input, "\n")] = '\0';

        // Analizar el comando ingresado
        char *command = strtok(input, " ");  
        if (command == NULL) {
            continue; 
        }
        
        if (strcmp(command, "SEND") == 0) {
            char destinatario[256], titulo[256], mensaje[1024];
            strcpy(structSEND.remitente,usuario);
            structSEND.leido=false;
            if (sscanf(cadena, "SEND %255s %255[^\n]", structSEND.destinatario, structSEND.titulo) != 2) {
                printf("Formato incorrecto del comando SEND.\n");
                continue;
            }
            if(esAlfanumerico(structSEND.destinatario) == 0){
                printf("El destinatario debe ser Alfanumerico!\n");
                continue;
            }
            mensaje[0] = '\0';
            while (1) {
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                if (strcmp(input, ".") == 0) {
                    break; 
                }
                strcat(mensaje, input);
                strcat(mensaje, "\n");
            }

            strcpy(structSEND.cuerpo,mensaje);
            structSEND.tipoMensaje = 3;
            //TOMAR HORA ACTUAL Y FORMATEARLA EN UNA CADENA
            time(&tiempo_actual);
            structSEND.info_tiempo = std::localtime(&tiempo_actual);
            char buffer[80];
            std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", structSEND.info_tiempo);
            strcpy(structSEND.horaFormateada, std::string(buffer).c_str());
            //ENVIAR EL MENSAJE AL SERVIDOR
            send(socketServidor, &structSEND, sizeof(structSEND), 0);
        } else if (strcmp(command, "LIST") == 0) {
         
            strcpy(structSEND.remitente,usuario);
            structSEND.tipoMensaje = 4;
            send(socketServidor, &structSEND, sizeof(structSEND), 0);
            fflush(stdout);
        } else if (strcmp(command, "READ") == 0) {
            strcpy(structSEND.remitente,usuario);
            structSEND.tipoMensaje = 5;
            int elementosLeidos = sscanf(cadena, "READ %d", &structSEND.numeroMensaje);
            if (elementosLeidos == 1) {
                send(socketServidor, &structSEND, sizeof(structSEND), 0); 
            }
            else{
                printf("\nEntrada invalida. Debes ingresar un numero para leer un mensaje.\n");
                fflush(stdin);
                continue;
            }
            fflush(stdout);
        } else if (strcmp(command, "EXIT") == 0) {
            strcpy(structSEND.remitente,usuario);
            structSEND.tipoMensaje = 8;
            send(socketServidor, &structSEND, sizeof(structSEND), 0);
            close(socketServidor);
            fflush(stdin);
            exit(1);
        } else {
            printf("Comando no válido. Intente de nuevo.\n");
            fflush(stdin);
        }
    }
    close(socketServidor);
    return EXIT_SUCCESS;
}