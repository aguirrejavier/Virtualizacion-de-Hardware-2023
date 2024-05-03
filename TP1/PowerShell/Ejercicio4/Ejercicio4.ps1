#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Ejercicio4.ps1					#
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

<#
    .SYNOPSIS
        Este script realiza un monitoreo en segundo plano de un directorio.
		Lo que va a estar monitoreando del directorio son las creaciones
		de nuevos archivos o modificaciones. Luego de cada cambio se realiza un back up
		del directorio y lo guarda en otro directorio. Tambien debe crear un log
    	con los detalles de los cambios detectados.
    
    .DESCRIPTION

    Ejecuciones validas:
	    Este script admite 1 o 4 parametros
	
	Cuando recibe un parametro:"
			./Ejercicio4.ps1         -directorio          Ruta del directorio a monitorear
			./Ejercicio4.ps1         -salida              Ruta del directorio donde se guardar los backUp

	Cuando recibe 4 parametros:"
			./Ejercicio4.ps1   -directorio directorioAmonitorear -salida directorioBackups
			./Ejercicio4.ps1   -salida directorioBackups -directorio directorioAmonitorear

    Ejemplos de PRUEBA:
  		./Ejercicio4.ps1 -directorio ./Prueba/A_Monitorear/ -salida ./Prueba/BackUp/
 		Get-Help ./Ejercicio4.ps1"
   
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [ValidateScript({Test-Path $_})]
    [string]
    $directorio,
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [ValidateScript({Test-Path $_})]
    [string]
    $salida
)
#------------------------------------- FUNCIONES --------------------------------------------------#
function global:LoggerInfo{
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$pathModificado,
        [Parameter(Mandatory = $true)]
        [string]$detalle
    )
    $dateLogger = Get-Date -format "dd-MM-yyyy HH:mm:ss"
    $detalleUpper = $detalle.ToUpper()
    $mensajeLog = "${dateLogger} | DIRECTORIO_MONITOREADO: ${global:PATH_DIRECTORIO_ABSOLUTO} | Detalle: accion detectada ${detalleUpper} | PATH_ARCHIVO: ${pathModificado}"
    Out-File -FilePath "./Ejercicio4Log.Info" -Append -InputObject $mensajeLog
}

function global:CreateBackUp{
    try {
        $dateCompress = Get-Date -format "yyyyMMdd-HHmmss"
        Compress-Archive -Path "${global:PATH_DIRECTORIO}/*" -DestinationPath "${global:PATH_SALIDA}/${dateCompress}.zip"
    }
    catch {
        # All errors that are not of type UnauthorizedAccessException or ItemNotFoundException are capture here
        Set-Content -Path "${global:PATH_DIRECTORIO_ABSOLUTO}/UnauthorizedAccessExceptionExport.log"  -Value $_.Exception.Message
    }
}
#---------------------------------- FIN FUNCIONES --------------------------------------------------#

#---------------------------------- CUERPO SCRIPT --------------------------------------------------#
$global:PATH_DIRECTORIO = "$directorio"
$global:PATH_DIRECTORIO_ABSOLUTO = Resolve-Path $global:PATH_DIRECTORIO
$global:PATH_SALIDA = "$salida"
$global:PATH_SALIDA_ABSOLUTO = Resolve-Path $global:PATH_SALIDA
$global:lastTimestamp = "empty"

$eventCreatedExists = Get-EventSubscriber -SourceIdentifier $global:PATH_DIRECTORIO_ABSOLUTO"monitorearCreated" -ErrorAction SilentlyContinue
$eventChangedExists = Get-EventSubscriber -SourceIdentifier $global:PATH_DIRECTORIO_ABSOLUTO"monitorearChanged" -ErrorAction SilentlyContinue

if ($eventCreatedExists -or $eventChangedExists -or $eventDeletedExists) {
    Write-Host "Error. Ya existe un monitoreo en el directorio ingresado!" -ForegroundColor Red
    Write-Host "Ejecute 'Get-Help ./Ejercicio4.ps1' para mostrar la ayuda." -ForegroundColor Green
    Exit
}
    
$global:watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = Resolve-Path $global:PATH_DIRECTORIO_ABSOLUTO
$watcher.IncludeSubdirectories = $true
$watcher.EnableRaisingEvents = $true  

$action = {
        $detalles = $Event.SourceEventArgs
        $archivo = $detalles.FullPath
        $changeType = $detalles.ChangeType     
        $currTimestamp = $Event.TimeGenerated.toString()
        if ($global:lastTimestamp -ne $currTimestamp) {
            global:LoggerInfo $archivo $changeType   
            global:CreateBackUp 
        }
        $global:lastTimestamp = $currTimestamp
        
}

$null = Register-ObjectEvent $watcher "Created"  -SourceIdentifier $global:PATH_DIRECTORIO_ABSOLUTO"monitorearCreated" -Action $action
$null = Register-ObjectEvent $watcher "Changed"  -SourceIdentifier $global:PATH_DIRECTORIO_ABSOLUTO"monitorearChanged" -Action $action

#---------------------------------- FIN SCRIPT --------------------------------------------------#
