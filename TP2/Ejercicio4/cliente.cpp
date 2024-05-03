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

#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <getopt.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define NAME_MEMORY "sharedMemory"
#define SEM_MUTEX_MEMORIA "MutexMemoria"
#define SEM_MUTEX_CLIENTES "MutexClientes"
#define SEM_RESPUESTA_SERVIDOR "RespuestaServidor"
#define SEM_RESPUESTA_CLIENTE "RespuestaCliente"
#define SEM_SERVIDOR "Servidor"
#define SEM_CLIENTES "Cliente"

using namespace std;

int shm_fd;
void *ptr;
string usuario;
sem_t *semMutexMemoria,*semMutexClientes,*semRespuestaCliente,*semRespuestaServer,*semServer, *semClientes;

void mostrarAyudaCliente();
string validarParametros(int argc,char* argv[]);
void limpiarSemaforos();
void manejador_signal(int signal);
void conexionASemaforos();
bool preguntarServerActivo();
void salirSiServidorCerro();
void registrarCliente(string user, void* ptrMemoria);
void modificarMemoriaCompartida(void* ptrMemoria,string mensaje);
bool validarSend(string str);
void comandoSend(string str, string user, void* ptrMemoria);
void llenarMensaje(string& mensaje, string user);
void comandoRead(string str, string user, void* ptrMemoria);
bool validarRead(string str);
void comandoList(string str,string user,void* ptrMemoria);
bool validarList(string str);
void parsearYMostrarList(string res);
void comandoExit(string user,void* ptrMemoria);
bool validarExit(string str);

