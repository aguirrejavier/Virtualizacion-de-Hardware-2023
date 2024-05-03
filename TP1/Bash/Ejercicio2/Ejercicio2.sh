#!/bin/bash

#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Ejercicio2.sh					#
#	APL Nro: 1											#
# 	Ejercicio Numero 2									#
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Kevin, Sanchez				41173649			#
#		Baranda, Leonardo			36875068			#
#														#
#-------------------------------------------------------#

#----------------------------------FUNCIONES--------------------------------------------------#
MostrarAyuda() {
echo -e "Uso del script:
                ./Ejercicio2.sh -e archivoEntrada -s separador
        
        -e / --entrada              archivo donde se encuentra la matriz a leer
        -s / --separador            caracter utilizado en la matriz para diferenciar valores


El orden de ambos parametros y sus argumentos pueden ser intercambiados
Es posible ejecutar el script solo con el parametro --entrada y su argumento,
en dicho caso se utilizara la coma ',' como caracter separador.
        
                Ej: ./Ejercicio2.sh --entrada archivoEntrada


El scrip lee la matriz en el archivo de entrada e informa por pantalla:
    • Orden de la matriz.
    • Si es cuadrada.
    • Si es identidad.
    • Si es nula.
    • Si es fila.
    • Si es columna.

Posibles salidas:
    0       El script termino sin problemas
    1       Comando --help, -h, -?
    2       Error en la cantidad o nombre de los parametros
    3       Archivo de entrada inexistente
    4       Archivo de entrada vacio
    5       Archivo de entrada sin permiso de lectura
    6       Caracter de separador invalido
    7       Matriz invalida
        "    
        exit 1
}
ErrorParametrosIncorrectos()
{
    echo "Error en la cantidad o nombre de los parametros"
    exit 2
}
ErrorMatrizInvalida()
{
    echo "MATRIZ INVALIDA: $1"
    exit 7
}
ValidacionArchivoEntrada()
{
    if ! [[ -f "$1" ]]; then
        echo "Error: archivo '$1' no existe"
        exit 3
    fi
    if ! [[ -s "$1" ]]; then
        echo "Error: El archivo pesa 0 bytes"
        exit 4
    fi
    if ! [[ -r "$1" ]]; then
        echo "Error: Archivo de texto sin permiso de lectura"
        exit 5
    fi
}
ValidacionSeparador()
{
    re='^[0-9]+$'
    if [[ $1 =~ $re ]] ; then
        echo "Error: Separador invalido"
        exit 6
    fi
    if [[ $1 =~ '-' ]] ; then
        echo "Error: Separador invalido"
        exit 6
    fi
}
InformePorPantalla()
{
    if [[ $2 -eq 0 ]]; then
        echo "$1 SI"
    else
        echo "$1 NO"
    fi
}
#-------------------------------------- FIN FUNCIONES -----------------------------------------------#

#-------------------------------------- VALIDACIONES ------------------------------------------------#

for arg in "$@"; do
    if [ "$arg" == "--help" ] || [ "$arg" == "-h" ] || [ "$arg" == "-?" ]; then
        MostrarAyuda
    fi
done
separador=','
if test $# -eq 2; then
    if [[ $1 = "-e" ]] || [[ $1 = "--entrada" ]]; then
        ValidacionArchivoEntrada "$2"
        archivoEntrada="$2"
    else
        ErrorParametrosIncorrectos
    fi
elif [[ $# -eq 4 ]]; then
    if [[ $1 = "-e" ]] || [[ $1 = "--entrada" ]]; then
        if [[ $3 = "-s" ]] || [[ $3 = "--separador" ]]; then
            ValidacionArchivoEntrada "$2"
            ValidacionSeparador $4
            archivoEntrada="$2"
            separador=$4
        else
            ErrorParametrosIncorrectos
        fi
    elif [[ $1 = "-s" ]] || [[ $1 = "--separador" ]]; then
        if [[ $3 = "-e" ]] || [[ $3 = "--entrada" ]]; then
            ValidacionArchivoEntrada "$4"
            ValidacionSeparador $2
            archivoEntrada="$4"
            separador=$2
        else
            ErrorParametrosIncorrectos
        fi
    else
        ErrorParametrosIncorrectos
    fi
else
    ErrorParametrosIncorrectos
fi
#-------------------------------- FIN VALIDACIONES ------------------------------------------------------#

#------------------------------------------ CUERPO SCRIPT ---------------------------------------------------#
cantFila=0
re='^[-+]?[0-9]+$'
esNula=0
esIdentidad=0
esCuadrada=1
esFila=1
esColumna=1
while IFS= read -r fila
do
    IFS=$separador
    read -a numArr <<< "$fila"
    cantColumnas=${#numArr[*]}
    if [[ $cantColumnas -eq 0 ]]; then
        ErrorMatrizInvalida
    fi
    if [[ $cantFila -eq 0 ]]; then
        cantColumnasAnterior=$cantColumnas
    fi
    if ! [[ $cantColumnas -eq $cantColumnasAnterior ]]; then
        ErrorMatrizInvalida "Cantidad de columnas no constante"
    fi
    regex="$separador$"
    if [[ "$fila" =~ $regex ]]; then
        ErrorMatrizInvalida "Ultimo valor de fila no valido"
    fi 
    if [[ "$fila" =~ $separador$separador ]]; then
        ErrorMatrizInvalida "Valores vacios"
    fi 

    for (( i=0; i<$cantColumnas; i++ ))
        do
        if ! [[ ${numArr[$i]} =~ $re ]]; then
            ErrorMatrizInvalida "Valores no numericos en la matriz"
        fi
        if [[ ${numArr[$i]} -ne 0 ]]; then
            esNula=1
        fi
        if [[ $cantFila -eq $i ]]; then
            if [[ ${numArr[$i]} -ne 1 ]]; then
                esIdentidad=1
            fi
        else
            if [[ ${numArr[$i]} -ne 0 ]]; then
                esIdentidad=1
            fi
        fi
        #echo Elemento $i: ${numArr[$i]}
    done

    ((cantFila++))
done < "$archivoEntrada"

if [[ $cantFila -eq $cantColumnas ]]; then
    esCuadrada=0
fi
if [[ $cantFila -eq 1 ]]; then
    esFila=0
fi
if [[ $cantColumnas -eq 1 ]]; then
    esColumna=0
fi
ordenMatriz=$((cantFila*cantColumnas))
echo -e "\tInforme de la matriz\n"
echo "Orden de la matriz: $ordenMatriz"
InformePorPantalla "Cuadrada:" $esCuadrada
InformePorPantalla "Identidad:" $esIdentidad
InformePorPantalla "Nula:" $esNula
InformePorPantalla "Fila:" $esFila
InformePorPantalla "Columna:" $esColumna
#------------------------------------------ FIN SCRIPT ---------------------------------------------------#