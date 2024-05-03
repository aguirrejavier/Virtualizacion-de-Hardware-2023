#! /bin/bash

#---------------------ENCABEZADO------------------------#
#														                            #
#	Nombre del script: Ejercicio1.ps1					            #
#	APL Nro: 1										                      	#
# 	Ejercicio Numero 1						                 			#
#														                            #
#	Integrantes:										                      #
#  		Rodriguez, Cesar Daniel		39166725			          #
# 		Aguirre, Carlos				38700231			              #
#  		Sanchez, Kevin				41173649			              #
#		Baranda, Leonardo			36875068			                #
#                                                       #
#-------------------------------------------------------#


#Funcion de Ayuda
ayuda() {
    echo "******************************************************"
    echo "   Este Script analiza los datos de los motores       "
    echo "   de una petrolera para obtener estadisticas         "
    echo "   generales de uso y temperatura.                    "
    echo "                                                      "
    echo "Parametros:                                           "
    echo "                                                      "
    echo "--entrada | -e \"archivo\": ruta del archivo .csv a   "
    echo "procesar.                                             "
    echo "Incluye nombre del archivo.                           "
    echo "                                                      "
    echo "--archivo | -a : indica que los datos analizados      "
    echo "se guardaran en un archivo. Opcional.                 "
    echo "                                                      "
    echo "--salida | -s \"archivo\": En caso de usar --archivo, "
    echo "debe indicar la ruta del archivo a exportar.          "
    echo "Incluye nombre del archivo                            "
    echo "                                                      "
    echo "-h | --help | -?: consultar la ayuda                  "
    echo "                                                      "
    echo "******************************************************"
}

#Resolucion del problema
generarReporte() {

    awk 'BEGIN{
    FS=","
    maxMotor1=0
    maxMotor2=0
    sumaTemperaturas=0
        }
        {
            if(maxMotor1 < $2){
                maxMotor1=$2
            }

            if(maxMotor2 < $3){
                maxMotor2=$3
            }
            sumaTemperaturas+=$4
        }
        END{
                if (NR > 1){
                    promTemp=sumaTemperaturas/(NR-1)
                }
                else{
                    promTemp=sumaTemperaturas
                }

                if("'$paramArchivo'" != ""){
                    printf "Motor1=%d\n", maxMotor1 > "'"$salida"'"
                    printf "Motor2=%d\n", maxMotor2 > "'"$salida"'"
                    printf "Temperatura=%.2f", promTemp > "'"$salida"'"
                }
                else{
                    printf "\nUso maximo motor 1: %d\n", maxMotor1
                    printf "Uso maximo motor 2: %d\n", maxMotor2
                    printf "Promedio de temperatura: %.2f\n\n", promTemp
                }

            }' "$entrada"

}


if [[ $# < 1 ]]
then
    echo "Parametros insuficientes. Para ayuda, ingrese los parametros -h | --help | -?"
    exit 1
fi

while [[ $# > 0 ]]
do
  case "$1" in
  -e|--entrada)
    paramEntrada=$1
    entrada=$2
    shift
    shift
    ;;    
  -a|--archivo)
    paramArchivo=$1
    shift
    ;;
  -s|--salida)
    paramSalida=$1
    salida=$2
    shift
    shift
    ;;
  -h|--help)
    ayuda
    exit 0
    ;;
  *)
    echo "Error en el parametro "$1". Para mas informacion, ingrese a la ayuda con los parametros -h | --help"
    exit 1
    ;;
  esac
done

validarParametros () {

  if [ -z "$paramEntrada" ]
  then
    echo "El parametro \"-e\" | \"--entrada\" es obligatorio"
    exit 1
  fi

  if [ ! -w "$entrada" ]
  then
      echo "El parámetro "$entrada" no es valido. Para mas informacion, ingrese a la ayuda con los parametros -h | --help"
      exit 1
  fi

  if [ ! -z "$paramArchivo" ]
  then {
    if [ -z "$paramSalida" ]
    then
      echo "El parametro \"-s\" | \"--salida\" es obligatorio cuando se utiliza \"-a\" | \"--archivo\". Para mas informacion, ingrese a la ayuda con los parametros -h | --help"
      exit 1
    fi
    if [ ! -f "$salida" ]
    then
      touch "$salida"
    fi
  }
  elif [[ -z "$paramArchivo"  && ! -z "$paramSalida" ]]
  then
    echo "Falto el parametro \"-a\" | \"--archivo\" para realizar la salida por archivo correctamente. Para mas informacion, ingrese a la ayuda con los parametros -h | --help"
    exit 1
  fi

}

#Ejecutar reporte 
validarParametros
generarReporte