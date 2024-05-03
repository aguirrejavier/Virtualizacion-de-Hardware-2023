#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/*
    ####################   ENCABEZADO    ##########################
    #							                                 #
    #	Nombre de los archivos: ejercicio1.c            #
    #	Numero de APL : 2				                         #
    #	Numero de Ejercicio: 1		                 		     #
    #							                                 #
    #	Integrantes:								             #
    #  	Aguirre, Carlos 		    38700231	                 #
    # 	Baranda, Leonardo			36875068	                 #
    #   Rodriguez, Cesar Daniel		39166725	                 #
    # 	Sanchez, Kevin				41173649	                 #
    #                                                            #
    ###############################################################
*/

void ayuda();
void imprimir(int, pid_t , pid_t );

int main(int argc, const char *argv[])
{
  
  //Validacion de parámetro de ayuda.
  if (argc > 1) {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
      ayuda();
      exit(0);
    } else {
      printf("Error en los parámetros. Para consultar la ayuda ingrese -h | --help\n");
      exit(-1);
    }
  }
  
  pid_t pid = fork();

  if (pid == 0) {
    //Proceso3
    imprimir(3, getpid(), getppid());
    
    pid = fork();
    if (pid == 0) {
      //Proceso6
      imprimir(6, getpid(), getppid());
      
      pid = fork();
      if (pid == 0) {
        //Proceso9
        imprimir(9, getpid(), getppid());

        pid = fork();
        if (pid == 0) {
          //Proceso11
          imprimir(11, getpid(), getppid());
        }
      }
    }
  } else if(pid > 0) {
    //Proceso1
    imprimir(1, getpid(), getppid());
    pid = fork();
    
    if (pid == 0) {
      //Proceso4
      imprimir(4, getpid(), getppid());

      pid = fork();
      if (pid == 0) {
        //Proceso7
        imprimir(7, getpid(), getppid());

        pid = fork();
        if (pid == 0) {
          //Proceso10
          imprimir(10, getpid(), getppid()); 
        }
      } else if (pid > 0) {
        pid = fork();
        if(pid == 0) {
          //Proceso8
          imprimir(8, getpid(), getppid());
        }
      }
    } else {
      pid = fork();
      if (pid == 0) {
        //Proceso2
        imprimir(2, getpid(), getppid());
        setsid();

        pid_t pid5 = fork();
        int status;
        
        if (pid5 == 0) {
          //Proceso5
          imprimir(5, getpid(), getppid());
          while(1){}
        } else {
          waitpid(pid5, &status, WUNTRACED);
        }
      }
    }
  }

  sleep(10);
  
  return 0;
}

void ayuda()
{
    puts("****Ayuda****");
    puts("Proceso que genera una jerarquia de procesos");
    puts("Imprime el Numero de proceso, cual es su PID y cual es el padre.");
    puts("El proceso durará 10 segundos para tener tiempo de verificar con el comando ps y pstree.");
    puts("No se requieren pasar parametros para ejecutar el proceso.");
    puts("Ejemplo de ejeución: ");
    puts("  ./ejercicio1");
    exit(EXIT_SUCCESS);
}

void imprimir(int nroProceso, pid_t pid, pid_t pidPadre)
{
  printf("Proceso %d - PID %d - PID Padre: %d\n", nroProceso, pid, pidPadre);
}