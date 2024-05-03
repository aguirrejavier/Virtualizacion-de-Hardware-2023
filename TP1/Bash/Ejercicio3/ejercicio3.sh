#!/bin/bash

#################  ENCABEZADO  ##################
#												#
#	Nombre del script: ejercicio3.sh			#
#	Numero de APL: 1							#
# 	Numero de ejercicio: 3						#
#												#
#	Integrantes:								#
#  		Rodriguez, Cesar Daniel		39166725	#
# 		Aguirre, Carlos             38700231	#
#  		Sanchez, Kevin              41173649 	#
#		Baranda, Leonardo 			36875068	#
#												#
#################################################

#Funcion de Ayuda
ayuda() {
    echo "****************************************************************************************************"
    echo "   Este Script analiza los archivos de texto en un directorio, realizando un informe con:           "
    echo "   - Palabras con mas ocurrencias.                "
    echo "   - Palabras con menos ocurrencia.               "
    echo "   - Cantidad total de palabras.                  "
    echo "   - Promedio de palabras por archivo. (cantidad de palabras sobre total de archivos).              "
    echo "   - Palabras más largas, en cantidad de caracteres. Pueden ser mas de una palabra                    "
    echo "   Para todos los archivos del directorio, se debe tener permisos de lectura en cada uno.            "
    echo "   El informe se mostrará por consola          "
    echo "Parámetros (sin orden):                           "
    echo "                                                  "
    echo "-d / --directorio \"directorio\": Ruta del archivo a procesar. Puede ser ruta relativa o absoluta.  "
    echo "                                                  "
    echo "-x / --extension \"extension\": (opcional). Indica la extension de los archivos a analizar.      "
    echo "                                                  "
    echo "-h | --help | : consultar la ayuda                "
    echo "                                                  "
    echo "                                                  "
    echo "  Ejemplo de ejecucion:  "
    echo "  ./ejercicio3.sh -d \"/home/archivos\" -x txt    "
    echo "  ./ejercicio3.sh --directorio \"/home/archivos\" --extension txt    "
    echo "  ./ejercicio3.sh --directorio \"/home/archivos\"                    "
    echo "                                                  "
    echo "****************************************************************************************************"
}

while [[ $# > 0 ]]
do
  case "$1" in
  -d|--directorio)
    parametroDir=$1
    directorio=$2
    shift
    shift
    ;;    
  -x|--extension)
    parametroExt=$1
    extension=$2
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

  if [ -z "$parametroDir" ]
  then
    echo "El parametro \"-d\" | \"--directorio\" es obligatorio."
    exit 1
  fi

  if [ ! -d "$directorio" ]
  then
      echo "El parámetro "$directorio" no es un directorio"
      exit 1;
  fi;

  if [ -n "$parametroExt" ]
  then
    if [ -z "$extension" ]
    then
      echo "Si desea generar un informe con los archivos de una extensión especifica, debe indicarla luego del parámetro -x | --extension. (ver ayuda -h | --help)"
      exit 1
    fi;
  fi;

  for archivo in "$directorio"/*;
  do
    if [ ! -r "$archivo" ]
    then
      echo "El archivo "$archivo" no tiene permisos de lectura"
      exit 1;
    fi;
  done;
}

generarInforme () {
  
  if [ -n "$2" ];
  then
    ext=("."$2)
  fi;
  
  archivos=$(find "$1" -type f -name "*$ext")
  cantArchivos=$(find "$1" -type f -name "*$ext" | wc -l)
  
  if [ $cantArchivos -eq 0 ]
  then
    echo "No existe ningun archivo con la extension "$extension
    exit 1;
  fi;

  #extraigo todas las palabras de todos los archivos correspondientes
  palabras="$( echo "$archivos" | awk ' {
      while ((getline line<$0) > 0 ){
          print line;
      }
  }')"

  #Proceso todas las palabras
  echo "$palabras" | awk 'BEGIN {
    cantidadPalabras = 0
    longitudMaxima = 0
  }
  {   
    for (i = 1; i <= NF; i++) {
      if ( $i ~ /^[a-zA-Z0-9áéíóúÁÉÍÓÚñÑ]+$/ ) {
        cantidadPalabras ++
        if(longitudMaxima < length($i)) {
          longitudMaxima = length($i)
        } 
        palabras[$i]++
      }
    }
  } 
  END {
    
    nroMaximoOcurrencias = 0
    nroMinimoOcurrencias = cantidadPalabras
    indexMax = 1
    indexMin = 1
    indexLarga = 1
        
    for (palabra in palabras) {
      if ( palabras[palabra] < nroMinimoOcurrencias )
        nroMinimoOcurrencias = palabras[palabra]
      if ( palabras[palabra] > nroMaximoOcurrencias )
        nroMaximoOcurrencias = palabras[palabra]
      if ( longitudMaxima == length(palabra) ) {
        palabrasMaslargas[indexLarga] = palabra
        indexLarga++
      }
    }

    for ( palabra in palabras ) {
      if ( palabras[palabra] == nroMaximoOcurrencias )
        palabrasMasOcurrentes[indexMax++] = palabra
      if ( palabras[palabra] == nroMinimoOcurrencias )
        palabrasMenosOcurrentes[indexMin++] = palabra
    }

    print " - Cantidad total de palabras:", cantidadPalabras 
    
    print " - Palabras mas ocurrentes con " nroMaximoOcurrencias " ocurrencias:"
    for (i=1; i <= length(palabrasMasOcurrentes); i++) {
      printf "%s%s", palabrasMasOcurrentes[i], i == length(palabrasMasOcurrentes) ? "" : ", "
    }
    print ""
    print " - Palabras menos ocurrentes con " nroMinimoOcurrencias " ocurrencias:"
    for (i=1; i <= length(palabrasMenosOcurrentes); i++) {
      printf "%s%s", palabrasMenosOcurrentes[i], i == length(palabrasMenosOcurrentes) ? "" : ", "
    }
    print ""
    print " - Promedio de palabras por archivo: " (cantidadPalabras / '$cantArchivos')
    
    print " - Palabras mas largas con " longitudMaxima " caracteres "
    for ( i=1; i<= length(palabrasMaslargas); i++ ) {
      printf "%s%s", palabrasMaslargas[i], i == length(palabrasMaslargas) ? "" : ", "
    }
    print ""

  }'
}

validarParametros

generarInforme "$directorio" $extension