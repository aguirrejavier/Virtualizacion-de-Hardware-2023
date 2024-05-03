#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h> 

/*
    ####################   ENCABEZADO    ##########################
    #							                                 #
    #	Nombre de los archivos: procesoA.c procesoB.c            #
    #	Numero de APL : 2				                         #
    #	Numero de Ejercicio: 3		                 		     #
    #							                                 #
    #	Integrantes:								             #
    #  	Aguirre, Carlos 		    38700231	                 #
    # 	Baranda, Leonardo			36875068	                 #
    #   Rodriguez, Cesar Daniel		39166725	                 #
    # 	Sanchez, Kevin				41173649	                 #
    #                                                            #
    ###############################################################
*/

/* Filename for unicaInstancia() lock. */
#define INSTANCE_LOCK "proceso-B"
#define MY_FIFO "/tmp/myfifo"

/* Path to unicaInstancia() lock. */
static char *ooi_path;

int fd = -1;
FILE *fileSalida;

void ayuda();
void fail(const char *);
void ooi_unlink(); 
void unicaInstancia();
void manejador_signals(int ); 

int main(int argc, const char *argv[])
{
  unicaInstancia();
  
  signal(SIGINT, manejador_signals);
  signal(SIGTERM, manejador_signals); 
  
  char archivoSalida[1000];

  if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      ayuda();
      exit(0);
    } else {
      printf("Error de parametros \n");
      exit(1);
    }
  } else if (argc == 3) {
    if (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--archivo") == 0) {
      strcpy(archivoSalida, argv[2]);
    } else {
      printf("Error de parametros.\n");
      exit(2);
    }
  } else {
    printf("Erorr en la cantidad de parametros. Consulte la ayuda con -h o --help.\n");
    exit(4);
  }

  
  mkfifo(MY_FIFO, 0666);

  char buffer[1000];
  
  fd = open(MY_FIFO, O_RDONLY);
  if (fd == -1) {
    printf("Error al abrir el FIFO");
    return -1;
  }

  fileSalida = fopen(archivoSalida, "w");
  if (fileSalida == NULL) {
    printf("Error al abrir el archivo de salida");
    return -1;
  }

  while (1) {
    ssize_t cantidadLeida = read(fd, buffer, sizeof(buffer));
    if (cantidadLeida == -1) {
      printf("Error al leer del FIFO");
      return -1;
    }
    if (cantidadLeida == 0) {
      break;
    }

    fwrite(buffer, 1, cantidadLeida, fileSalida);
    fflush(fileSalida);
  }

  unlink(MY_FIFO);
  fclose(fileSalida);
  close(fd);


  return 0;
}

void ayuda()
{
  puts("**************************************************************************************************");
  puts(" - El proceso A lee el archivo pasado por parámetro linea a linea");
  puts(" - Luego, envía las lineas de forma desordenada al proceso B mediante un FIFO.");
  puts(" - Una vez que envia todas las lineas, el proceso finaliza.");
  puts(" - El proceso B, genera un archivo que contiene todas las líneas recibidas mediante el FIFO.");
  puts(" - Si el proceso A no inicializo, se quedará bloqueado esperando.");
  puts(" Parámetros: ");
  puts(" -a / --archivo : Obligatorio. Ruta del archivo a leer o escribir tanto para el proceso A como el proceso B");
  puts(" Ejemplo: ");
  puts(" ./procesoB -a salida.txt");
  puts(" ./procesoB --archivo salida.txt");
}

void fail(const char *message) {
	perror(message);
	exit(1);
}

void ooi_unlink(void) {
	unlink(ooi_path);
}

/* Si ya existe otra instancia del proceso actualmente corriendo, finaliza el proceso. */
void unicaInstancia(void) {
	struct flock fl;
	size_t dirlen;
	int fd;
	char *dir;

	/*
	 * Place the lock in the home directory of this user;
	 * therefore we only check for other instances by the same
	 * user (and the user can trick us by changing HOME).
	 */
	dir = getenv("HOME");
	if (dir == NULL || dir[0] != '/') {
		fputs("Bad home directory.\n", stderr);
		exit(1);
	}
	dirlen = strlen(dir);

	ooi_path = malloc(dirlen + sizeof("/" INSTANCE_LOCK));
	if (ooi_path == NULL) {
		fail("malloc");
  }
	memcpy(ooi_path, dir, dirlen);
	memcpy(ooi_path + dirlen, "/" INSTANCE_LOCK, sizeof("/" INSTANCE_LOCK));

	fd = open(ooi_path, O_RDWR | O_CREAT, 0600);
	if (fd < 0) {
		fail(ooi_path);
  }

	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	if (fcntl(fd, F_SETLK, &fl) < 0) {
		fputs("Otra instancia del proceso se encuentra actualmente corriendo.\n", stderr);
		exit(1);
	}

	atexit(ooi_unlink);
}

void manejador_signals(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    printf("\nSe recibio la señal SIGINT o SIGTERM! \nFinalizando recursos...\n");
    if (fd != -1) {
      close(fd);
      unlink(MY_FIFO);
    }
    if(fileSalida != NULL) {
      fclose(fileSalida);
    }
    exit(0);
  }
}