#------------------ENCABEZADO---------------------------#
#														#
#	Nombre del script: Ejercicio5.ps1					#
#	APL Nro: 1											#
# 	Ejercicio Numero 5									#
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
        Este script realiza conexiones a la API relacion con Pokemon https://pokeapi.co/.
        Se permitira obtener informacion a los pokemons por id o nombre. Una vez obtenida
        la informacion se mostrara por pantalla la informacion basica del pokemon o de los pokemons.
    
    .DESCRIPTION

    Ejecuciones validas:
	    Este script admite 2 parametros para consultar la API
        Se puede consultar por id o nombre.
	
	Cuando recibe un parametro:"
			./Ejercicio5.ps1         -id         Id del pokemon
			./Ejercicio5.ps1         -nombre     Nombre del pokemon

	Cuando recibe 2 parametros:"
			./Ejercicio5.ps1   -id 1,2 -nombre pikachu,otro
			./Ejercicio5.ps1   -nombre pikachu,otro -id 1,2

    Ejemplos de PRUEBA:
  		./Ejercicio5.ps1 -id 1 -nombre pikachu
 		Get-Help ./Ejercicio5.ps1
   
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory = $false)]
    [ValidateNotNullOrEmpty()]
    [ValidateRange(1, [int]::MaxValue)]
    [int[]]
    $id,
    [Parameter(Mandatory = $false)]
    [ValidateNotNullOrEmpty()]
    [ValidatePattern('[a-z]')]
    [String[]]
    $nombre
)
function global:PokemonExist{
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [int]$id
    )
    $result = $global:ListaPokemon | where-object ID -eq $id
    if ($result.Count -gt 0) {
        return $true
    }
    return $false
}
class Pokemon
{ 
  $ID
  $Name
  $Height
  $Weight
  $Types
 
  #Constructor
  Pokemon($ID,$Name,$Height,$Weight,$Types)
  {
    $this.ID=$ID
    $this.Name=$Name
    $this.Height=$Height
    $this.Weight=$Weight
    $this.Types=$Types
  }

}

$ConsultaPokemon = @()
$busquedaCACHE = @()
$global:ListaPokemon = @()
if($id.Count -eq 0 -and $nombre.Count -eq 0){ Write-Error "Error en pasar la informacion por los parametros"; exit 20}
if($id.Count -ne 0){
    $id | ForEach-Object { if (( $_ -eq 0)){ Write-Error "Error! Se ingreso un ID nulo."; exit 10}}
    $ConsultaPokemon += $id
}
if($nombre.Count -ne 0){
    $nombre | ForEach-Object { if ( [string]::IsNullOrEmpty($_.Trim()) -eq $True ) { Write-Error "Error! Se ingreso un nombre de pokemon nulo."}}
    $ConsultaPokemon += $nombre
}
$ArchivoCACHE="./Pokemon.Cache"
$ENDPOINT = "https://pokeapi.co/api/v2/pokemon/"

#Busco los pokemones que hay en cache
if( Test-Path $ArchivoCACHE )
{
    $busquedaCACHE = Get-Content -Path $ArchivoCACHE | Out-String | ConvertFrom-Json | Select-Object
}
else {
    $busquedaCACHE = @()
}
$ConsultaPokemon | ForEach-Object {
        $consultar = $_
        #Consulta a la api
        $URL = "${ENDPOINT}${consultar}"
        try {
            #Consultar si existe el pokemon en cache
            if( $consultar -is [Int] ){
                $salida = $busquedaCACHE | where-object id -eq $consultar
            }else{
                $salida = $busquedaCACHE | where-object name -eq $consultar
            }    
            if($salida){
                $IdPokemon = $salida | Select-Object ID | Sort-Object -Unique
                $Existe = global:PokemonExist $IdPokemon.ID
                if(!$Existe){$global:ListaPokemon += $salida}
            }
            else {
                $Respuesta = Invoke-RestMethod $URL
                #arma los estilos del pokemon consultado
                $tiposPokemon = [system.String]::Join(",", $Respuesta.types.type.name)
                #Creo el objeto Pokemon
                $PokemonObj = [Pokemon]::new($Respuesta.id,$Respuesta.name,$Respuesta.height,$Respuesta.weight,$tiposPokemon)
                
                if($PokemonObj.ID){
                    #Cuando no este en cache se debe agregar
                    [array]$busquedaCACHE += $PokemonObj
                    #Agrego a la lista que se va a mostrar
                    $global:ListaPokemon += $PokemonObj
                }
                else{
                    Write-Error "No se encontro informacion del Pokemon Solicitado. ID o nombre invalido"
                    exit 5
                }
            }
        } catch {
            switch ($_.Exception.Response.StatusCode.value__ ) {
                '404' { 
                    Write-Error "StatusCode: 404" 
                    Write-Error "StatusDescription: Not Found" 
                    Write-Error "ErrorDescription: El servidor no pudo encontrar el contenido solicitado"
                    exit 1
                 }
                 '400' { 
                    Write-Error "StatusCode: 400" 
                    Write-Error "StatusDescription: Bad Request" 
                    Write-Error "ErrorDescription: El servidor no pudo interpretar la solicitud"
                    exit 2
                 }
                 '500' { 
                    Write-Error "StatusCode: 500" 
                    Write-Error "StatusDescription: Internal Server Error" 
                    Write-Error "ErrorDescription: Error en el Servidor"
                    exit 3
                 }
                Default {
                    Write-Host "ErrorDescription: Error al consultar la api de Pokemon. Intente mas tarde"
                    exit 4
                }
            }
        }
}

$salida2 = $busquedaCACHE | ConvertTo-Json -AsArray | Set-Content -Path $ArchivoCACHE
$global:ListaPokemon