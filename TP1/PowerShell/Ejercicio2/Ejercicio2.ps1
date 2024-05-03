#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Ejercicio2.ps1					#
#	APL Nro: 1											#
# 	Ejercicio Numero 2									#
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Sanchez, Kevin				41173649			#
#		Baranda, Leonardo			36875068			#

<#
    .SYNOPSIS
        Script que analiza una matriz e informa por pantalla:
        • Orden de la matriz.
        • Si es cuadrada.
        • Si es identidad.
        • Si es nula.
        • Si es fila.
        • Si es columna.
    .DESCRIPTION
        -entrada: ruta al archivo que contiene la matriz
        -separador: caracter que separa los valores de la matriz (Opcional,"," es el caracter por defecto)
    .EXAMPLE
        • Ejercicio2.ps1 -entrada m1 -separador ','
        • Ejercicio2.ps1 -entrada m2
        • Ejercicio2.ps1 -entrada m3 -separador '_'
        • Ejercicio2.ps1 -entrada m4 -separador ' '
        • Get-Help Ejercicio2.ps1
#>

[CmdletBinding(PositionalBinding=$false)]
Param(
    [Parameter(Mandatory = $false)]
    [ValidatePattern('\D')]
    [ValidatePattern('[^-]')]
    [char]$separador = ",",
    [Parameter(Mandatory = $true)]
	[ValidateScript({
        if (Test-Path "$_") {
            if ([String]::IsNullOrWhiteSpace((Get-content "$_"))) {
                throw "FICHERO VACIO"
            }
        } else {
            throw "FICHERO NO EXISTE"
        }
        $flag=0
        foreach ($fila in Get-Content $_) {
            $aux = ($fila -replace '-',"0")
            $temp_string = ($aux -replace '\D',"")
            if ($aux -match '\D') {
                $pattern = '\D'
                $match = $aux | Select-String -Pattern $pattern
                $position = $match.Matches.Index
                $separador = $aux[$position]
                $arrColum=@($aux.Split($separador))
                foreach ($valor in $arrColum) {
                    if ($valor -match '\D') {
                        throw "MATRIZ INVALIDA; Valores no numericos en la matriz"
                    }
                }
            }
            if($flag -eq 0) {
                $countAnt = $fila.Length - $temp_string.Length
                $flag = 1
            }
            $count = $aux.Length - $temp_string.Length
            if($countAnt -ne $count) {
                throw "MATRIZ INVALIDA; Cantidad de columnas no constante"
            }
            if ($aux -match '\D$') {
                throw "MATRIZ INVALIDA; Ultimo valor de fila no valido"
            }
            if ($aux -match '\D\D') {
                throw "MATRIZ INVALIDA; Valores vacios"
            }
        }
        return $true
	})]
    [String]$entrada
	)
function informar() {
    param (
        $mensaje,
        $boolean
    )
    if ($boolean.Equals($true)) {
        $mensaje+="Si"
    }else {
        $mensaje+="No"
    }
    Write-Host $mensaje
}
$FILE = Get-Content "$entrada"
$results=@()
$cntFila=0
$esCuadrada=$false
$esIdentidad=$true
$esNula=$true
$esFila=$false
$esColumna=$false
foreach ($fila in $FILE)
{
    $results+=$fila
    $cntColumna = 0
    $arrColum=@($fila.Split($separador))
    foreach ($valorMatriz in $arrColum)
    {
        if ($cntColumna -eq $cntFila) {
            if ($valorMatriz -ne 1) {
                $esIdentidad = $false
            }
        }
        else {
            if ($valorMatriz -ne 0) {
                $esIdentidad = $false
            }
        }
        if ($valorMatriz -ne 0) {
            $esNula=$false
        }
        #Write-Host "[$cntFila,$cntColumna] = $valorMatriz"
        $cntColumna++
    }
    $cntFila++
}
$ordenMatriz = $cntFila*$cntColumna
if ($cntFila -eq $cntColumna) {
    $esCuadrada = $true
}
if ($cntFila -eq 1) {
    $esFila=$true
}
if ($cntColumna -eq 1) {
    $esColumna=$true
}
Write-Host "    Informe de la matriz"
Write-Host "Orden de matriz: $ordenMatriz"
informar "Cuadrada: " $esCuadrada
informar "Identidad:" $esIdentidad
informar "Nula:" $esNula
informar "Fila:" $esFila
informar "Columna:" $esColumna