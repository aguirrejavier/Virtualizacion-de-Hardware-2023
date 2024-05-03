/*

#---------------------ENCABEZADO------------------------#
#														#
#	Nombre del script: Ejercicio2.cpp				    	#
#	APL Nro: 2										    #
# 	Ejercicio Numero 2 - Reentrega						            #
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Sanchez, Kevin				41173649			#
#		Baranda, Leonardo			36875068			#
#                                                       #
#-------------------------------------------------------#

*/

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <filesystem>

std::mutex mtx;  // Mutex para evitar la concurrencia en la salida
std::mutex mtxDuplicados; // Mutex para evitar conflictos en el mapa mapaHashArchivos
std::unordered_map<std::string, std::vector<std::string>> mapaHashArchivos;
std::queue<std::string> archivosAProcesar;
std::condition_variable cv;
bool done = false;

void ayuda() {
    std::cout << "" << std::endl;
    std::cout << "*************Ayuda*************" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Este es un proceso que encuentra archivos duplicados" << std::endl;
    std::cout << "Imprime el path de los archivos coincidentes" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Parametros:" << std::endl;
    std::cout << "-t / --threads [nro]: Indica cantidad de threads a ejecutar (obligatorio)" << std::endl;
    std::cout << "-d / --directorio [path]: Indica el directorio a analizar (obligatorio)" << std::endl;
    std::cout << "-r / --recursivo: Indica si se debe analizar de forma recursiva los subdirectorios (opcional)" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Ejemplos de ejecución:" << std::endl;
    std::cout << "         ./ejercicio2 -t 4 -d /Home/usuario/prueba/ -r" << std::endl;
    std::cout << "" << std::endl;
    std::exit(EXIT_SUCCESS);
}

size_t miHash(const std::string& contenido) {
    size_t hash = 0;
    int contador = 1;
    for (char c : contenido) {

        hash += c*contador;
        contador++;
    }
    return hash;
}

void procesarArchivo(const std::string& pathArch) {
    std::ifstream file(pathArch);
    if (file) {
        std::string contenido((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        size_t hash = miHash(contenido);
        std::string hashStr = std::to_string(hash);
        std::lock_guard<std::mutex> lock(mtxDuplicados);
        mapaHashArchivos[hashStr].push_back(pathArch);
    }
}

void encoladorDeArchivos(const std::string& directorio, bool recursivo) {
    for (const auto& entry : std::filesystem::directory_iterator(directorio)) {
        if (entry.is_regular_file()) {
            std::string pathArch = entry.path().string();
            {
                std::lock_guard<std::mutex> lock(mtx);
                archivosAProcesar.push(pathArch);
            }
            cv.notify_one();
        } else if (recursivo && entry.is_directory()) {
            std::string subdir = entry.path().string();
            encoladorDeArchivos(subdir, recursivo);
        }
    }
}

void threadProcesador() {
    while (true) {
        std::string pathArch;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return !archivosAProcesar.empty() || done; });

            if (done && archivosAProcesar.empty()) {
                return;
            }

            pathArch = archivosAProcesar.front();
            archivosAProcesar.pop();
        }

        //Ver que hilo procesa cada archivo
      //  std::thread::id threadId = std::this_thread::get_id();

     //   std::cout << "Hilo " << threadId << " procesando archivo: " << pathArch << std::endl;


        procesarArchivo(pathArch);
        
        
    }
}

void buscarDuplicados(const std::string& directorio, int cantThreads, bool recursivo) {
    std::vector<std::thread> threadProcesadores;

    for (int i = 0; i < cantThreads; i++) {
        threadProcesadores.emplace_back(threadProcesador);
    }

    encoladorDeArchivos(directorio, recursivo);

    {
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
    }
    cv.notify_all();

    for (auto& threadProcesador : threadProcesadores) {
        threadProcesador.join();
    }
}

int main(int argc, char* argv[]) {
   
   if (argc > 6){
    std::cerr << "Demasiados parametros." << std::endl;
    ayuda();
    return 1;
   }

    int cantThreads = 0;
    std::string directorio;
    bool recursivo = false;
     
    bool threadsProvided = false;
    bool directorioProvided = false;
    bool recursivoProvided = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            ayuda();
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg == "-t" || arg == "--threads") {
                if (i + 1 < argc) {
                    try {
                      cantThreads = std::stoi(argv[i + 1]);
                    } catch (const std::invalid_argument& e) {
                      std::cout << "La cantidad de threads debe ser valor numérico." << std::endl;
                      ayuda();
                      return -1;
                    }
                    if (cantThreads <= 0) {
                        std::cerr << "El numero de threads debe ser un entero positivo." << std::endl;
                        ayuda();
                        return 1;
                    }
                    i++;  // Avanzar al siguiente parametro
                    threadsProvided = true;
                } else {
                    std::cerr << "Falta el numero de threads despues de -t / --threads." << std::endl;
                    ayuda();
                    return 1;
                }
            } else if (arg == "-d" || arg == "--directorio") {
                if (i + 1 < argc) {
                    directorio = argv[i + 1];
                    i++;  // Avanzar al siguiente parametro
                    directorioProvided = true;

                        if(!(std::filesystem::exists(directorio))){
                            std::cerr << "La carpeta no existe." << std::endl;
                            ayuda();
                            return 1;
                        }

                } else {
                    std::cerr << "Falta la ruta del directorio despues de -d / --directorio." << std::endl;
                    ayuda();
                    return 1;
                }
            } else if (arg == "-r" || arg == "--recursivo") {
                recursivo = true;
                recursivoProvided = true;
            } else {
                std::cerr << "Parametro no valido: " << arg << std::endl;
                ayuda();
                return 1;
            }
        }
    }

    
    if (!(threadsProvided && directorioProvided)) {
        std::cerr << "Faltan parametros requeridos." << std::endl;
        ayuda();
        return 1;
    }

    buscarDuplicados(directorio, cantThreads, recursivo);

    for (const auto& pair : mapaHashArchivos) {
        if (pair.second.size() > 1) {
            std::cout << "Archivos con el mismo hash (" << pair.first << "):" << std::endl;
            for (const std::string& archivo : pair.second) {
                std::cout << archivo << std::endl;
            }
        }
    }

    return 0;

}