int main(int argc, char *argv[]) {
    usuario = validarParametros(argc,argv); //usuario es global
    signal(SIGINT,manejador_signal);
    conexionASemaforos();
    salirSiServidorCerro();
    sem_post(semClientes);
    int tamanio=4096;
    shm_fd = shm_open(NAME_MEMORY, O_CREAT | O_RDWR, 00666); //sh_fd es global
    ftruncate(shm_fd, tamanio);
    ptr = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,0); //ptr es global

    registrarCliente(usuario, ptr);

    while (true)
    {
        cout << "\nEscriba un comando. "<< endl;
        string str;
        getline(cin, str);
        salirSiServidorCerro();
        int pos = str.find(" ");
        string comando = str.substr(0, pos);
        if (comando == "SEND") {
            if (validarSend(str) == false)
                continue;
            comandoSend(str, usuario, ptr);
        }
        else if (comando == "READ") { // READ numero
            if (validarRead(str) == false)
                continue;
            comandoRead(str, usuario, ptr);
        }
        else if (comando == "LIST") {
            if (validarList(str) == false)
                continue;
            comandoList(str, usuario, ptr);
        }
        else if (comando == "EXIT") {
            if (validarExit(str) == false)
                continue;
            comandoExit(usuario, ptr);
        }
        else {
            cout << "Acción no reconocida: " << comando <<endl;
            cout << "Las acciones posibles son: SEND, READ, LIST y EXIT"<<endl;
        }
    }
    close(shm_fd);
    return 0;
}
void mostrarAyudaCliente()
{
    printf("\n--------------------------------------- HELP CLIENTE -----------------------------------------\n");
    printf("\n DESCRIPCION:\n");
    printf("\t Esta aplicacion brinda el servicio de mensajeria como un mailbox\n");
    printf("\t donde cada usuario puede enviar un mensaje a otro destinatario. \n");
    printf("\t Cada usuario puede enviar, recibir y listar los mensajes del servicio de mensajeria. \n");
    printf("\n SINTAXIS:\n");
    printf("\t ./Cliente -h/--help\n");
    printf("\t ./Cliente -u/--user nombreuser\n");
    printf("\t -u | --user Usuario del cliente que se quiere enviar y recibir mensaje. OBLIGATORIO \n");
    printf("\n FORMAS DE USAR:\n");
    printf("\t SEND <Destinatario>  \"<Título del Mensaje>\" <Mensaje>\n");
    printf("\t LIST\n");
    printf("\t READ <Número de Mensaje>\n");
    printf("\t EXIT\n");
    printf("\n Ejemplos: \n");
    printf("\t ./Cliente -h\n");
    printf("\t ./Cliente -u nombreUser\n");
    printf("\n----------------------------------------------------------------------------------------------\n");
    exit(EXIT_SUCCESS);
}
string validarParametros(int argc,char* argv[])
{
    string nombreUsuario;
    if (argc == 1) {
        std::cerr << "Error: El script necesita los argumentos -u/--user y un nombre\n";
        exit(3);
    } else {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                if (i + 1 < argc) {
                    std::cerr << "Error: Mas argumentos de los necesarios, por favor revise la ayuda con el argumento -h/--help\n";
                    exit(1);
                }
                mostrarAyudaCliente();
            } else if ((strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) && i + 1 < argc) {
                nombreUsuario = argv[i + 1];
                std::cout << "Usuario: " << nombreUsuario << "\n";
                i++;
                if (i + 1 < argc) {
                    std::cerr << "Error: Mas argumentos de los necesarios, por favor revise la ayuda con el argumento -h/--help\n";
                    exit(3);
                }
            } else {
                std::cerr << "Error: Argumento desconocido, por favor revise la ayuda con el argumento -h/--help\n";
                exit(3);
            }
        }
    }
    return nombreUsuario;
}
void limpiarSemaforos(){
    sem_close(semMutexMemoria);
    sem_close(semMutexClientes);
    sem_close(semRespuestaCliente);
    sem_close(semRespuestaServer);
    sem_close(semServer);
    sem_close(semClientes);
}
void manejador_signal(int signal){
    if (signal == SIGINT)
    {
        cout << "\nSaliendo del cliente.."<<endl;
        sem_wait(semMutexClientes);
        modificarMemoriaCompartida(ptr,"EXIT " + usuario);
        sem_post(semMutexClientes);
        limpiarSemaforos();
        close(shm_fd);
        exit(1);
    }
}
void modificarMemoriaCompartida(void* ptrMemoria,string mensaje){
    sem_wait(semMutexMemoria);
    //////////////////////////////////////////
    char info[1024];
    strcpy(info,mensaje.c_str());
    memcpy(ptrMemoria,info,strlen(info)+1);
    //////////////////////////////////////////
    sem_post(semRespuestaCliente);
    sem_post(semMutexMemoria);
}
bool validarSend(string str){
    stringstream ss(str);
    string comando, nombre, titulo, extra;
    ss >> comando >> nombre;// Leer el título entre comillas dobles
    if (!(ss >> std::ws && ss.get() == '\"' && std::getline(ss, titulo, '\"'))) {
        cout << "El formato del comando SEND es el siguiente: 'SEND nombre_usuario \"titulo del mensaje\""<<endl;
        cout <<"En la siguiente línea se ingresa el mensaje y este finaliza cuando se escriba una línea que contenga solamente un punto (.)"<<endl;
        return false;
    } else if (ss >> extra) {
        cout << "El formato del comando SEND es el siguiente: 'SEND nombre_usuario \"titulo del mensaje\""<<endl;
        cout <<"En la siguiente línea se ingresa el mensaje y este finaliza cuando se escriba una línea que contenga solamente un punto (.)"<<endl;
        return false;
    }
    return true;
}
void llenarMensaje(string& mensaje, string user){
    bool fin=false;
    string aux;
    while (!fin)
    {
        mensaje+="\n";
        getline(cin, aux);
        if (aux==".")
            fin=true;
        mensaje+= aux;
    }
    // Obtiene la hora actual
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm* timeinfo = std::localtime(&time_now);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%d/%m/%Y %H:%M");
    string fecha_hora_actual = oss.str();
    
    mensaje+="\n"+user;
    mensaje+="\n"+fecha_hora_actual;
}
void comandoSend(string mensaje,string user, void* ptrMemoria){
    llenarMensaje(mensaje, user);
    sem_wait(semMutexClientes);
    modificarMemoriaCompartida(ptrMemoria, mensaje);
    sem_post(semMutexClientes);
}
void comandoRead(string str, string user, void* ptrMemoria){
    string pedido=str + " " + user;
    sem_wait(semMutexClientes);
    modificarMemoriaCompartida(ptrMemoria, pedido);
    sem_wait(semRespuestaServer);
    string res(static_cast<const char*>(ptrMemoria));
    sem_post(semMutexClientes);
    cout << res <<endl;
}
bool validarRead(string str){
    std::stringstream ss(str);
    std::string comando, numero, extra;
    ss >> comando >> numero;
    try {
        int num = std::stoi(numero);
        if (num <= 0) {
            throw std::invalid_argument("Número no positivo");
        }
    } catch (const std::invalid_argument&) {
        cout << "El formato del comando READ es el siguiente: 'READ valor_numerico'"<<endl;
        cout << "El número debe ser un entero positivo mayor a 0.\n";
        return false;
    } catch (const std::out_of_range&) {
        cout << "El número es demasiado grande.\n";
        cout << "El formato del comando READ es el siguiente: 'READ valor_numerico'"<<endl;
        cout << "El número debe ser un entero positivo mayor a 0.\n";
        return false;
    }
    if (ss >> extra) {
        cout << "El formato del comando READ es el siguiente: 'READ valor_numerico'"<<endl;
        return false;
    }
    return true;
}
void comandoList(string str,string user,void* ptrMemoria){
    string pedido=str + " " + user;
    sem_wait(semMutexClientes);
    modificarMemoriaCompartida(ptrMemoria, pedido);
    sem_wait(semRespuestaServer);
    string res(static_cast<const char*>(ptrMemoria));
    sem_post(semMutexClientes);
    parsearYMostrarList(res);
}
bool validarList(string str){
    if( str != "LIST"){
        cout << "El formato del comando LIST, es el siguiente: 'LIST'"<<endl;
        return false;
    }
    return true;
}
void parsearYMostrarList(string res){
    std::cout << left << std::setw(4) << "Nro" << std::setw(12) << "Remitente" << std::setw(20) << "Enviado" << std::setw(30) << "Título" << "\n";
    int id,pos;
    string remitente, enviado, titulo;
    while ( ! res.empty())
    {
        pos = res.find(" ");
        id = stoi(res.substr(0,pos));
        res = res.substr(pos+1);
        pos = res.find(" ");
        remitente = res.substr(0,pos);
        res = res.substr(pos+1);
        pos = res.find(" ");
        enviado = res.substr(0,pos);
        res = res.substr(pos+1);
        pos = res.find(" ");
        enviado += " " + res.substr(0,pos);
        res = res.substr(pos+1);
        pos = res.find("\n");
        titulo = res.substr(0,pos);
        res = res.substr(pos+1);
        cout << left <<  setw(4) << id << setw(12) << remitente << setw(20) << enviado << setw(30) << titulo << endl;
    }
}
void comandoExit(string user,void* ptrMemoria){
    cout << "\nSaliendo del cliente.."<<endl;
    sem_wait(semMutexClientes);
    modificarMemoriaCompartida(ptrMemoria,"EXIT " + user);
    sem_post(semMutexClientes);
    limpiarSemaforos();
    close(shm_fd);
    exit(1);
}
bool validarExit(string str){
    if( str != "EXIT"){
        cout << "El formato del comando EXIT, es el siguiente: 'EXIT'"<<endl;
        return false;
    }
    return true;
}
void conexionASemaforos(){
    semMutexMemoria = sem_open(SEM_MUTEX_MEMORIA, O_CREAT, 0777, 1);
    semMutexClientes = sem_open(SEM_MUTEX_CLIENTES, O_CREAT, 0777, 1);
    semRespuestaCliente = sem_open(SEM_RESPUESTA_CLIENTE, O_CREAT, 0777, 0);
    semRespuestaServer = sem_open(SEM_RESPUESTA_SERVIDOR, O_CREAT, 0777, 0);
    semServer = sem_open(SEM_SERVIDOR, O_CREAT, 0777, 1);
    semClientes = sem_open(SEM_CLIENTES, O_CREAT, 0777, 0);
    /*if (semMutexClientes == SEM_FAILED){
        perror("Error al crear el semaforo");
        exit(EXIT_FAILURE);
    }*/
}
bool preguntarServerActivo(){
    int valor;
    sem_getvalue(semServer,&valor);
    if (valor == 1)
        return false;
    return true;
}
void salirSiServidorCerro(){
    if (! preguntarServerActivo()){
        sem_post(semMutexClientes);
        cout << "El servidor no esta activo"<<endl;
        limpiarSemaforos();
        close(shm_fd);
        exit(3);
    }
    sem_t *sem = sem_open(SEM_SERVIDOR, 0);
    if (sem == SEM_FAILED) {
        sem_post(semMutexClientes);
        cout << "El servidor no esta activo."<<endl;
        limpiarSemaforos();
        close(shm_fd);
        exit(3);
    }
}
void registrarCliente(string user,void* ptrMemoria){
    //salirSiServidorCerro();
    sem_wait(semMutexClientes);
    modificarMemoriaCompartida(ptrMemoria, user);
    sem_wait(semRespuestaServer);
    salirSiServidorCerro();
    //////////////////////////////////////////
    string res(static_cast<const char*>(ptrMemoria));
    //////////////////////////////////////////
    sem_post(semMutexClientes);

    cout << res <<endl;
    if (res.find("exito") == string::npos){
        limpiarSemaforos();
        close(shm_fd);
        exit(4);
    }
    return ;
}