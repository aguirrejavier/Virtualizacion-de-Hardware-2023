#!/bin/bash

#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Ejercicio4.sh					#
#	APL Nro: 1											#
# 	Ejercicio Numero 4									#
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Kevin, Sanchez				41173649			#
#		Baranda, Leonardo			36875068			#
#														#
#-------------------------------------------------------#

#----------------------------------FUNCIONES--------------------------------------------------#
mostrarAyuda(){
	echo ""
	echo "*************************************************AYUDA*******************************************"
	echo "*"
	echo "* Descripcion del Script: Este script realiza un monitoreo en segundo plano de un directorio."
	echo "*					Lo que va a estar monitoreando del directorio son las creaciones"
	echo "*					de nuevos archivos o modificaciones. Luego de cada cambio se realiza un back up"
	echo "*					del directorio y lo guarda en otro directorio. Tambien debe crear un log"
    echo "*					con los detalles de los cambios detectados"
    echo "*"
	echo "* IMPORTANTE"
	echo "* 			Para la ejecucion de este script es importante tener instalado el package inotify-tools."
	echo "*				Instalacion:"
	echo "*					sudo apt-get update"
	echo "*					sudo apt-get install inotify-tools"
	echo "*"
	echo "* Ejecuciones validas:"
	echo "*  		Este script admite 1 o 4 parametros"
	echo "*"
	echo "*		Cuando recibe un parametro:"
	echo "*				./Ejercicio4.sh        -h                Brinda la ayuda del script"
	echo "*				./Ejercicio4.sh        --help            Brinda la ayuda del script"
	echo "*"
	echo "*		Cuando recibe 4 parametros:"
	echo "*				./Ejercicio4.sh -d directorioAmonitorear -s directorioBackups"
	echo "*				./Ejercicio4.sh -s directorioBackups -d directorioAmonitorear"
	echo "*				./Ejercicio4.sh --directorio directorioAmonitorear --salida directorioBackups"
	echo "*				./Ejercicio4.sh --salida directorioBackups --directorio directorioAmonitorear"
	echo "*"
	echo "* Ejemplos de PRUEBA:"
	echo "*  		./Ejercicio4.sh -d ./Prueba/A_Monitorear/ -s ./Prueba/BackUp/"
	echo "*  		./Ejercicio4.sh -h"
    echo "*"
	echo "***************************************************************************************************"
	exit 1
}

LoggerInfo(){
	pathDirectorioMonitoreado=$(realpath "$pathMonitorear")
	pathModificado="$1"
	actionLogger="$2"
	fileLogger="$3"
	dateLogger=$(date '+%d/%m/%Y - %H:%M:%S')
	echo "${dateLogger} | DIRECTORIO_MONITOREADO = ${pathDirectorioMonitoreado} | Detalle = accion detectada ${actionLogger} PATH:${pathModificado} en el archivo ${fileLogger}" >> "Ejercicio4Log.Info"
}

Monitorear(){
	while true
	do
		inotifywait -r "$pathMonitorear" -e modify,create --quiet | while read path action file ;do
			LoggerInfo "$path" "$action" "$file"
			realizarBackup
		done
	done
}

realizarBackup(){
	dateBackUp=$(date '+%Y%m%d-%H%M%S')
	tar -cf "$pathBackups/$dateBackUp.tar.gz" "$pathMonitorear"
}
#---------------------------------- FIN FUNCIONES --------------------------------------------------#

#-------------------------------- VALIDACIONES ------------------------------------------------------#
pathMonitorear=""
pathBackups=""

if [[ -z $1 || "$1" = "--help" || "$1" = "-h" ]]
then 
    echo "Error"
	mostrarAyuda
	exit 0
fi

if [[ $# = 2 || $# = 3 || $# > 4 || $# < 2 ]]
then
	echo "Error en la cantidad de parametros"
	mostrarAyuda
	exit 0
fi

if [[ $# = 4 ]]
then
	if [[ ( "$1" != "-d" || "$3" != "-s" ) && ( "$1" != "-s" || "$3" != "-d" ) && ( "$1" != "--salida" || "$3" != "--directorio" ) && ( "$1" != "--directorio" || "$3" != "--salida" ) ]]
	then
		echo "Error en el uso de parametros"
		mostrarAyuda
		exit 0
	fi
fi

if [[ $# = 1 ]]
then
	if [[ "$1" != "--help" || "$1" != "-h" ]]
	then
		echo "Error en el uso de parametros"
		mostrarAyuda
		exit 0
	fi
fi

while [ "$*" ]
do
	case $1 in
		-d|--directorio )
			if [[ -d "$2" ]]
			then
				pathMonitorear="$2"
			else
				echo "El path indicado no existe.."
				echo "$2"" directorio invalido"
				exit 0
			fi				 
		;;
		-s|--salida )
			if [[ -d "$2" ]]
			then
				pathBackups="$2"
			else
				echo "El path indicado no existe.."
				echo "$2"" directorio invalido"
			fi	
		;;
	esac
	shift
done

#-------------------------------- FIN VALIDACIONES ------------------------------------------------------#
#-----------------CUERPO SCRIPT--------------------------#
Monitorear &
#-------------------FIN SCRIPT---------------------------#
