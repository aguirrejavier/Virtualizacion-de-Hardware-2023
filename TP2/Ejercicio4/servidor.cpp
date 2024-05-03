/*
#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: servidor.cpp 					#
#	APL Nro: 2											#
# 	Ejercicio Numero 4 - Reentrega									#
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Sanchez, Kevin				41173649			#
#		Baranda, Leonardo			36875068			#
#														#
#-------------------------------------------------------#
*/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <semaphore.h>
#include <vector>
#include <algorithm>
#include <iomanip>

#define NAME_MEMORY "sharedMemory"
#define SEM_MUTEX_MEMORIA "MutexMemoria"
#define SEM_MUTEX_CLIENTES "MutexClientes"
#define SEM_RESPUESTA_SERVIDOR "RespuestaServidor"
#define SEM_RESPUESTA_CLIENTE "RespuestaCliente"
#define SEM_SERVIDOR "Servidor"
#define SEM_CLIENTES "Cliente"

using namespace std;

struct mensaje {
    int id;
    string destinatario;
    string titulo;
    string mensaje;
    string remitente;
    string horaEnvio;
};

struct cliente
{
    string nombre_id;
    vector<mensaje> mensajes;
    int contMsj=0;
    bool conectado=false;
};

int shm_fd;
sem_t *semMutexMemoria,*semMutexClientes,*semRespuestaCliente,*semRespuestaServer,*semServer, *semClientes;

void validarParametros(int argc,char* argv[]);
void mostrarAyuda();
void creacionSemaforos();
void pedirSemServer();
void serverDemonio();
void limpiarSemaforos();
void manejador_signal(int signal);
void modificarMemoriaCompartida(string* ptrMemoria,string mensaje);
void llegaNuevoUsuario(vector<cliente>& clientes, string nombre, void* ptrMemoria);
void cierreUsuario(vector<cliente>& clientes, string buffer);
string buscarUsuarioConexion(vector<cliente>& clientes, string nombre);
void parsearSend(mensaje& msj,string buffer);
void comandoSend(vector<cliente>& clientes, string buffer);
cliente& buscarCliente(vector<cliente>& clientes, string nombre);// si no lo encuentra, devuelve uno nuevo con id=nombre
void agregarMensaje(cliente& cliente,mensaje msj);
void comandoRead(vector<cliente> &clientes, string buffer, void* ptrMemoria);
void parsearRead(mensaje& msj,string buffer);
bool buscarMensaje(cliente cliente,int id, mensaje &msj);
void eliminarMensaje(cliente &clienteBuscado,int id);
string darFormatoRead(mensaje msj);
void comandoList(vector<cliente> clientes,string buffer, void* ptr);
void parsearList(string buffer,string &res);
string darFormatoList(vector<mensaje> msjs);

