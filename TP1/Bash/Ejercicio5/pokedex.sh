#!/bin/bash

#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: pokedex.sh   					#
#	APL Nro: 1											#
# 	Ejercicio Numero 5									#
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Sanchez, Kevin				41173649			#
#		Baranda, Leonardo			36875068			#

################ INSTALAR ###################
    # CURL: sudo apt-get install curl
    # JQ: sudo apt-get install jq
#############################################
#------------------------------------------ FUNCIONES -----------------------------------------------#
mostrarAyuda() {
    echo -e "Uso del script:
                    ./pokedex.sh -i id -n nombre
            
            -i / --id              Id o ids de los pokemon a buscar.
            -n / --nombre          Nombre o nombre de los pokemon a buscar.


    El orden de ambos parametros y sus argumentos pueden ser intercambiados
    Es posible ejecutar el script solo con un parametro y su argumento

                    Ej: ./pokedex.sh --id id
                    Ej: ./pokedex.sh -n nombre

    El scrip se comunica con la api https://pokeapi.co/ para buscar informacion de los pokemon
    y devuelve sus: ID, Nombre, Height, Weight, Types

    Posibles salidas:
        0       El script termino sin problemas
        1       Comando --help, -h
        2       Error en alguno de los parametros
        3       Argumentos de ID o Nombre vacios
            
    En este script se utilizaron los paquetes JQ y cURL, por favor instalar ejecutando los siguientes comandos en la consola:
                sudo apt-get install curl
                sudo apt-get install jq
            "    
        exit 1
}
function crearCache {
    if ! [ -d "$1" ];
    then
        mkdir "$1"
    fi
}
function descargarPokemon {
    { #try
        infoJson=$(curl -s "https://pokeapi.co/api/v2/pokemon/{$1}" | jq '{id,name,weight,height,types}' 2>/dev/null)
        # --fail --show-error
    } || { #catch
        return 1
    }
        #echo "$infoJson"
        local id=$(echo $infoJson | jq -r '.id')
        local name=$(echo $infoJson | jq -r '.name')
        local height=$(echo $infoJson | jq -r '.height')
        local weight=$(echo $infoJson | jq -r '.weight')
        flag=0
        cantDeTipos=$(echo $infoJson | jq '.types | length')
        strTipos=""
        for ((i=0; i<cantDeTipos; i++)); do
            if [[ $flag -ne 0 ]]; then
                strTipos+=", "
            fi
            flag=1
            strTipos+="$(echo $infoJson | jq -r ".types[$i].type.name")"
        done
        echo -e "Id: $id\nName: $name\nHeight: $height\nWeight: $weight\nTypes: $strTipos" > $cache/$id"_"$name
}
function buscarIdEnCache {
    lines=$(find ./$cache -iname "$1\_*" | wc -l)
}
function buscarNombreEnCache {
    lines=$(find ./$cache -iname "*_$1" | wc -l)
}
function buscarEnCache {
    re='^[0-9]+$'
    if [[ $1 =~ $re ]] ; then
        buscarIdEnCache $1
    else
        buscarNombreEnCache $1
    fi
}
function mostrarPorIdDesdeCache {
    find ./$cache -iname "$1\_*" -exec cat {} \;
}
function mostrarPorNombreDesdeCache {
    find ./$cache -iname "*_$1" -exec cat {} \;
}
function mostrarDesdeCache {
    re='^[0-9]+$'
    if [[ $1 =~ $re ]] ; then
        mostrarPorIdDesdeCache $1
    else
        mostrarPorNombreDesdeCache $1
    fi
}
function buscarYMostrarPokemon {
    IFS=',' read -r -a array <<< "$1,$2"
    for element in "${array[@]}"
    do
        buscarEnCache $element
        #buscarEnCache modifica $lines
        if [ $lines -eq 0 ]; then
            echo "Pokemon: $element no esta en la cache. Descargando.."
            descargarPokemon $element
            if [[ $? -eq 0 ]]; then
                mostrarDesdeCache $element
            else
                echo "Hubo un problema con su peticion, por favor revise el id/nombre: '$element'."
                echo "O consulte el estado del servidor"
            fi
        else
            mostrarDesdeCache $element
        fi
        echo ""
    done
}
validarParametros () {
    if [ -z $paramId ] && [ -z $paramNombre ]; then
        echo "Es necesario por lo menos un parametro. Para mas informacion, ingrese a la ayuda con los parametros -h | --help"
        exit 2
    fi
    if [ -n "$paramId" ]; then
        if [[ -z $id ]]; then
            echo "ID vacio"
            exit 3;
        fi
        local re='^[1-9][0-9]*$'
        if ! [[ $id =~ $re ]]; then
            echo "El parámetro ID debe ser un numero positivo mayor 0"
            exit 2;
        fi;
    fi
    if [ -n "$paramNombre" ]; then
        if [[ -z $nombre ]]; then
            echo "Nombre vacio"
            exit 3;
        fi
        reg='^[^0-9]+$'
        if ! [[ "$nombre" =~ $reg ]]; then
            echo "El parámetro Nombre contiene caracteres no validos"
            exit 2;
        fi;
        nombre=$(echo ${nombre,,})
        echo $nombre
    fi
}
#---------------------------------------- FIN FUNCIONES -----------------------------------------------#
#---------------------------------------- VALIDACIONES ------------------------------------------------#
while [[ $# > 0 ]]
do
    case "$1" in
    -i|--id)
    paramId=$1
    id=$2
    shift
    shift
    ;;    
    -n|--nombre)
    paramNombre=$1
    nombre="$2"
    shift
    shift
    ;;
    -h|--help)
    mostrarAyuda
    ;;
    *)
    echo "Error en el parametro "$1". Para mas informacion, ingrese a la ayuda con los parametros -h | --help"
    exit 2
    ;;
    esac
done
#-------------------------------------- FIN VALIDACIONES ------------------------------------------------#
#----------------------------------------- CUERPO SCRIPT -----------------------------------------------#
validarParametros
cache="Cache"
crearCache $cache
buscarYMostrarPokemon $id $nombre
#----------------------------------------- FIN CUERPO SCRIPT -----------------------------------------------#
