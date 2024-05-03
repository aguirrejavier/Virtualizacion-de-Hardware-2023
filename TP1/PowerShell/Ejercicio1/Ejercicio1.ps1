<#
    .SYNOPSIS
        Este Script analiza los datos de los motores de una 
        petrolera para obtener estadisticas generales de uso y temperatura.   
    .DESCRIPTION
        -entrada: (obligatorio) ruta al archivo .csv que contiene los datos a analizar
        -archivo: (opcional) indica que la informacion analizada se exportara en un archivo
        -salida: (obligatorio si se utiliza -archivo) indica el archivo a exportar los datos
    .EXAMPLE
        • Ejercicio1.ps1 -entrada arch1.csv 
        • Ejercicio1.ps1 -entrada arch1.csv -archivo -salida arch2.txt
        • Get-Help Ejercicio1.ps1
#>


#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Ejercicio1.ps1					#
#	APL Nro: 1											#
# 	Ejercicio Numero 1									#
#														#
#	Integrantes:										#
#  		Rodriguez, Cesar Daniel		39166725			#
# 		Aguirre, Carlos				38700231			#
#  		Sanchez, Kevin				41173649			#
#		Baranda, Leonardo			36875068			#
#                                                       #
#-------------------------------------------------------#


[CmdletBinding(DefaultParameterSetName='Entrada')]
Param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [ValidateScript({ 
        if(-Not ($_ | Test-Path)){
        Throw "El directorio $_ no existe o no es valido." 
    }
    else {
        $True
    } 
    })]
    [string]
    $entrada,

    [Parameter(ParameterSetName='Salida')]
    [Switch]
    $archivo,

    [Parameter(Mandatory = $true, ParameterSetName='Salida')]
    [ValidateNotNullOrEmpty()]
    [ValidateScript({ if(-Not ($_ | Test-Path)){
        New-Item -Path $_ 
    }
    else {
        $True
    } 
    })]
    [string]
    $salida
)

function GenerarReporte {
    param (
        [int]$MaxMotor1 = 0,
        [int]$MaxMotor2 = 0,
        [float]$SumTemp = 0,
        [float]$PromTemp = 0
    )
    
    $CSV = Import-Csv  $entrada 



    $CantReg = (Get-Content $entrada -TotalCount 1).Split(',').Count - 1
   
foreach($LINE in $CSV){


    if([int]$LINE.UsoMotor1 -gt $MaxMotor1){
        $MaxMotor1 = $LINE.UsoMotor1
    }

    if([int]$LINE.UsoMotor2 -gt $MaxMotor2){
        $MaxMotor2 = $LINE.UsoMotor2
    }

    $SumTemp += $LINE.temperatura
    
}

    $PromTemp = $SumTemp/$CantReg
    
    if($salida){
        Set-Content -Value "Motor1=$MaxMotor1" -Path "$salida"
        Add-Content -Value "Motor2=$MaxMotor2" -Path "$salida"
        Add-Content -Value "Temperatura=$('{0:N2}' -f $PromTemp)" -Path "$salida"
    }
    else{
        "Uso maximo motor 1: $MaxMotor1"
        "Uso maximo motor 2: $MaxMotor2"
        "Promedio de temperatura: $('{0:N2}' -f $PromTemp)"
    }

}

GenerarReporte