int main(int argc, char *argv[]) {

    validarParametros(argc,argv);
    // MANEJADOR DE SENIALES //
    signal(SIGINT,manejador_signal);
    signal(SIGTERM, manejador_signal);

    creacionSemaforos();

    pedirSemServer(); //P(SERVIDOR)? continuar : detener ejecucion
    serverDemonio();
    // MEMORIA COMPARTIDA // POSIX
    int tamanio=4096;
    shm_fd = shm_open(NAME_MEMORY, O_CREAT | O_RDWR, 00666);
    ftruncate(shm_fd,tamanio);
    ////////////////////////////////////////////////////
    close(shm_fd);
    shm_fd = shm_open(NAME_MEMORY, O_RDWR, 00400);
    ////////////////////////////////////////////////////
    void* ptr;
    ptr = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,0);
    string comando;
    vector<cliente> clientes;
    while (true) {
        sem_wait(semRespuestaCliente);
        //////////////////////////////////
        string buffer(static_cast<const char*>(ptr));
        //////////////////////////////////
        int pos = buffer.find(" ");
        comando = buffer.substr(0, pos);
        if (comando == "SEND") {
            comandoSend(clientes,buffer);
        }
        else if (comando == "READ") {
            comandoRead(clientes,buffer,ptr);
        }
        else if (comando == "LIST") {
            comandoList(clientes,buffer,ptr);
        }
        else if (comando == "EXIT") {
            cierreUsuario(clientes, buffer);
        }
        else {
            llegaNuevoUsuario(clientes, comando, ptr);
        }
    }
}
void validarParametros(int argc,char* argv[]){
    if (argc == 1) {
    } else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            mostrarAyuda();
        } else {
            std::cerr << "Error: Argumento desconocido"<<endl;
            exit(3);
        }
    } else {
        std::cerr << "Error: Demasiados argumentos"<<endl;
        exit(3);
    }
}
void creacionSemaforos(){
    semMutexMemoria = sem_open(SEM_MUTEX_MEMORIA, O_CREAT, 0777, 1);
    semMutexClientes = sem_open(SEM_MUTEX_CLIENTES, O_CREAT, 0777, 1);
    semRespuestaCliente = sem_open(SEM_RESPUESTA_CLIENTE, O_CREAT, 0777, 0);
    semRespuestaServer = sem_open(SEM_RESPUESTA_SERVIDOR, O_CREAT, 0777, 0);
    semServer = sem_open(SEM_SERVIDOR, O_CREAT, 0777, 1);
    semClientes = sem_open(SEM_CLIENTES, O_CREAT, 0777, 0);
}
void pedirSemServer(){
    int valor;
    sem_getvalue(semServer,&valor);
    if (valor == 0)
    {
        cout << "Ya se encuentra un servidor activo"<<endl;
        exit(1);
    }
    sem_wait(semServer);
}
void mostrarAyuda()
{
    printf("\n--------------------------------------- HELP SERVIDOR -----------------------------------------\n");
    printf("\n DESCRIPCION:\n");
    printf("\t Esta aplicacion brinda el servicio de mensajeria como un mailbox\n");
    printf("\t donde cada usuario puede enviar un mensaje a otro destinatario. \n");
    printf("\t El mensaje consta de un destinario, un título y un cuerpo donde se escribe el texto del mismo. \n");
    printf("\n SINTAXIS:\n");
    printf("\t ./servidor -h/--help\n");
    printf("\t ./servidor\n");
    printf("\n----------------------------------------------------------------------------------------------\n");
    exit(EXIT_SUCCESS);
}
void serverDemonio(){
    pid_t pid = fork(); // Crea un nuevo proceso hijo
    if (pid < 0) {
        exit(EXIT_FAILURE); // Error al crear el proceso hijo
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS); // Proceso padre (cierre)
    }
    umask(0); // Proceso hijo (demonio)

    // Crear una nueva sesión de grupo para el demonio
    if (setsid() < 0) {
        syslog(LOG_ERR, "No se pudo crear una nueva sesión de grupo");
        exit(EXIT_FAILURE);
    }
    // Cambiar el directorio de trabajo al directorio raíz o a otro directorio seguro
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "No se pudo cambiar al directorio raíz");
        exit(EXIT_FAILURE);
    }
}
void limpiarSemaforos(){
    sem_close(semMutexMemoria);
    sem_unlink(SEM_MUTEX_MEMORIA);
    sem_close(semMutexClientes);
    sem_unlink(SEM_MUTEX_CLIENTES);
    sem_close(semRespuestaCliente);
    sem_unlink(SEM_RESPUESTA_CLIENTE);
    sem_close(semRespuestaServer);
    sem_unlink(SEM_RESPUESTA_SERVIDOR);
    sem_close(semServer);
    sem_unlink(SEM_SERVIDOR);
    sem_close(semClientes);
    sem_unlink(SEM_CLIENTES);
}
void manejador_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM){
        int cantClientes;
        sem_getvalue(semClientes,&cantClientes);
        for (size_t i = 0; i < cantClientes; i++)
        {
            sem_post(semRespuestaServer);
        }
        limpiarSemaforos();
        shm_unlink(NAME_MEMORY);
        close(shm_fd);
        exit(0);
    }
}
void modificarMemoriaCompartida(void* ptrMemoria,string mensaje){
    sem_wait(semMutexMemoria);
    ////////////////////////////////////
    char info[1024];
    strcpy(info,mensaje.c_str());
    memcpy(ptrMemoria,info,strlen(info)+1);
    ////////////////////////////////////
    sem_post(semRespuestaServer);
    sem_post(semMutexMemoria);
}
string buscarUsuarioConexion(vector<cliente>& clientes, string nombre){
    string res;
    bool encontro=false;
    for (cliente& clienteAux : clientes)
    {
        if ( clienteAux.nombre_id == nombre){
            encontro=true;
            if( clienteAux.conectado == false){
                clienteAux.conectado=true;
                res="Conexion exitosa";
            }
            else{
                res= "Un cliente con ese nombre ya se encuentra conectado";
            }
            break;
        }
    }
    if (! encontro)
    {
        cliente nuevoCliente;
        nuevoCliente.conectado=true;
        nuevoCliente.nombre_id=nombre;
        clientes.push_back(nuevoCliente);
        res="Registro y conexion exitosa.";
    }
    return res;
}
void llegaNuevoUsuario(vector<cliente>& clientes, string nombre, void* ptrMemoria){
    string res = buscarUsuarioConexion(clientes,nombre);
    modificarMemoriaCompartida(ptrMemoria,res);
}
void cierreUsuario(vector<cliente>& clientes, string buffer){
    string nombre = buffer.substr(buffer.find(" ")+1);
    for (cliente& clienteAux : clientes)
    {
        if ( clienteAux.nombre_id == nombre){
            clienteAux.conectado=false;
        }
    }
}
void parsearSend(mensaje& msj,string buffer){
    // buffer = SEND DESTINATARIO TITULO
    //          MENSAJE
    //          REMITENTE
    //          FECHA Y HORA
    int pos = buffer.find(" ");
    int pos2;
    buffer = buffer.substr(pos+1);
    pos = buffer.find(" ");
    msj.destinatario = buffer.substr(0, pos);
    pos = buffer.find("\"");
    pos2 = buffer.find("\"",pos+1);

    msj.titulo = buffer.substr(pos+1,pos2 - pos - 1);
    pos = buffer.find("\n.\n");
    msj.mensaje = buffer.substr(pos2+2, pos - pos2 - 1);

    buffer = buffer.substr(pos+3);
    msj.remitente = buffer.substr(0,buffer.find("\n"));
    msj.horaEnvio = buffer.substr(buffer.find("\n")+1);
}
cliente& buscarCliente(vector<cliente>& clientes, string nombre){
    for (cliente& clienteAux : clientes)
    {
        if ( clienteAux.nombre_id == nombre)
            return clienteAux;
    }
    cliente clienteNuevo;
    clienteNuevo.nombre_id=nombre;
    clientes.push_back(clienteNuevo);
    return clientes.back();
}
void agregarMensaje(cliente& cliente,mensaje msj){
    msj.id=++cliente.contMsj;
    cliente.mensajes.push_back(msj);
}
void comandoSend(vector<cliente>& clientes, string buffer){
    mensaje msj;
    parsearSend(msj,buffer);
    agregarMensaje(buscarCliente(clientes,msj.destinatario),msj); //inicializa el id y agrega a vector
}
void comandoRead(vector<cliente> &clientes, string buffer, void* ptrMemoria){
    mensaje msj;
    string res;
    parsearRead(msj,buffer); // READ numero remitente
    cliente &clienteBuscado = buscarCliente(clientes,msj.destinatario);
    bool encontrado = buscarMensaje(clienteBuscado,msj.id,msj);//si se encuentra devuelve true y el mensaje esta en msj
    if (encontrado){
        res = darFormatoRead(msj);
        eliminarMensaje(clienteBuscado,msj.id);
    } else {
        res = "No se encontro mensaje con ese ID";
    }
    modificarMemoriaCompartida(ptrMemoria,res);
}
void parsearRead(mensaje& msj,string buffer){
    // READ numero remitente
    int pos = buffer.find(" ");
    buffer = buffer.substr(pos+1);
    pos = buffer.find(" ");
    msj.id = stoi(buffer.substr(0,pos));
    msj.destinatario = buffer.substr(pos+1);
}
bool buscarMensaje(cliente cliente,int id, mensaje &msj){
    for (mensaje msjAux : cliente.mensajes)
    {
        if ( msjAux.id == id){
            msj.mensaje = msjAux.mensaje;
            msj.remitente = msjAux.remitente;
            msj.titulo = msjAux.titulo;
            msj.horaEnvio = msjAux.horaEnvio;
            return true;
        }
    }
    return false;
}
void eliminarMensaje(cliente &clienteBuscado,int id){
    vector<mensaje> &msj = clienteBuscado.mensajes;
    //msj.erase(std::remove(msj.begin(),msj.end(),id),msj.end());
    auto ref = find_if(msj.begin(), msj.end(), [id](const mensaje& m){
        return m.id == id;
    });
    msj.erase(ref); 
}
string darFormatoRead(mensaje msj){
    string res;
    res="Remitente: " + msj.remitente + "\n";
    res+="Enviado: " + msj.horaEnvio + "\n";
    res+="Titulo: " + msj.titulo + "\n";
    res+="Mensaje: " + msj.mensaje.substr(0,msj.mensaje.find("\n.\n"));
    return res;
}
void comandoList(vector<cliente> clientes,string buffer, void* ptrMemoria){
    string res;
    parsearList(buffer, res);//LIST remitente
    cliente clienteBuscado = buscarCliente(clientes, res);
    vector<mensaje> vectorAux = clienteBuscado.mensajes;
    res = darFormatoList(vectorAux);
    modificarMemoriaCompartida(ptrMemoria,res);
}
void parsearList(string buffer,string &res){
    int pos = buffer.find(" ");
    buffer = buffer.substr(pos+1);
    res=buffer;
}
string darFormatoList(vector<mensaje> msjs){
    string res;
    if ( ! msjs.empty()){
        for (mensaje msjAux : msjs)
            res += to_string(msjAux.id) + " " + msjAux.remitente + " "+ msjAux.horaEnvio + " " + msjAux.titulo + "\n";
    }
    return res;
}