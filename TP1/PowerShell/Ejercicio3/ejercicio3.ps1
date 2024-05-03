##############################################################
##  Ejercicio 3 del APL 1 - 2C-2023 - Entrega
##  Script: Ejercicio05.ps1
##
##  Integrantes del grupo
##  Aguirre, Carlos, 38700231
##  Baranda, Leonardo, 36875068
##  Rodriguez, Cesar Daniel, 39166725
##  Sanchez, Kevin, 41173649
##############################################################

<#
.SYNOPSIS
    Este script analiza los archivos de texto en un directorio, realizando un informe con:
	- Palabras con mas ocurrencias.                
	- Palabras con menos ocurrencia.               
	- Cantidad total de palabras.                  
	- Promedio de palabras por archivo. (cantidad de palabras sobre total de archivos).     
	- Palabras más largas, en cantidad de caracteres.
.DESCRIPTION
    Dado un directorio pasado por parametro, se hace una lectura de todos los archivos de texto
	que se encuentren en ese directorio. La lectura consiste en leer cada palabra de cada archivo 
	y realizar un informe detallado del total de las palabras, las que son mas y menos ocurrentes,
	el promedio por archivo, y la palabras mas largas de todas.
	Este informe se imprime por consola.
.PARAMETER -directorio
    Indica el directorio correspondiente a analizar, leyendo sus archivos. Todos los archivos incluidos
	en el directorio tienen que ser de formato texto y deben tener permiso de lectura.
.PARAMETER -extension
    Indica la extension de los archivos que se requieren analizar del directorio. Este parámetro es
	opcional. Si no se pasa la extension, se leerán todos los archivos de texto independientemente de
	su extension.
.EXAMPLE
    ./ejercicio3.ps1 -directorio /home/archivos -extension txt
.EXAMPLE
    ./ejercicio3.ps1 -directorio /home/archivos
.OUTPUTS
#>

Param (
	[Parameter(Mandatory=$True)]
  [ValidateNotNullOrEmpty()]
	[ValidateScript({
      if( !( Test-Path $_ ) ) {
        Write-Host -ForegroundColor DarkRed "Error en el argumento del parametro directorio. El directorio $_ no existe o no es valido."
        $False
      }
		$True
    })]
	[string] $directorio,

	[Parameter(Mandatory=$False)]
  [ValidateNotNullOrEmpty()]
	[string] $extension
)

# Obtengo la lista del nombre de todos los archivos del directorio.
function obtenerArchivos () {
  if ( ! $extension ) {
    return Get-ChildItem -Path $directorio -Recurse -File 
  } else {
    return Get-ChildItem -Path $directorio\ -Recurse -File -Include *.$extension
  }
}

#Valido los permisos de lectura sobre los archivos
foreach($archivo in $archivos) {
  $acl = Get-Acl -Path $archivo.FullName
  $permisoLectura = $acl.Access | Where-Object { $_.FileSystemRights -like '*FullControl*' || $_.FileSystemRights -like '*Read*'}
  
  if ($permisoLectura.Count -eq 0) {
    Write-Host -ForegroundColor Red "el archivo "$archivo.Name "no tiene permisos de lectura"
    exit 1
  }
}

function realizarInforme () {
  $palabras =@()
  # Agrego todas las palabras de cada archivo, contando como palabra las cadenas de caracteres separadas por espacios y saltos de linea
  foreach($archivo in $archivos) {
    $palabras += (Get-Content -Path $archivo) -split '\s+' | Where-Object { $_ -match '\b[a-zA-Z0-9ÁÉÍÓÚáéíóúÑñ]+\b' }  
  }
  
  # Obtengo la cantidad total de palabras 
  $cantidadPalabras = $palabras.Length
  
  $longitudMaxima = 0
  $cantOcurrencias = @{}
  $palabrasMasOcurrentes = @()
  $palabrasMenosOcurrentes = @()
  $palabrasMasLargas = @()
  
  foreach($palabra in $palabras) {
    
    # Obtengo la palabra mas larga
    if ( $longitudMaxima -lt $palabra.Length ) {
      $longitudMaxima = $palabra.Length
    }
  
    if ( !$cantOcurrencias.ContainsKey($palabra) ) {
      $cantOcurrencias[$palabra] = 1
    } else {
      $cantOcurrencias[$palabra]++
    }
  }
  # Obtengo las palabras con mayor y menor ocurrencias
  $cantOcurrenciasMayor = [int]($cantOcurrencias.Values | Measure-Object -Maximum | Select-Object -Property Maximum).Maximum
  $cantOcurrenciasMenor = [int]($cantOcurrencias.Values | Measure-Object -Minimum | Select-Object -Property Minimum).Minimum
  
  foreach ($clave in $cantOcurrencias.Keys) {
    if( $cantOcurrenciasMayor -eq $cantOcurrencias[$clave]) {
      $palabrasMasOcurrentes += $clave
    }
  
    if ( $cantOcurrenciasMenor -eq  $cantOcurrencias[$clave]) {
      $palabrasMenosOcurrentes += $clave
    }

    if ( $longitudMaxima -eq $clave.Length ) {
      $palabrasMasLargas += $clave
    }
  }

  $palabrasMasOcurrentes = $palabrasMasOcurrentes -join ', '
  $palabrasMenosOcurrentes = $palabrasMenosOcurrentes -join ', '
  $palabrasMasLargas = $palabrasMasLargas -join ', '

  Write-Host -ForegroundColor Green " - Cantidad total de palabras:" $cantidadPalabras
  Write-Host -ForegroundColor Green " - Palabras mas ocurrentes con $cantOcurrenciasMayor ocurrencias:" $palabrasMasOcurrentes
  Write-Host -ForegroundColor Green " - Palabras menos ocurrentes con $cantOcurrenciasMenor ocurrencias:" $palabrasMenosOcurrentes
  
  $promedio = $cantidadPalabras / $archivos.Length
  Write-Host -ForegroundColor Green " - Promedio de palabras por archivo:" $promedio
  Write-Host -ForegroundColor Green " - Palabras más largas:"$palabrasMasLargas
}

$archivos = obtenerArchivos

realizarInforme $archivos

