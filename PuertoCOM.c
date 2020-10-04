#include "PanelPuertoCom.h"

/*******************************************************************************
 Fichero:PuertoCom.c
 Pr�posito: Implementa variables y funciones para manejar f�cilmente la conexi�n      
 con un puerto COM.
 
 Para utilizar este m�dulo hay que incluir PuertoCOM.h en la compilaci�n
 adem�s de estar en el proyecto los m�dulos listaPaco.c y listaPaco.h

 Para abrir el puerto hay que ejecutar abre_puerto_automaticamente.
 Esta funci�n abre el puerto COM que le devuelva la identificaci�n
 correcta. Si no abre el puerto devuelve -1 (SIN_PUERTO).
 
 TO DO
 
 1-Mejor gesti�n de errores
 2-Revisar el error en recepci�n de cadena demasiado larga
 	limpiar la cola de entrada o leer los datos y limpiar la cola �?
 
 
*********************************************************************************/
/*
	Copyright � 2019 Patricio Coronado
	
	This file is part of  Base SPM  proyect

    Base SPM is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Base SPM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CMM.  If not, see <http://www.gnu.org/licenses/>
*/
/********************************************************************************
 	ficheros include                                                          
*********************************************************************************/
#include "BaseSPM.h"
#include <userint.h>
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include "PanelPuertoCOM.h"
#include "ErroresRS232.h"
#include <rs232.h>
#include "locatecom.h"
#include "PuertoCOM.h" 
/********************************************************************************
 	constantes
*********************************************************************************/
#define  ERROR_DATOS_EXCESIVOS -9// C�digo no utilizado en la libreria RS-232
#define MAX_ERR_TRANSMISION 3
#define MASTER 0
#define USUARIO 1
#define TECLA_ENTER 1280
#define SI 1
#define NO 0
#define MENSAJE_ABOUT_BASE_SPM "\n\
-----------------------------------------------------------------------\n\
Copyright � 2020 Patricio Coronado\n\n\
This file is part of PCOM proyect\n\n\
PuertoCOM.c is free software: you  can redistribute  it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version  3 of  the License, or\n\
(at your option) any later version.\n\
PuertoCOM.c  is distributed  in the  hopethat it will be\n\
useful,but WITHOUT ANY WARRANTY;without even the implied\n\
 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n\
PURPOSE. See the GNU General Public  License for  more details.\n\
You  should have received a copy  of the GNU General Public License\n\
along with BaseSPM.c If not, see <http://www.gnu.org/licenses/>\n\
Library for reading system ports COM: https://forums.ni.com  (msaxo)\n\n\
Support:https://github.com/PatricioCoronado/PuertoCOM\n\
----------------------------------------------------------------------------\n"

#define MENSAJE_AYUDA_FICHERO"\n\
----------------------------------------------------------------------------\n\
                  �Ejecuta la aplicaci�n por primera vez?                   \n\
----------------------------------------------------------------------------\n\
Cuando el programa se ejecuta, busca un fichero llamado BaseSPM.dat\n\
en el disco del sistema. En ese fichero est�n los par�metros del puerto\n\
COM que tiene que abrir para conectar con la base.\n\
\n\
Pero LA PRIMERA VEZ QUE SE EJECUTA la aplicaci�n el fichero no existe,\n\
por lo que este panel se ha programado para crear ese fichero.\n\
\n\
Busca el puerto en el que est� la base, anotalo en [puerto COM].\n\
El resto de par�metros son los que se muestran por defecto.\n\
Pulsa [crear fichero] y si tiene �xito pulsa [aplicar fichero]\n\
\n\
�SI TIENE UN PUERTO ABIERTO Y LA BASE CONECTADA NO NECESITA SEGUIR!\n\
----------------------------------------------------------------------------\n"
/********************************************************************************
 	variables privadas
*********************************************************************************/
//Handlers
int MensajesHandle=0;
int ConfigHandle=0;
int	EstatusHandle=0;
int FicheroHandle=0;
//  Lista de puertos COM
int listaPortCOM[255];
int nPortCom;
// 
int short ModoMasterUsuario=USUARIO; //Para tener funciones extendidas MASTER
int ModoPanelConfiguracion;
int short FlagDatosRecibidos=NO;//Se pone a SI cuando se produce recepci�n autom�tica
static int FlagPanelConfigRecienAbierto=1;// Para reconfigurar o no el panel cada vez que se abra  
// Variables privadas del puerto serie
//-PARAMETROS DEL PUERTO QUE DEBEN COINCIDIR CON LOS ESTABLECIDOS EN EL PANEL--
static int PuertoCOM=-1; //PUERTO UTILIZADO
static int VelocidadPuerto;
static int Paridad;
static int BitsDatos=8;
static int BitsDeStop=2;
static int ColaDeEntrada=512;  		// Establece la long.de la cola de recepci�n
static int ColaDeSalida=512; 		// Establece la long.de la cola de transmisi�n
static int ModoX=0;
static int ModoCTS=0;
static int TerminadorTransmision=1; // Selecci�n del terminador de transmisi�n 
static int TerminadorRecepcion=1; // Seleccion del terminador de recepci�n 
float Timeout=1.0;
float Retardo=0.1;//Retardo tras enviar un comando esperando una respuesta
int ErrorDeTransmision=0;//Contador de errores de transmisi�n
//********************************************************************************
static int ErrorRS232;      //C�digo de error del puerto serie 
//static int FlagPuertoAbierto=0;  // Flag que indica si hay alg�n puerto abierto  
char NombrePuertoCOM[STRCORTO];	// Nombre l�gico del puerto COM1, COM2, etc.
char ComMsg[STRCORTO];
char Msg1[STRCORTO];   // Cadenas de caracteres auxiliares
char IDNinstrumento[STRMEDIO];//Cadena en la que se lee el identificador enviado por el sistema
enum enumColor {Rojo,Azul,Verde};
//********************************************************************************
// Funciones privadas
int abrir_puerto(int,const char[],int,int,int,int,int,int);//Unico sitio donde se abre el puerto realmente
int cerrar_puerto(int);//Unico sitio donde se cierra el puerto realmente
int carga_panel_de_configuracion(void);//Pone en memoia el panel de configuraci�n
int carga_panel_de_mensajes(void);//Pone en memoia el panel de mensajes
int hacer_lista_de_puertos_del_sistema(void); 
int identifica_al_sistema(void);//Lee el *IDN del sistema conectado en IDNinstrumento
int test_puerto_abierto(void);// Devuelve 0 si el puerto est�abierto y -3 si est� cerrado
void cambia_de_modo_controles_de_configuracion(void);            
void cambia_de_modo_controles_de_mensajes(void);
void led_color(enum enumColor);
void salva_parametros(void);
void descarga_paneles_del_puerto_com(void);//Qita de memoria todos los paneles del puerto
void display_error_RS232(void);
void establece_parametros_com_en_panel(void);
void lee_parametros_com_del_panel (void);
void muestra_mensaje_status(int);
void CVICALLBACK terminador_detectado_en_buffer_de_entrada (int portNo,int eventMask,void *callbackData); 
void actualiza_controles(void);
/********************************************************************************
 	variables p�blicas
*********************************************************************************/
// Cadenas p�blicas de comunicaci�n con el programa principal
char CadenaComando[STRMEDIO];   // Aqu� se pone el comando a enviar al instrumento
char CadenaRespuesta[STRLARGO]; // Aqu� esta la respuesta del instrumento
int FlagPuertoAbierto;  // Flag que indica si hay alg�n puerto abierto  
//==============================================================================       
/****************************************************************************************
VARIABLES Y FUNCIONES PARA GUARDAR Y RECUPERAR DE UN FICHERO LOS DATOS DEL PUERTO SERIE 
*****************************************************************************************/
// Estructura para guardar los datos de un puerto COM
struct strPuertoCOM
{
	char IDNinstrumento[STRMEDIO];
	char NombrePuertoCOM[STRCORTO];
	int  PuertoCOM;
	int  VelocidadPuerto;
	int  Paridad;
	int  BitsDatos;
	int  BitsDeStop;
	int  ColaDeEntrada;
	int  ColaDeSalida;
	int	 ModoX;
	int	 ModoCTS;
	float Timeout;
	int TerminadorTransmision; 
	int TerminadorRecepcion;
	float Retardo;
};
//Estructura global en el m�dulo con los datos del puerto que hay en fichero
struct strPuertoCOM filePuertoCOM;
//Funciones privadas en el m�dulo para manejar el fichero con los datos del puerto
int guarda_puerto_com_en_fichero(void);//Salva la configuraci�n actual en un fichero
int lee_puerto_com_de_fichero(void);
int abre_puerto_desde_fichero(void);//Funcion principal
//
/************************************************************************************
	ENTRADA PRINCIPAL PARA ABRIR UN PUERTO COM CON EL SISTEMA CONECTADO
	------------------------------------------------------------------------------
	BUSCA EL SISTEMA ENTRE LOS PUERTOS DEL PC Y SI LO ENCUENTRA ABRE EL PUERTO 
	SERIE Y CONFIGURA EL PROTOCOLO Y TIMEOUT.
	Intenta abrir el puerto a partir de los datos de un fichero. Si fracasa
	intenta abrir todos los puertos de 0 a 255.Con la configuraci�n 
	del panel de configuraci�n. Si puede abrir alguno, le envia
	un comando de identificaci�n. Y si la cadena devuelta es la esperada, 
	IdentificacionDelInstrumento[],termina de configurar el puerto.
	DEVUELVE: El n�mero del puerto COM abierto.Si no consigue abrir 
	un puerto devuelve -1 (SIN_PUERTO) y PuertoCOM=-1
*************************************************************************************/
int pcom_abre_puerto_serie(void) 
{
	int Indice;
	ssize_t LongMensaje;
	char CadenaComando[STRLARGO];
	//
	//Para conocer las variables del puerto...
	if(!MensajesHandle) carga_panel_de_mensajes();      
	if(!ConfigHandle) carga_panel_de_configuracion();		
	if(PuertoCOM !=-1)
	{
		cerrar_puerto(PuertoCOM);//Por si acaso lo cierra
		FlagPuertoAbierto=0;
	}
	//Intenta abrir el puerto desde un fichero
	if (abre_puerto_desde_fichero()!=SIN_PUERTO)
	{   //Si abre pide la versi�n del sistema sale
		SetCtrlAttribute (ConfigHandle,CONFIG_LEDCONFIG, ATTR_ON_COLOR, VAL_GREEN);//Para saber que se ha abierto desde fichero
		return PuertoCOM;//Si tiene �xito sale con el puerto abierto
	}
	// Si no puede utilizar el fichero para abrir el puerto..
	SetCtrlAttribute (ConfigHandle,CONFIG_LEDCONFIG, ATTR_ON_COLOR, VAL_RED);//Para saber que se ha buscado el puerto
	FlagPuertoAbierto = 0;  // inicializa el flag a "puerto si abrir"
    PuertoCOM=-1;
	lee_parametros_com_del_panel (); // Lee los par�metros de configuraci�n del puerto
	//DisableBreakOnLibraryErrors (); // Funci�n obsoleta 					
	LocateCom("",listaPortCOM,255);//Localiza los puertos COM del sistema
	//Muestra el panel de configuraci�n para que el usuario busque el puerto manualmente
	//pcom_muestra_el_panel_de_configuracion(PANEL_MODO_HIJO,100,300);  
	return -1;
}
/************************************************************************************
 		FUNCIONES PARA GUARDAR Y LEER UN FICHERO CON LOS DATOS DEL PUERTO
 *************************************************************************************/
/************************************************************************************
 GUARDA EN EL FICHERO "configuracion.com" LOS DATOS DEL PUERTO QUE SE HA ABIERTO
 Y HA FUNCIONADO
 *************************************************************************************/
int guarda_puerto_com_en_fichero(void)
{
	FILE *ptrFichero2;
	char* DirAppData;//Path de %appdata%
	char Path[128];//Path del fichero
	DirAppData = getenv("APPDATA"); //Si falla retorna NULL	
	if(DirAppData==NULL) return -1;//Si no est� el directorio sale con error
	sprintf(Path,"%s%s",DirAppData,"\\BaseSPM.dat");		
	ptrFichero2=fopen(Path,"wb");// Crea el fichero
	if (!ptrFichero2) return -1;//Si no habre sale con error
	//Si abre el fichero rellena la estructura con los datos del fichero
    strcpy(filePuertoCOM.IDNinstrumento,IDNinstrumento);
	strcpy(filePuertoCOM.NombrePuertoCOM,NombrePuertoCOM);
	filePuertoCOM.PuertoCOM=PuertoCOM;
	filePuertoCOM.VelocidadPuerto=VelocidadPuerto;
	filePuertoCOM.Paridad=Paridad;
	filePuertoCOM.BitsDatos=BitsDatos;
	filePuertoCOM.BitsDeStop=BitsDeStop;
	filePuertoCOM.ColaDeEntrada=ColaDeEntrada;
	filePuertoCOM.ColaDeSalida=ColaDeSalida;
	filePuertoCOM.ModoX=ModoX;
	filePuertoCOM.ModoCTS=ModoCTS;
	filePuertoCOM.Timeout=Timeout;
	filePuertoCOM.PuertoCOM=PuertoCOM;
	filePuertoCOM.TerminadorTransmision=TerminadorTransmision;
	filePuertoCOM.TerminadorRecepcion=TerminadorRecepcion; 
	filePuertoCOM.Retardo=Retardo;
	// Si abre el fichero sin errores..guarda la estructura en el fichero.
	fwrite(&filePuertoCOM,sizeof(struct strPuertoCOM),1,ptrFichero2);
	fclose(ptrFichero2);//Cierra el fichero2
	return filePuertoCOM.PuertoCOM; 
}
/************************************************************************************
 FUNCION QUE BUSCA EL FICHERO "PuertoCOM.ini" 
 DEVUELVE: Si existe el fichero, lo abre y lee con exito devuelve el n� de puerto COM
 y una estructura llamada filePuertoCOM con los datos del puerto.
 Si no hay fichero � no se abre devuelve -1.
 *************************************************************************************/
int lee_puerto_com_de_fichero(void)
{
	ssize_t Dummy;
	FILE *ptrFichero2;
	char* DirAppData;//Path de %appdata%
	char Path[128];//Path del fichero
	int ErrorFichero;
	//nuevo
	DirAppData = getenv("APPDATA"); //Si falla retorna NULL	
	if(DirAppData==NULL) return -1;//Si no est� el directorio sale con error
	sprintf(Path,"%s%s",DirAppData,"\\BaseSPM.dat");		
	DisableBreakOnLibraryErrors ();//Para evitar errores en debug  
	ErrorFichero=GetFileSize (Path, &Dummy);//Pregunta el tama�o del fichero    
	EnableBreakOnLibraryErrors ();
	if (ErrorFichero>=0)//Si no da error, el fichero existe
	{
		ptrFichero2=fopen(Path,"rb");// Abre el fichero
		if (!ptrFichero2){return -1;} //Si falla la apertura, sale con 0
		// Si abre el fichero sin errores lee los datos del puerto COM
		fread(&filePuertoCOM,sizeof(struct strPuertoCOM),1,ptrFichero2);
		fclose(ptrFichero2);//Cierra el fichero
		return filePuertoCOM.PuertoCOM; //Sale con un n�mero de puerto
	}//if (ErrorFichero>=0)
	return -1;//Si el fichero no existe sale con -1
	//fin nuevo
}
/**************************************************************************************
   			LEE EL FICHERO DE CONFIGURACI�N DEL PUERTO Y LO APLICA
			Abre el puerto que indica el fichero y busca el sistema indicado por:
			char IdentificacionDelInstrumento[] (se rellena en el main)			
			DEVUELVE:el n�mero de puerto si abre el puerto donde est� el sistema o
			 -1 si no hay fichero, no pudo abrir el puerto o el sistema no esta en 
			 el puerto.
***************************************************************************************/
int abre_puerto_desde_fichero(void)
{
	int Indice,FlagPuerto;
	ssize_t LongMensaje;
	int short PCOM;//Para guardar el COM que hab�a cuando se llama a la funci�n
	//Guardamos el PuertoCOM que tenga el sistema y el FlagPuertoAbierto
	PCOM=PuertoCOM;
	FlagPuerto=FlagPuertoAbierto; 
	// Si no encuentra el fichero o no lo puede abrir sale con -1 
	if (lee_puerto_com_de_fichero()==-1) return -1;
	// Si encuentra el fichero y lee una estructura con los datos la utiliza  
    DisableBreakOnLibraryErrors ();// Inhabilita mensajes de error en tiempo de ejecuci�n
	FlagPuertoAbierto=1;//Para enviar y recibir del puerto 
	// Abre el puerto con la configuraci�n leida del fichero
	ErrorRS232 = abrir_puerto (filePuertoCOM.PuertoCOM, filePuertoCOM.NombrePuertoCOM,// ABRE EL PUERTO
		filePuertoCOM.VelocidadPuerto,filePuertoCOM.Paridad,filePuertoCOM.BitsDatos, 
		filePuertoCOM.BitsDeStop, filePuertoCOM.ColaDeEntrada, filePuertoCOM.ColaDeSalida);
	EnableBreakOnLibraryErrors (); // Habilita mensajes de error en tiempo de ejecuci�n
    if (ErrorRS232) //Si da error al abrir el puerto, salimos con fracaso.
	{
		FlagPuertoAbierto=FlagPuerto; //Deja el flag como estaba al entar en la funci�n
		return -1;//   devuelve el control 
	}
    else //(ErrorRS232 == 0) Si no da error al abrir el puerto, lo configura
    {
        //Comprueba que el sistema est� en el puerto
		PuertoCOM = filePuertoCOM.PuertoCOM; //Este es el puerto que hemos abierto
		//Termina de configura el puerto
		SetXMode (filePuertoCOM.PuertoCOM,filePuertoCOM.ModoX);
	    SetCTSMode (filePuertoCOM.PuertoCOM, filePuertoCOM.ModoCTS);
	    SetComTime (filePuertoCOM.PuertoCOM, filePuertoCOM.Timeout);
		TerminadorTransmision=filePuertoCOM.TerminadorTransmision; 
		TerminadorRecepcion=filePuertoCOM.TerminadorRecepcion; 
        FlushInQ (PuertoCOM);//Limpia las  colas de entada y salida
        FlushOutQ (PuertoCOM);                                                                                                            
		Delay(1);//Espera porque al abrir alg�n puerto del microcontrolador este se puede resetar
		identifica_al_sistema();//Identifica el sistema en IDNinstrumento
		identifica_al_sistema();//Identifica el sistema en IDNinstrumento
		if(!strcmp (IDNinstrumento, IdentificacionDelInstrumento))//Compara identificaciones, si son iguales devuelve 0
		{  // Y si es correcta termina de configurar el puerto con los datos del fichero
			// Configura la opci�n protocolo software 
	        SetXMode (filePuertoCOM.PuertoCOM,filePuertoCOM.ModoX);
	        // Configura la opci�n protocolo hardware 
	        SetCTSMode (filePuertoCOM.PuertoCOM, filePuertoCOM.ModoCTS);
	        // Configura la opci�n "Timeout" 
	        SetComTime (filePuertoCOM.PuertoCOM, filePuertoCOM.Timeout);
			// Indica al sistema que puerto se ha abierto
			PuertoCOM=filePuertoCOM.PuertoCOM;
			//Configura los terminadores
			TerminadorTransmision=filePuertoCOM.TerminadorTransmision; 
			TerminadorRecepcion=filePuertoCOM.TerminadorRecepcion; 
	 		//Anota el puerto en el panel de configuraci�n
			SetCtrlVal (ConfigHandle, CONFIG_PUERTOCOM, PuertoCOM);
			//Identifica al instrumento
			strcpy (IDNinstrumento, filePuertoCOM.IDNinstrumento);   
			// Activa la recepci�n autom�tica para ese puerto
			pcom_recepcion_automatica(SI);
			//Actualiza las variables del puerto desde los datos del fichero
			FlagPuertoAbierto=1;
			//Actualiza las variables globales del puerto
			Timeout=filePuertoCOM.Timeout;
			Retardo=filePuertoCOM.Retardo;
			VelocidadPuerto=filePuertoCOM.VelocidadPuerto;
			Paridad=filePuertoCOM.Paridad;
			BitsDatos=filePuertoCOM.BitsDatos;
			BitsDeStop=filePuertoCOM.BitsDeStop;
			ColaDeEntrada=filePuertoCOM.ColaDeEntrada;
			ColaDeSalida=filePuertoCOM.ColaDeSalida;
			ModoCTS=filePuertoCOM.ModoCTS;
			ModoX=filePuertoCOM.ModoX;
			TerminadorRecepcion=filePuertoCOM.TerminadorRecepcion;
			TerminadorTransmision=filePuertoCOM.TerminadorTransmision;
			//Actualiza el panel de configuraci�n
			establece_parametros_com_en_panel();
			//sale devolviendo el puerto COM que ha abierto abierto y el FlagPuertoAbierto a 1
			SetActiveCtrl(ConfigHandle,CONFIG_QUITCONFIG);//Lleva el control al bot�n de salir
			return PuertoCOM;//Sale con �xito	
		}
		 // Si no est� identifica el sistema en el puerto lo cerramos y salimos dejando las cosas como estaban
		else
		{
			cerrar_puerto (PuertoCOM); 
			FlagPuertoAbierto=FlagPuerto; //Deja el flag como estaba al entrar en la funci�n
			PuertoCOM=PCOM;//Deja PuertoCOM como estaba al entar en la funci�n
			return -1;
		}
	}//else //(ErrorRS232 == 0) Si no hay errores configura el protocolo 
}
/**************************************************************************************
 		FIN FUNCIONES PARA GUARDAR Y LEER UN FICHERO CON LOS DATOS DEL PUERTO
 **************************************************************************************/
 /************************************************************************************** 
 		FUNCIONES PARA HACER UNA LISTA DE PUERTOS COM ENCONTRADOS EN EL SISTEMA
 **************************************************************************************/
/****************************************************************************************
	FUNCION QUE SE EJECUTA CUANDO SE PULSA EL BOTON DE "buscar"
	Pone un pop-up de espera y ejecuta 	com_hacer_lista_de_puertos_del_sistema(); 
	Cuando termina actualiza los controles que muestran los puertos del panel
*****************************************************************************************/
int CVICALLBACK buscar_puertos_serie (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int Resultado,indice,ValorPuerto;
if(event==EVENT_LEFT_CLICK)
{
	if (FlagPuertoAbierto){
		cerrar_puerto(PuertoCOM);//Cierra el puerto actual
		FlagPuertoAbierto=0;
	}
	SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,"Selecciona un puerto");	
	hacer_lista_de_puertos_del_sistema();
	//anota el puerto que se ve en el RING en los controles
	GetCtrlIndex (ConfigHandle, CONFIG_RINGCOM, &indice);//Lee el �ndice del puerto seleccionado
	Resultado=GetValueFromIndex (ConfigHandle, CONFIG_RINGCOM, indice,&ValorPuerto);
	if(Resultado<0)
	{
		MessagePopup ("ERROR", "No se han encontrado puertos");
	}
	else
	{
		SetCtrlVal (ConfigHandle, CONFIG_PUERTOCOM, ValorPuerto); // Anota el valor del puerto seleccionado   
	}
}
	return 0;
}
/*****************************************************************************************
   SELECCIONA UN PUERTO DEL RING DONDE EST�N LOS PUERTOS DEL SISTEMA Y LO COLOCA
   EN EL CONTROL DEL QUE SE LEE EL PUERTO A ABRIR.
******************************************************************************************/
int CVICALLBACK seleciona_un_puerto_de_la_lista (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
int indice,ValorPuerto,Resultado;
if(event== EVENT_VAL_CHANGED)
{
	GetCtrlIndex (ConfigHandle, CONFIG_RINGCOM, &indice);//Lee el �ndice del puerto seleccionado
	Resultado=GetValueFromIndex (ConfigHandle, CONFIG_RINGCOM, indice,&ValorPuerto);// Le el valor apuntado por le �ndice 
	if(Resultado<0)
	{
		MessagePopup ("ERROR", "No se han encontrado puertos");
	}
	else
	{
		SetCtrlVal (ConfigHandle, CONFIG_PUERTOCOM, ValorPuerto); // Anota el valor del puerto seleccionado   
	}
}
return 0;
}
/****************************************************************************************
	LEE LOS PUERTOS DEL SISTEMA, Y LOS PONE EN UN RING
	DEVUELVE:El n�mero de puertos encontrados.
****************************************************************************************/
int hacer_lista_de_puertos_del_sistema(void) 
{
	char NombrePuerto[STRCORTO];
	int PuertosEncontrados=0;
	//Limpia la lista
	SetCtrlAttribute (ConfigHandle,CONFIG_RINGCOM, ATTR_DIMMED, 0);
	ClearListCtrl (ConfigHandle,CONFIG_RINGCOM);
	nPortCom=LocateCom("",listaPortCOM,255);//Localiza los puertos COM del sistema
	PuertosEncontrados=nPortCom;
	//for (PuertoCOM=1;PuertoCOM <=255;PuertoCOM++)// Recorre los puertos posibles
	while(nPortCom!=-1) 	
	{
		Fmt (NombrePuerto, "%s<COM %i", listaPortCOM[nPortCom]);  // Crea la cadena nombre del puerto
		InsertListItem (ConfigHandle,CONFIG_RINGCOM,0, NombrePuerto, listaPortCOM[nPortCom]); 	
		nPortCom--;	//Apunta al siguiente puerto de la lista
	}
	return PuertosEncontrados;
}
/************************************************************************************** 
 		FIN FUNCIONES PARA HACER UNA LISTA DE PUERTOS COM ENCONTRADOS EN EL SISTEMA         
**************************************************************************************/
/**************************************************************************************
 		       	FUNCION DEL PANEL DE CONFIGURACION                          
				ESTA FUNCI�N PUEDE CERRAR EL PANEL 
				Recupera la configuraci�n realizada por el usuario	
***************************************************************************************/
int CVICALLBACK panel_configuracion (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{   
	if( event==EVENT_GOT_FOCUS)
	{
		//Esto lo hace solo una vez
		if(FlagPanelConfigRecienAbierto)//Usa el flag para que esto lo haga una sola vez
		{	
			// Llama a la funci�n que establece par�metros del panel
			establece_parametros_com_en_panel();
			FlagPanelConfigRecienAbierto=0;
			hacer_lista_de_puertos_del_sistema();//Rellena el edit ring RINGCOM con los puertos del sistema			
		}
		if(FlagPuertoAbierto)SetCtrlAttribute (ConfigHandle, CONFIG_APLICA_TR, ATTR_DIMMED,0);
		else SetCtrlAttribute (ConfigHandle, CONFIG_APLICA_TR, ATTR_DIMMED,1);
		// Si no hay un puerto abierto borra la identificaci�n del sistema
		if(PuertoCOM==SIN_PUERTO) 
		{
			SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION, "no hay puerto abierto");			
		}
		return 0;
	}//if( event==EVENT_GOT_FOCUS)	
	// Para cerrar el panel sin aplicar cambios
	if (event==EVENT_CLOSE)
	{
		FlagPanelConfigRecienAbierto=1;  
		//if(ModoPanelConfiguracion==PANEL_MODO_HIJO)
		//RemovePopup (0);
		//if(ModoPanelConfiguracion==PANEL_MODO_TOP)
		HidePanel (ConfigHandle);
		return 0;
	}//if (event==EVENT_CLOSE)
	//Cambia entre modo Master y Usuario
	if(event==EVENT_RIGHT_CLICK)
	{
	 	//Aqu� iba el cambio de modo, pero se lo he puesto al led                            
	}//if(event==EVENT_LEFT_CLICK)
return 0;
}
/************************************************************************************** 
 			FUNCIONES PARA CAMBIAR ENTRE MODO MASTER Y USUARIO
**************************************************************************************/ 
/****************************************************************************************
		FUNCION QUE REALIZA EL CAMBIO DE LA VARIABLE ModoMasterUsuario
		Y cambia los controles de los paneles a dimados no desdimados	
****************************************************************************************/
int CVICALLBACK cambiar_de_modo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_RIGHT_DOUBLE_CLICK)
	{
		//char MensajeTopMaster[]="Estas en modo master";
		//char MensajeTopUsuario[]="Estas en modo usuario";			
		//char MensajeDownMaster[]="�Quieres cambiar a modo usuario?";		
		//char MensajeDownUsuario[]="�Quieres cambiar a modo master?";
		if(ModoMasterUsuario==MASTER)
		{
			//if(ConfirmPopup (MensajeTopMaster,MensajeDownMaster))
			{	
				pcom_modo_pcom(USUARIO);
			}
		}
		else 
		{                                                                
			//if(ConfirmPopup (MensajeTopUsuario,MensajeDownUsuario))         
			{
				pcom_modo_pcom(MASTER);
			}
		}                                                                
	}//if(event==EVENT_LEFT_CLICK)
	
	return 0;
}
/******************************************************************************************
			FUNCION PARA MODIFICAR LOS CONTROLES EN UN CABIO DE MODO MASTER / USUARIO
			Actualiza la variable ModoMasterUsuario.                                  			
			Dima o desdima los controles que configuran el puerto en funcion del modo.		
*******************************************************************************************/
void cambia_de_modo_controles_de_configuracion(void)
{
	if(ConfigHandle)
	{
		//SetCtrlAttribute (ConfigHandle, CONFIG_CERRARPUERTO, ATTR_DIMMED,ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_VELOCIDADPUERTO, ATTR_DIMMED,ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_BITSTOP, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_BITDATOS, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_PARIDAD, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_TERMINADORLECTURA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_TERMINADORESCRITURA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_COLAENTRADA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_COLASALIDA, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_MODOCTS, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_MODOXONOFF, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_PUERTOCOM, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_RINGCOM, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_BUSCARPUERTOS, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_ABRIRPUERTO, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (ConfigHandle, CONFIG_BUSCARBASE, ATTR_DIMMED, ModoMasterUsuario);
		
	}
	//Men�
}
/****************************************************************************************** 
			FUNCION PARA MODIFICAR LOS CONTROLES DEL PANEL MENSAJES SEGUN EL MODO       
			Dima o desdima los controles del panel "mensajes" en funcion del modo.		
*******************************************************************************************/
void cambia_de_modo_controles_de_mensajes(void)                                                      
{
	int menuHandle;
	if(MensajesHandle)
	{		
		menuHandle=GetPanelMenuBar(MensajesHandle); 		                      
		SetMenuBarAttribute (menuHandle, MENUSCPI_COM, ATTR_DIMMED, ModoMasterUsuario);        
		SetMenuBarAttribute (menuHandle, MENUSCPI_MENUERROR, ATTR_DIMMED, ModoMasterUsuario);  
		SetMenuBarAttribute (menuHandle, MENUSCPI_IDN, ATTR_DIMMED, ModoMasterUsuario);
		SetMenuBarAttribute (menuHandle, MENUSCPI_MENUCLS, ATTR_DIMMED, ModoMasterUsuario);
		SetCtrlAttribute (MensajesHandle, MENSAJES_COMANDO, ATTR_DIMMED, ModoMasterUsuario);
	}
}
/*************************************************************************************** 
		FUNCION PARA INICIALIZAR LAS VARIABLES Y PANELES DEL PUERTO                                              
	pcom no necesita inicializaci�n, por defecto est� en modo USUARIO. Pero podemos                          
	inicializarlo para que empiece en modo MASTER                                                            
	ARGUMENTO DE ENTRADA:0 para modo MASTER, 1 Para modo USUARIO  
	Hay que asegurarnos de que se han cargado los 2 paneles del puerto "Configuraci�n" 
	y "Mensajes". Luego cambia la variable global ModoMasterUsurio, y por fin dimar
	o desdimar los controles de los paneles en funcion del modo solicitado.
****************************************************************************************/
int pcom_modo_pcom(int short ModoNuevo)                                       
{                                           
	 //Carga paneles en memoria si no lo estaban  	
	if(!MensajesHandle) carga_panel_de_mensajes();
	if(!ConfigHandle) carga_panel_de_configuracion();	
	// Cambia la variable global ModoMasterUsurio en funci�n del par�metro de entrada
	// Y cambia el estado de los paneles
	if(ModoNuevo==USUARIO)                                       
	{                                                                   
		ModoMasterUsuario=USUARIO;                                  
		cambia_de_modo_controles_de_configuracion();                
		cambia_de_modo_controles_de_mensajes();                     
		return 1;
	}                                                                   
	if(ModoNuevo==MASTER)                                       
	{                                                                   
		ModoMasterUsuario=MASTER;                                   
		cambia_de_modo_controles_de_configuracion();                
		cambia_de_modo_controles_de_mensajes();				        
		return 1;
	}
	return 0;
}   
/************************************************************************************** 
 			FIN FUNCIONES PARA CAMBIAR ENTRE MODO MASTER Y USUARIO                          
**************************************************************************************/ 
/**************************************************************************************
		FUNCION QUE ACTUALIZA EL VALOR DEL RETARDO Y EL TIMEOUT
***************************************************************************************/                                
int CVICALLBACK aplicar_retardo_timeout (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT && FlagPuertoAbierto)
	{
		GetCtrlVal (ConfigHandle, CONFIG_TIMEOUT, &Timeout);                                                  	
		SetComTime (PuertoCOM, Timeout);//Aplica el nuevo TimeOut  
		GetCtrlVal (ConfigHandle, CONFIG_RETARDO, &Retardo);
		FlagPanelConfigRecienAbierto=1;
		HidePanel(ConfigHandle);       
	}
	return 0;
}

/**************************************************************************************
         FUNCION PARA APLICAR LAS OPCIONES DEL PANEL DE CONFIGURACION DEL PUERTO 
		 SI EST� EN MODO USUARIO SOLO PUEDE CAMBIAR EL TIMEOUT Y EL RETARDO
		 SI EST� EN MODO MASTER PUEDE CAMBIAR CUALQUIER PAR�METRO, AL APLICAR
		 ABRE EL PUERTO COM CON LOS PARAMETROS SELECCIONADOS
***************************************************************************************/
int CVICALLBACK AplicarConfigCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_COMMIT)
	{
		//Como puede que haya un puerto abierto lo cerramos. Si no lo podemos dejar abierto
		if(PuertoCOM!=-1) cerrar_puerto (PuertoCOM); 			
		lee_parametros_com_del_panel();//Vamos a aplicar los nuevos par�metros de puerto						
		if(ModoMasterUsuario==USUARIO)
		{	
			//Aplica par�metros que pueden cambiar en modo USUARIO
			if(FlagPuertoAbierto) SetComTime (PuertoCOM, Timeout);//Aplica el nuevo TimeOut
		}
		//En modo master se permite tocar todo
		if(ModoMasterUsuario==MASTER) //Cierra el puerto actual, configura el nuevo puerto y lo intenta abrir
		{	
			DisableBreakOnLibraryErrors ();// Inhabilita mensajes de error en tiempo de ejecuci�n			
			cerrar_puerto(PuertoCOM);//Cierra el puerto actual antes de abrir el nuevo
			Delay(1);
			ErrorRS232=abrir_puerto(PuertoCOM,NombrePuertoCOM,VelocidadPuerto,Paridad,BitsDatos,BitsDeStop,ColaDeEntrada,ColaDeSalida);//ABRE EL PUERTO
			Delay(1);
    		if (ErrorRS232!=0) //Si da error al abrir el puerto....
			{                                                                                    			
				cerrar_puerto(PuertoCOM);//Por si acaso lo cierra
				FlagPuertoAbierto=0; //Deja el flag a puerto cerrado
				MessagePopup("Error","No se pudo abrir el puerto.\n No hay ning�n puerto abierto."); 
				PuertoCOM=SIN_PUERTO;
				strcpy(IDNinstrumento,"no hay puerto abierto conectado");//Actualiza IDNinstrumento
				FlagPanelConfigRecienAbierto=1;
				actualiza_controles();//Dima o no los controles en funci�n de como est� el puerto
				return -1;//Salimos con fracaso y sin puerto;
			}                                                                                    			
			else
			{//Si no da errores al abrir termina de configurar el puerto
				FlagPuertoAbierto=1;
				//Leemos los par�metros de configuraci�n del puerto
				SetXMode(PuertoCOM,ModoX);                                                
				SetCTSMode(PuertoCOM, ModoCTS);                                           
				SetComTime (PuertoCOM, Timeout);//Aplica el nuevo TimeOut  
				//Los terminadores son variables de la aplicaci�n que ya est�n actualizadas
				// Si el panele de comunicaci�n no esta en memoria lo carga
				if(!MensajesHandle) carga_panel_de_mensajes();
				// Intenta recibir una identificaci�n del sistema
				identifica_al_sistema();//Actualiza IDNinstrumento	
				// Activa la recepci�n autom�tica para ese puerto        
				pcom_recepcion_automatica(SI);                           
				//Como los par�metros del puerto aplicados son correctos, los salvamos
			}
			SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION, IDNinstrumento);				
			SetCtrlVal (ConfigHandle, CONFIG_PUERTOCOM,PuertoCOM);				
		}                                                                                     			
		EnableBreakOnLibraryErrors (); // Habilita mensajes de error en tiempo de ejecuci�n  			
		FlagPanelConfigRecienAbierto=1;
		pcom_modo_pcom(USUARIO);
		actualiza_controles();//Dima o no los controles en funci�n de como est� el puerto
		//HidePanel(ConfigHandle);       
	}//if(event== EVENT_COMMIT)
   	return 0;                      
}
/*************************************************************************************
	FUNCION QUE DA SERVICIO AL BOTON DE CERRAR EL PUERTO SERIE
**************************************************************************************/
int CVICALLBACK CerrarPuertoCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
if(event==EVENT_COMMIT)
{
	cerrar_puerto(PuertoCOM);
	FlagPuertoAbierto=0;
	actualiza_controles();//Dima los controles del panel mensajes
	pcom_modo_pcom(MASTER);//Para que  pueda volver a abrir el puerto
}
	return 0;
}
/*************************************************************************************
	FUNCION SECRETA PARA ABRIR Y CERRAR EL PUERTO CON EL LED DE "MENSAJES"
	Si el puerto est� abierto lo cierra y si est� cerrado lo abre. Utiliza las 
	variables globales de configuraci�n del puerto para abririlo (no del panel)
	Usar con precauci�n, solo para depuraci�n. Puede dar lugar a errores.
**************************************************************************************/
int CVICALLBACK abre_cierra_com (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_RIGHT_DOUBLE_CLICK)
	{
		if (FlagPuertoAbierto)
		{
			pcom_recepcion_automatica(NO);
			cerrar_puerto(PuertoCOM);
			FlagPuertoAbierto=0;
		}
		else 
		{
			if(PuertoCOM!=-1 && abrir_puerto(PuertoCOM, NombrePuertoCOM, VelocidadPuerto, Paridad,BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida)==0);//ABRE EL PUERTO
			{
				FlagPuertoAbierto=1;
				pcom_recepcion_automatica(SI);//Callback para respuesta autom�tica
			}
		}
	actualiza_controles();
	}
return 0;
}
/****************************************************************************************
	FUNCION QUE ENVIA UN COMANDO POR EL PUERTO SERIE CUANDO 
	SE PULSA ENTER EN EL TEXTBOX enviar
****************************************************************************************/                                
int CVICALLBACK enviar_comando (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_KEYPRESS && eventData1==TECLA_ENTER)
	{	
		GetCtrlVal(MensajesHandle, MENSAJES_COMANDO,CadenaComando);
		
		pcom_enviar_datos(CadenaComando,1);
	}
return 0;
}
/************************************************************************************
			FUNCION QUE ACTIVA O DESACTIVA LA RECEPCION AUTOMATICA DEL PUERTO COM
			ARGUMENTO DE ENTRADA: int 1 para activar o int 0 para desactivar
			DEVUELVE:nada
*************************************************************************************/
int pcom_recepcion_automatica(int activar)
{
	if(!FlagPuertoAbierto) return -3;
	{	
		if(activar)
		{
			switch (TerminadorRecepcion)                                                                                            
        	{                                                                                                                       
          		case 0: //NONE                                                                                       
				return -1; //Sin terminador de recepci�n no se puede activar la recepci�n autom�tica
            	break;                                                                                                              
         		case 1: //CR
					InstallComCallback(PuertoCOM,LWRS_RXFLAG,0,
					TERMINADOR_CR,terminador_detectado_en_buffer_de_entrada,NULL);
	            break;                                                                                                              
    	     	case 2: //LF
					InstallComCallback(PuertoCOM,LWRS_RXFLAG,0,
					TERMINADOR_LF,terminador_detectado_en_buffer_de_entrada,NULL);
            	break;                                                                                                              
				case 3: //CR+LF                                                                              
					InstallComCallback(PuertoCOM,LWRS_RXFLAG,0,
					TERMINADOR_LF,terminador_detectado_en_buffer_de_entrada,NULL);
				break;                                                                                                              			
        	}                  
		}
		else
		{
			InstallComCallback (PuertoCOM, 0, 0, TERMINADOR_LF,/*Da igual el que se ponga aqu�*/
	 		terminador_detectado_en_buffer_de_entrada, 0);//El primer cero desactiva el callback
		}
	}
	return PuertoCOM;	
}
/**************************************************************************************
	FUNCIONES QUE DEVUELVE FlagPuertoAbierto (1 si el puerto est� abierto y 0 si no)
***************************************************************************************/
int pcom_puerto_abierto (void)
{
		return FlagPuertoAbierto; //Devuelve 1 abierto, 0 cerrado
}
/****************************************************************************************
								FUNCIONES DEL MENU
****************************************************************************************/
/****************************************************************************************
					FUNCION QUE CARGA EL PANEL DE CONFIGURACION
****************************************************************************************/
void CVICALLBACK configuracion (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	pcom_muestra_el_panel_de_configuracion(PANEL_MODO_HIJO,100,300);
}
/****************************************************************************************
		FUNCION QUE CARGA EL PANEL DE ESTATUS
****************************************************************************************/
void CVICALLBACK Estatus (int menuBar, int menuItem, void *callbackData,int panel)
{	
	char MensajeDeError[STRMEDIO];
	if(FlagPuertoAbierto)	
	{
		if(!EstatusHandle)EstatusHandle = LoadPanel (MensajesHandle, 
							  "PanelPuertoCOM.uir", ESTATUS);
    	InstallPopup (EstatusHandle); // Pone el panel en memoria 

	}
	else
	{
		Fmt (MensajeDeError, "%s", "Error:�No hay ning�n puerto abierto!\n\n"
			"Abra el panel de configuraci�n\n"
               "en el men� COM/Configurar\n");
          	MessagePopup ("Mensaje RS232", MensajeDeError);
	}
}
/****************************************************************************************                                                     
				FUNCIONES DEL MENU CON COMANDOS SCPI COMUNES                                                                                  
			Envian por el puerto serie el comando seleccionado                                                                                
			Adem�s muestra en el panel MENSAJES las cadenas de salida                                                                         
****************************************************************************************/                                                     
void CVICALLBACK opc_scpi (int menuBar, int menuItem, void *callbackData,                                                                     
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","*OPC");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
}                                                                                                                                             
void CVICALLBACK error_scpi (int menuBar, int menuItem, void *callbackData,                                                                   
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","ERR?");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
}                                                                                                                                             
                                                                                                                                              
void CVICALLBACK version_scpi (int menuBar, int menuItem, void *callbackData,                                                                 
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","MOT:VER?");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
}                                                                                                                                             
void CVICALLBACK cls_scpi (int menuBar, int menuItem, void *callbackData,                                                                     
		int panel)                                                                                                                            
{                                                                                                                                             
	sprintf(CadenaComando,"%s","*CLS");                                                                                                       
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
} 
/****************************************************************************************** 
								FIN FUNCIONES DE MENU                                        
*******************************************************************************************/
/*****************************************************************************************
 		       				FUNCIONES DEL PANEL STATUS                           
******************************************************************************************/
int CVICALLBACK cierra_panel_status (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			if(EstatusHandle)
			{
				DiscardPanel (EstatusHandle);
				EstatusHandle=0;
			}
			break;
		case EVENT_RIGHT_CLICK:

			break;
		}
	return 0;
}
/*****************************************************************************************
					FUNCIONES DEL PANEL MENSAJES
****************************************************************************************/
// Funci�n del Panel
int CVICALLBACK panel_mensajes (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	Rect CoordenadasPanel;
	switch (event)
	{
		case EVENT_PANEL_MOVING:
			GetPanelEventRect (eventData2, &CoordenadasPanel/*Rect *rectangle*/);
			SetCtrlVal(MensajesHandle,MENSAJES_X_POS,CoordenadasPanel.top);
			SetCtrlVal(MensajesHandle,MENSAJES_Y_POS,CoordenadasPanel.left);
		break;		
		case EVENT_CLOSE:
			HidePanel(MensajesHandle);
			//pcom_cierra_puerto_serie(QUITAR_PANELES);
			//QuitUserInterface (0); //SE ABANDONA LA APLICACION
		break;
		case EVENT_GOT_FOCUS:
			actualiza_controles();
		break;
	}
	return 0;
}
/*****************************************************************************************
	FUNCION QUE DIMA O DESDIMA LOS CONTROLES SEGUN ESTE EL PUERTO ABIERTO O NO
****************************************************************************************/
void actualiza_controles(void)
{
	int menuHandle;
	static int FlagAnterior=0;//Inicialmente los controles est�n desdimados
	if(FlagAnterior==FlagPuertoAbierto) return;//Si nada ha cambiado sale
	menuHandle=GetPanelMenuBar(MensajesHandle); 		                      
	SetMenuBarAttribute (menuHandle, MENUSCPI_COM, ATTR_DIMMED, !FlagPuertoAbierto);        
	SetMenuBarAttribute (menuHandle, MENUSCPI_MENUERROR, ATTR_DIMMED, !FlagPuertoAbierto);
	SetMenuBarAttribute (menuHandle, MENUSCPI_IDN, ATTR_DIMMED, !FlagPuertoAbierto);  
	//SetMenuBarAttribute (menuHandle, MENUSCPI_MENUVERSION, ATTR_DIMMED,!FlagPuertoAbierto);
	SetMenuBarAttribute (menuHandle, MENUSCPI_MENUCLS, ATTR_DIMMED, !FlagPuertoAbierto);
	SetCtrlAttribute (MensajesHandle, MENSAJES_COMANDO, ATTR_DIMMED, !FlagPuertoAbierto);
	SetCtrlAttribute (MensajesHandle, MENSAJES_RECIBIR, ATTR_DIMMED, !FlagPuertoAbierto);
	FlagAnterior=FlagPuertoAbierto;
}
/****************************************************************************************
      				 FUNCION PARA LIMPIAR LA VENTANA DE RECEPCION          
****************************************************************************************/
int CVICALLBACK LimpiarRecibir (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{																						 
			case EVENT_COMMIT:
			ResetTextBox (MensajesHandle, MENSAJES_CADENA_RECIBIDA, "");
			break;
		}
	return 0;
}
/****************************************************************************************
      				 FUNCION PARA LIMPIAR LA VENTANA DE COMANDOS          
****************************************************************************************/
int CVICALLBACK LimpiarEnviar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event==EVENT_COMMIT)
	{
		ResetTextBox (MensajesHandle, MENSAJES_CADENA_ENVIAR,"");
	}
	return 0;
}
/*********************************************************************************************
				PONE EN MEMORIA EL PANEL DE MENSAJES COMO TOP-LEVEL
**********************************************************************************************/
int carga_panel_de_mensajes(void)
{
	#define PANEL_PADRE 0 //0 para top-level 1 para child
	if ((MensajesHandle = LoadPanel (PANEL_PADRE, "PanelPuertoCOM.uir", MENSAJES)) < 0)
		return -1;
	
	else
	{
		cambia_de_modo_controles_de_mensajes();
		return MensajesHandle;
	}	
}
/*********************************************************************************************
				MUESTRA EL PANEL DE MENSAJES
**********************************************************************************************/
void pcom_muestra_el_panel_de_mensajes(int Top,int Left)
{
	//Antes de mostrar el panel comprueba si est� en memoria, si no lo carta
	if(!MensajesHandle) carga_panel_de_mensajes();
	DisplayPanel(MensajesHandle);// Muestra el panel de mensajes
	// Coloca el panel en una posici�n determinada en la pantalla
	SetPanelAttribute (MensajesHandle, ATTR_TOP,Top);
	SetPanelAttribute (MensajesHandle, ATTR_LEFT,Left);
	SetCtrlVal(MensajesHandle,MENSAJES_X_POS,Top);
	SetCtrlVal(MensajesHandle,MENSAJES_Y_POS,Left);
}
/*********************************************************************************************
	CARGA EL PANEL DE CONFIGURACION EN MEMORIA COMO HIJO DEL PAR�METRO DE ENTRADA
	Par�metro de entrada: tipo int; tiene que ser un Handle de panel activo
	o 0 para cargarlo como top-level.
**********************************************************************************************/
int carga_panel_de_configuracion(void)
{
#define PANEL_PADRE 0	//0 para que sea top level 1 para que no lo sea
	if ((ConfigHandle = LoadPanel (PANEL_PADRE, "PanelPuertoCOM.uir", CONFIG)) < 0)
	return -1;
	else 
	{
		return ConfigHandle;
	}
}
/*********************************************************************************************
			MUESTRA EL PANEL DE CONFIGURACION DEL PUERTO COM
**********************************************************************************************/
void pcom_muestra_el_panel_de_configuracion(int ModoPanel,int Top,int Left)
{
	//Antes de mostrar el panel tiene que ver si est� en memoria. Si no est� lo carga.
	if(!ConfigHandle) carga_panel_de_configuracion();
	lee_parametros_com_del_panel ();//Actualiza las variables del puerto desde el panel
	if(ModoPanel==PANEL_MODO_HIJO)	
	{if(ConfigHandle) InstallPopup (ConfigHandle);ModoPanelConfiguracion=PANEL_MODO_HIJO;}
	if(ModoPanel==PANEL_MODO_TOP)	
	{
		if(ConfigHandle) DisplayPanel (ConfigHandle);ModoPanelConfiguracion=PANEL_MODO_TOP;
	}
	//Si carga el panel y est� en modo usuario y no hay puerto abierto, hay que activar
	//los  controles para que pueda abrir el puerto
	if (!FlagPuertoAbierto && ModoMasterUsuario==USUARIO)
	{
		pcom_modo_pcom(MASTER);
	}
	SetPanelAttribute (ConfigHandle, ATTR_TOP,Top);
	SetPanelAttribute (ConfigHandle, ATTR_LEFT,Left);
}
/*********************************************************************************************
				QUITA DE LA MEMORIA LOS PANELES DEL PUERTO COM
**********************************************************************************************/
void descarga_paneles_del_puerto_com(void)
{
	if(ConfigHandle) {DiscardPanel (ConfigHandle);ConfigHandle=0;}
	if(MensajesHandle) {DiscardPanel (MensajesHandle);MensajesHandle=0;}
}
/*********************************************************************************************
 				LEE LOS PARAMETROS DE CONFIGURACION DEL PUERTO                            
 *********************************************************************************************/
void lee_parametros_com_del_panel (void)
{
	GetCtrlVal (ConfigHandle, CONFIG_PUERTOCOM, &PuertoCOM);//Este es el puerto que queremos utilizar	
	GetCtrlVal (ConfigHandle, CONFIG_TIMEOUT, &Timeout);                                                  	
	GetCtrlVal (ConfigHandle, CONFIG_RETARDO, &Retardo);                                                  	
	GetCtrlVal (ConfigHandle, CONFIG_MODOCTS, &ModoCTS);                                                  	
	GetCtrlVal (ConfigHandle, CONFIG_MODOXONOFF, &ModoX);                                                 	
	GetCtrlIndex (ConfigHandle, CONFIG_TERMINADORLECTURA, &TerminadorRecepcion);                          	
	GetCtrlIndex (ConfigHandle, CONFIG_TERMINADORESCRITURA, &TerminadorTransmision);                      	
	GetCtrlVal (ConfigHandle, CONFIG_COLASALIDA, &ColaDeSalida);                       			          	
	GetCtrlVal (ConfigHandle, CONFIG_COLAENTRADA, &ColaDeEntrada);                     			          	
	GetCtrlVal (ConfigHandle, CONFIG_BITSTOP, &BitsDeStop);                            	                  	
	GetCtrlVal (ConfigHandle, CONFIG_BITDATOS, &BitsDatos);                            			          	
	GetCtrlVal (ConfigHandle, CONFIG_PARIDAD, &Paridad);                                                  	
	GetCtrlVal (ConfigHandle, CONFIG_VELOCIDADPUERTO, &VelocidadPuerto);                                  	
}
/********************************************************************************************
			ESTABLECE LOS PARAMETROS DE CONFIGURACION DEL PUERTO                     
*********************************************************************************************/
void establece_parametros_com_en_panel(void)
{
	
	SetCtrlVal (ConfigHandle, CONFIG_TIMEOUT, Timeout);
	SetCtrlVal (ConfigHandle, CONFIG_RETARDO, Retardo);
    SetCtrlVal (ConfigHandle, CONFIG_PUERTOCOM, PuertoCOM);
    SetCtrlVal (ConfigHandle, CONFIG_VELOCIDADPUERTO, VelocidadPuerto);
    SetCtrlVal (ConfigHandle, CONFIG_PARIDAD, Paridad);
    SetCtrlVal (ConfigHandle, CONFIG_BITDATOS, BitsDatos);
    SetCtrlVal (ConfigHandle, CONFIG_BITSTOP, BitsDeStop);
    SetCtrlVal (ConfigHandle, CONFIG_COLAENTRADA, ColaDeEntrada);
    SetCtrlVal (ConfigHandle, CONFIG_COLASALIDA, ColaDeSalida);
    SetCtrlVal (ConfigHandle, CONFIG_MODOCTS, ModoCTS);
    SetCtrlVal (ConfigHandle, CONFIG_MODOXONOFF,ModoX);
    SetCtrlIndex (ConfigHandle, CONFIG_TERMINADORLECTURA, TerminadorRecepcion);
	SetCtrlIndex (ConfigHandle, CONFIG_TERMINADORESCRITURA, TerminadorTransmision);
	SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION, IDNinstrumento);				
}

/******************************************************************************************      
	FUNCIONES QUE DEVUELVE EL FLAG DE DATO RECIBIDO AUTOM�TICAMENTE
	Devuelve 1 cuando se produce recepci�n autom�tica de cadena por el puerto serie	
	Borra el flag tras ejecutarse.
*******************************************************************************************/     
int short com_flag_dato_recibido (void)                                                          
{                                                                                            
	if(FlagDatosRecibidos)
	{	
		FlagDatosRecibidos=NO;
		return SI;
	}
	return NO;
}                                                                                            
/******************************************************************************************
				FUNCION QUE MUESTRA LOS ERRORES DEL PUERTO                  
/******************************************************************************************/
void display_error_RS232 (void)
{
	int Respuesta;
	char ErrorMessage[STRLARGO];
	switch (ErrorRS232)
        {
        case 0  :
            MessagePopup ("Mensaje RS232", "No hay errores.");
            break;
        case -1  :
            //For error code -1 (UnknownSystemError), call the GetErrorRS232String 
			//function to obtain a specific Windows message string. 
			Respuesta=ConfirmPopup ("Mensaje RS232",
				"�El puerto se desconect�!\n�Cerrar la aplicaci�n?");
			if(Respuesta==1)
			{
				pcom_cierra_puerto_serie(QUITAR_PANELES);
				QuitUserInterface (0);
			}
            break;
		case -2 :
            Fmt (ErrorMessage, "%s", "n�mero de puerto invalido (debe valer "
                                     "entre 1 y 255).");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
        case -3 :
            Fmt (ErrorMessage, "%s", "El puerto no est� abierto.\n"
                 "Entre en el men�  Com/configurar ");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
		case -5 :
            Fmt (ErrorMessage, "%s", "Error en el puerto serie");
            MessagePopup ("Mensaje RS232", ErrorMessage);
        break;
		case -6 :
            Fmt (ErrorMessage, "%s", "Puerto no encontrado");
            MessagePopup ("Mensaje RS232", ErrorMessage);
        break;
        case -99 :
            Fmt (ErrorMessage, "%s", "-99 error de Timeout.\n\n"
				 "Incremente el valor de Timeout,\n"
                 "Compruebe  la  configuraci�n, o\n"
                 "Compruebe el puerto.");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
			case ERROR_DATOS_EXCESIVOS :
            Fmt (ErrorMessage, "%s", "error, demasiados datos en el buffer.\n");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
		default :
            if (ErrorRS232 < 0)
                {  
                Fmt (ErrorMessage, "%s<RS232 error n�mero %i", ErrorRS232);
                MessagePopup ("Mensaje RS232", ErrorMessage);
                }
            break;
        }
}
/***************************************************************************************
								FUNCIONES DE MENU
****************************************************************************************/
/**************************************************************************************                                                   
			FUNCION QUE  MUESTRA EL NUMERO DE BYTES EN LA COLA DE ENTRADA                                                                 	 
***************************************************************************************/                                                  
int CVICALLBACK pcom_longitud_cola_entrada (int panel, int control, int event,                                                             
                                void *callbackData, int eventData1,                                                                       
                                int eventData2)                                                                                           
{                                                                                                                                         
    int BytesEnBufferDeEntrada;                                                                                                           
	if (event == EVENT_COMMIT && FlagPuertoAbierto)                                                                                                           
        {                                                                                                                                 
        BytesEnBufferDeEntrada = GetInQLen (PuertoCOM);                                                                                   
        Fmt (Msg1, "%s<Longitud de la cola de entrada = %i", BytesEnBufferDeEntrada);                                                     
        MessagePopup ("Mensaje RS232", Msg1);                                                                                             
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
/**************************************************************************************                                                   
		FUNCION QUE  MUESTRA EL NUMERO DE BYTES EN LA COLA DE SALIDA                                                                      
*******	********************************************************************************/                                                 
int CVICALLBACK pcom_longitud_cola_salida (int panel, int control, int event,                                                              
                                 void *callbackData, int eventData1,                                                                      
                                 int eventData2)                                                                                          
                                                                                                                                          
{                                                                                                                                         
int BytesEnBufferDeSalida;                                                                                                                
	if (event == EVENT_COMMIT && FlagPuertoAbierto)
        {                                                                                                                                 
        BytesEnBufferDeSalida = GetOutQLen (PuertoCOM);                                                                                   
        Fmt (Msg1, "%s<Longitud de la cola de salida = %i", BytesEnBufferDeSalida);                                                       
        MessagePopup ("Mensaje RS232", Msg1);                                                                                             
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
/**************************************************************************************                                                   
             FUNCION QUE LIMPIA LA COLA DE ENTRADA                                                                                        
***************************************************************************************/                                                  
int CVICALLBACK pcom_borra_cola_entrada (int panel, int control, int event,                                                             
                                 void *callbackData, int eventData1,                                                                      
                                 int eventData2)                                                                                          
{                                                                                                                                         
    if (event == EVENT_COMMIT && FlagPuertoAbierto)
        {                                                                                                                                 
        FlushInQ (PuertoCOM);                                                                                                  
		MessagePopup ("Mensaje RS232", "�Cola de entrada limpia!");                                                                       
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
                                                                                                                                          
/**************************************************************************************                                                   
                FUNCION QUE LIMPIA LA COLA DE SALIDA                                                                                      
***************************************************************************************/                                                  
int CVICALLBACK pcom_borra_cola_salida (int panel, int control, int event,                                                                 
                                   void *callbackData, int eventData1,                                                                    
                                   int eventData2)                                                                                        
{                                                                                                                                         
    if (event == EVENT_COMMIT && FlagPuertoAbierto)
        {                                                                                                                                 
        FlushOutQ (PuertoCOM);                                                                                                            
		MessagePopup ("Mensaje RS232", "�Cola de salida limpia!");                                                                        
        }                                                                                                                                 
    return 0;                                                                                                                             
}                                                                                                                                         
/**************************************************************************************                                                   
           FUNCION QUE LEE EL ESTATUS DEL PUERTO SERIE                                                                                    
***************************************************************************************/                                                  
int CVICALLBACK pcom_muestra_status (int panel, int control, int event,                                                                    
                                   void *callbackData, int eventData1,                                                                    
                                   int eventData2)                                                                                        
{                                                                                                                                         
    int com_status;      // C�digo de estatus del puerto                                                                                  
	if (event == EVENT_COMMIT && FlagPuertoAbierto)                                                                                                            
        {                                                                                                                                 
        com_status = GetComStat (PuertoCOM);                                                                                              
        muestra_mensaje_status (com_status);                                                                                          
        }                                                                                                                                 
    return 0;                                                                                                                             
} 
/**************************************************************************************                                                   
 		       FUNCION QUE LEE ERRORES DEL PUERTO                                                                                         
***************************************************************************************/                                                  
int CVICALLBACK pcom_muestra_error (int panel, int control, int event,                                                                     
                               void *callbackData, int eventData1,                                                                        
                               int eventData2)                                                                                            
{                                                                                                                                         
	switch (event)                                                                                                                        
        {                                                                                                                                 
        case EVENT_COMMIT:                                                                                                                
            ErrorRS232 = ReturnRS232Err (); // El sistema informa del error registrado                                                    
            display_error_RS232 ();			// en la �ltima operaci�n del puerto                                                      
            break;                                                                                                                        
        case EVENT_RIGHT_CLICK :                                                                                                          
            break;                                                                                                                        
        }                                                                                                                                 
    return 0; 
}                                                                                                                                         
/**************************************************************************************                                                   
          FUNCION QUE MUESTRA EL ESTATUS DEL PUERTO SERIE                                                                                 
***************************************************************************************/                                                  
void muestra_mensaje_status (com_status)                                                                                              
{                                                                                                                                         
    ComMsg[0] = '\0';                                                                                                                     
    if (com_status & 0x0001)                                                                                                              
        strcat (ComMsg, "Input lost: Input queue"                                                                                         
                " filled and characters were lost.\n");                                                                                   
    if (com_status & 0x0002)                                                                                                              
        strcat (ComMsg, "Asynch error: Problem "                                                                                          
                "determining number of characters in input queue.\n");                                                                    
    if (com_status & 0x0010)                                                                                                              
        strcat (ComMsg, "Paridad error.\n");                                                                                              
    if (com_status & 0x0020)                                                                                                              
        strcat (ComMsg, "Overrun error: Received"                                                                                         
                " characters were lost.\n");                                                                                              
    if (com_status & 0x0040)                                                                                                              
        strcat (ComMsg, "Framing error: Stop bits were not received"                                                                      
                " as expected.\n");                                                                                                       
    if (com_status & 0x0080)                                                                                                              
        strcat (ComMsg, "Break: A break signal was detected.\n");                                                                         
    if (com_status & 0x1000)                                                                                                              
        strcat (ComMsg, "Remote XOFF: An XOFF character was received."                                                                    
                "\nIf XON/XOFF was enabled, no characters are removed"                                                                    
                " from the output queue and sent to another device "                                                                      
                "until that device sends an XON character.\n");                                                                           
    if (com_status & 0x2000)                                                                                                              
        strcat (ComMsg, "Remote XON: An XON character was received."                                                                      
                "\nTransmisson can resume.\n");                                                                                           
    if (com_status & 0x4000)                                                                                                              
        strcat (ComMsg, "Local XOFF: An XOFF character was sent to\n"                                                                     
                " the other device.  If XON/XOFF was enabled, XOFF is\n"                                                                  
                " transmitted when the input queue is 50%, 75%, and 90%\n"                                                                
                " full.\n");                                                                                                              
    if (com_status & 0x8000)                                                                                                              
        strcat (ComMsg, "Local XON: An XON character was sent to\n"                                                                       
                " the other device.  If XON/XOFF was enabled, XON is\n"                                                                   
                " transmitted when the input queue empties after XOFF\n"                                                                  
                " was sent.  XON tells the other device that it can \n"                                                                   
                " resume sending data.\n");                                                                                               
    if (strlen (ComMsg) == 0)                                                                                                             
        strcat (ComMsg, "No status bits are set.");                                                                                       
    MessagePopup ("Mensaje RS232", ComMsg);                                                                                               
}                                                                                                                                         
/*************************************************************************************** 
		FUNCION QUE PIDE LA IDENTIFICACION DEL SISTEMA     
		Env�a al puerto serie PuertoCOM (global en el m�dulo) un *IDN
		Si todo va bien rellena la cadena IDNinstrumento[] y la formatea correctamente
		para poder hacer una comparaci�n.
		DEVUELVE:  la longitud de la cadena de identificaci�n. Si el puerto est�
		cerrado devuelve -3.
		
****************************************************************************************/
int identifica_al_sistema(void)
{
	int short Indice=0;
	int LongMensaje;
	int status;
	IDNinstrumento[0]='\0';//Borra la cadena donde se lee la identificaci�n
	sprintf( CadenaComando, "%s", "*IDN"); // Prepara la cadena con el comando de identificaci�n
	status=pcom_enviar_datos( CadenaComando,MOSTRAR);
	Delay(Retardo); // Espera un poco Retardo es el del panel de configuraci�n del puerto
	pcom_recibir_datos(IDNinstrumento,1);// Lee la posible respuesta
	// Busca el terminador (ver #define Terminador) y pone tras el un finalizador de cadena '\0'
	LongMensaje=strlen (IDNinstrumento);
	while(IDNinstrumento[Indice]!=TERMINADOR && Indice<=LongMensaje)Indice++;IDNinstrumento[Indice+1]='\0';
	return status;
}	
/****************************************************************************************    
			FUNCION PUBLICA PARA LIMPIAR LA PILA DE ERRORES SCPI DEL MICRO                          
			                                                                                 
****************************************************************************************/    
void pcom_limpia_pila_errores()                                                              
{                                                                                            
	int LongCadenaCls;                                                                       
	LongCadenaCls = StringLength ("*CLS");                                                   
	CopyString (CadenaComando, 0,"*CLS", 0,LongCadenaCls);                                   
	pcom_enviar_datos(CadenaComando,NO_MOSTRAR); // envia el comando completo por el puerto	 
}                                                                                            
/****************************************************************************************    
					FUNCION QUE CAMBIA DE COLOR EL LED DEL PANEL MENSAJES
****************************************************************************************/    
void led_color(enum enumColor ColorNuevo)
{
	switch (ColorNuevo)
	{
		case Rojo:
		SetCtrlAttribute (MensajesHandle,MENSAJES_LEDCOM, ATTR_ON_COLOR, VAL_RED);
		break;
		case Verde:
		SetCtrlAttribute (MensajesHandle,MENSAJES_LEDCOM, ATTR_ON_COLOR, VAL_GREEN);
		break;
		case Azul:
		SetCtrlAttribute (MensajesHandle,MENSAJES_LEDCOM, ATTR_ON_COLOR, VAL_BLUE);
		break;
	}
	
}
/*************************************************************************************                                          

		FUNCIONES QUE LEEN, ESCRIBEN, CONFIGURAN O CIERRAN EL PUERTO
		Susceptibles de producir un error a nivel de librer�a RS-232
				
**************************************************************************************/


/*************************************************************************************                                 
						FUNCION QUE LEE LOS DATOS DEL BUFFER DE ENTRADA                                                                
**************************************************************************************/                                
int CVICALLBACK leer_buffer_entrada (int panel, int control, int event,                                                                
		void *callbackData, int eventData1, int eventData2)                                                                            
{                                                                                                                                      
	int BytesEnBuffer;                                             
	int BytesLeidos;                                               
	char CadenaLeida[STRLARGO]="\0"; // Lee sobre una cadena limpia
	
	if (event==EVENT_COMMIT)                                                                                                                     
	{                                                                                                                                  
		// Si el pueto  no est� abierto  salimos
		if (!FlagPuertoAbierto)
		{
			SetCtrlVal(panel,MENSAJES_CADENA_RECIBIDA,"NO HAY NINGUN PUERTO ABIERTO\n");

		//MessagePopup ("Mensaje RS232", "No hay ning�n puerto abierto");
		return 0;
		}

		// Si el pueto  est� abierto  podemos recibir 
		else // RECEPION
		{
			// Si el n� de bytes en el buffer del COM es razonable..
			BytesEnBuffer = GetInQLen (PuertoCOM);
			if(BytesEnBuffer==0)
			{
				SetCtrlVal(panel,MENSAJES_CADENA_RECIBIDA,"NO HAY DATOS EN EL PUERTO\n");
				//MessagePopup ("Mensaje RS232", "No hay datos en el buffer");return 0;
				return 0;
			}
			if(BytesEnBuffer<0)
			{
				//display_error_RS232();
				SetCtrlVal(panel,MENSAJES_CADENA_RECIBIDA,"ERROR EN EL PUERTO\n");
				return 0;
			}// Si la lectura da un error lo avisa y sale   		
			if(BytesEnBuffer > STRLARGO)
			{
				//ErrorRS232=ERROR_DATOS_EXCESIVOS;display_error_RS232();
				SetCtrlVal(panel,MENSAJES_CADENA_RECIBIDA,"ERROR EXCESO DE DATOS\n");
				FlushInQ (PuertoCOM);
				return 0;
			
			}
			BytesLeidos=ComRd (PuertoCOM, CadenaLeida, BytesEnBuffer);
			if(BytesLeidos <0)
			{
				SetCtrlVal(panel,MENSAJES_CADENA_RECIBIDA,"IMPOSIBLE LEER DATOS\n");
				//ErrorRS232=BytesLeidos; display_error_RS232(); 
			return 0;
			}
	        // Tras hacer una lectura copia lo leido en CadenaRecibida y limpia el buffer del puerto COM
			CopyString (CadenaRespuesta, 0, CadenaLeida, 0, BytesLeidos);
			SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, CadenaRespuesta);// Muestra la cadena                                 
			return BytesLeidos;
		}//else
	}                                                                                                                                  
	return 0;                                                                                                                          
}                                                                                                                                      

/*************************************************************************************                                          
							CIERRA EL PUERTO SERIE                                                                              
	Si el puerto est� abierto, limpia los buffers de entrada y salida, cierra                                                                                        
	el puerto y descarga los paneles.                                                                                                                  
**************************************************************************************/                                         
void pcom_cierra_puerto_serie(int QuitarPaneles)                                                                                             
{                                                                                                                               
	if (FlagPuertoAbierto)                                                                                                      
    {                                                                                                                           
		// Primero guardo los datos del puerto en el fichero, si no hay errores en el puerto
		ErrorRS232 = ReturnRS232Err();
		if(ErrorRS232==0)guarda_puerto_com_en_fichero();    
		FlushOutQ (PuertoCOM);                                                                                                  
        FlushInQ (PuertoCOM);                                                                                                   
		ErrorRS232 = cerrar_puerto (PuertoCOM);                                                                                      
		FlagPuertoAbierto=0; // Para que el programa sepa que el puerto est� cerrado                                            
	    // TO DO Gesti�n de errores
		if (ErrorRS232) display_error_RS232 ();                                                                             
		PuertoCOM=SIN_PUERTO;                                                                                                   
		if (QuitarPaneles) descarga_paneles_del_puerto_com();//Quita los paneles de la memori
    }                                                                                                                           
 }                                                                                                                               
/***************************************************************************************                                        
	FUNCION PARA ENVIAR DATOS                                                                                                   
	Funci�n que comprueba si se pueden enviar datos por el puerto serie. Si se puede                                            
	a�ade el terminador y los envia; si no, da un mensaje de error en pantalla.                                                
	"Mostrar"  se pone a 1 para que la cadena se vea en el test box MENSAJES_CADENA_ENVIAR                                     
	Devuelve ErrorRS232
*****************************************************************************************/
int pcom_enviar_datos (char CadenaEnviar[], int Mostrar)  {                                                                     
// Si el pueto no est� abierto no podemos transmitir                                                                            
	int LongitudString;
	//  -3, "kRS_PortNotOpen Port", " is not open. ",                                                                           
	if(!FlagPuertoAbierto) return -3;// No accede al puerto si no est� abierto                                             
    // Si el puerto est� abierto transmitimos                                                                                   
    else                                                                                                                        
	{                                                                                                                           
    	switch (TerminadorTransmision)                                                                                          
    	{                                                                                                                       
    	case 1:                                                                                                                 
       		  strcat(CadenaEnviar, "\r");                                                                                         
       		break;                                                                                                              
    	case 2:                                                                                                                 
       		strcat(CadenaEnviar, "\n");                                                                                         
       		break;                                                                                                              
		case 3:                                                                                                                 
       		strcat(CadenaEnviar, "\r\n");                                                                                       
       		break;                                                                                                              
    	}                                                                                                                       
	    LongitudString = StringLength (CadenaEnviar);                                                                           
		// Env�a la cadena  por el puerto                                                                                       
		ComWrt (PuertoCOM, CadenaEnviar, LongitudString);//AQUI ESCRIBE EN EL PUERTO
		ErrorRS232 = ReturnRS232Err ();
		if (ErrorRS232)                                                                                                         
		{ 
			ErrorDeTransmision++;
			//Si suma 3 errores de transmisi�n...cierra el puerto y pone el panel de configuraci�n
			if(ErrorDeTransmision==MAX_ERR_TRANSMISION && PuertoCOM!=-1)
			{			
				FlagPuertoAbierto=0;
				cerrar_puerto(PuertoCOM);
				LocateCom("",listaPortCOM,255);//Localiza los puertos COM del sistema
				//Muestra el panel de configuraci�n para que el usuario busque el puerto manualmente
				pcom_muestra_el_panel_de_configuracion(PANEL_MODO_HIJO,100,300);  
				SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,"El puerto no est� transmitiendo, intenta abrir un puerto");	
				return ErrorRS232;
			}
			display_error_RS232 ();                                                                                           
			led_color(Rojo);
			return ErrorRS232;                                                                                                               
		}                                                                                                                       
		else                                                                                                                    
		{                                                                                                                       
			//if(Mostrar)ResetTextBox (MensajesHandle, MENSAJES_CADENA_ENVIAR, CadenaEnviar);                                   
			if(Mostrar)
			{                                                                                                       
				if(MensajesHandle)SetCtrlVal (MensajesHandle, MENSAJES_CADENA_ENVIAR, CadenaEnviar);                                
			}                                                                                                                  
		}                                                                                                                       
		led_color(Azul);
		return ErrorRS232;                                                                                                      
	}                                                                                                                           
}                                                                                                                               
/***************************************************************************************                                        
	FUNCION PARA RECIBIR DATOS                                                                                                  
	Lee datos del buffer seg�n el terminador seleccionado.
	DEVUELVE: El n�mero de bytes leidos o el c�digo de error si se produce
****************************************************************************************/                                       
int pcom_recibir_datos (char CadenaRecibida[],int short Mostrar)                                                                                  
{                                                                                                                               
	int BytesEnBuffer;                                                                                                          
	int BytesLeidos;                                                                                                            
	char CadenaLeida[STRLARGO]="\0"; // Lee sobre una cadena limpia                                                             
	static int CaracterTerminador; // Caracter a incluir en la cadena de emisi�n como terminador 
	if(!FlagPuertoAbierto) return -3;// No accede al puerto si no est� abierto                                             	                                                                                                                            
	led_color(Verde);
	// Si el pueto  est� abierto  podemos recibir                                                                       
  	// Selecciona el terminador de recepci�n                                                                                
   	switch (TerminadorRecepcion)                                                                                            
   {                                                                                                                       
	   	case 0:                                                                                                             
	    CaracterTerminador = 0;//NONE                                                                                       
	    break;                                                                                                              
	    case 1:                                                                                                             
	    CaracterTerminador = 13;  //CR                                                                                      
	    break;                                                                                                              
	    case 2:CaracterTerminador = 10; //LF                                                                                
	    break;                                                                                                              
    }                                                                                                                       
	//Cuenta los bytes en el buffer de entrada
	BytesEnBuffer = GetInQLen (PuertoCOM);                                                                                  
	//Ahora comprueba si se ha producido alg�n error
	ErrorRS232=ReturnRS232Err ();
	if(ErrorRS232<0) //Si la lectura da un error sale                                                                                                    
	{
		display_error_RS232 ();
		led_color(Rojo);
		return ErrorRS232;
	}
	// Comprueba posible error de cadena recibida demasiado larga (este error lo defino yo a -9)
	if(BytesEnBuffer > STRLARGO)                                                                                            
	{
		ErrorRS232=ERROR_DATOS_EXCESIVOS;
		display_error_RS232 ();
		led_color(Rojo);
		return ErrorRS232;
	}                                                 
	// Lee el puerto
	if (CaracterTerminador) // Lee con terminador                                                                           
		BytesLeidos = ComRdTerm (PuertoCOM, CadenaLeida, STRLARGO,CaracterTerminador);// LECTURA DEL PUERTO !!!                                  
		                                                                                                                   
    else // Lee sin terminador.                                                                                      
	  	BytesLeidos = ComRd (PuertoCOM, CadenaLeida, BytesEnBuffer);                                                    
	//Comprueba si a habido errores en la lectura
	ErrorRS232=ReturnRS232Err ();
	if(ErrorRS232<0) //Si la lectura da un error sale
	{
		display_error_RS232 ();
		led_color(Rojo);
		return ErrorRS232;
	}
       // Tras hacer una lectura correcta copia lo leido en CadenaRecibida y limpia el buffer del puerto COM                        
	CopyString (CadenaRecibida, 0, CadenaLeida, 0, BytesLeidos);
	if(Mostrar)
	{	
		SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, CadenaRecibida);// Muestra la cadena                                 
		SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, "\n");//                                  
	}
	led_color(Azul);
	return BytesLeidos;                                                                                                     
}                                                                                                                               
/*****************************************************************************************************                                 
	FUNCION QUE SE EJECUTA AUTOMATICAMENTE CUANDO SE DETECTA "Terminador" EN EL BUFFER                                            
 	Esta funci�n se ejecuta cuando se detecta Terminador de recepci�n en el buffer de entrada                                                
	del puerto COM.                                                                                                             
    Esta funci�n no utiliza pcom_recibir_datos(..) sino que utiliza ComRd(..). No utiliza                                       
	ComRdTerm(..) porque quita el terminador.                                                                                   
	Antes de salir limpia el buffer del COM abierto                                                                             
	Devuelve el resultado de la lectura en CadenaRespuesta que es global                                                                       
******************************************************************************************************/                                
void CVICALLBACK terminador_detectado_en_buffer_de_entrada(int portNo,int eventMask,void *callbackData)
{                                                                                                                               
	int BytesLeidos;                                                                                                            
	int Indice; // Para revisar la cadena de entrada                                                                            
	CadenaRespuesta[0]='\0';// Resetea la cadena para leer datos del puerto                                                     
	
	switch (TerminadorRecepcion)                                                                                            
        {                                                                                                                       
          	case 0: //NONE                                                                                       
				return; //
            break;                                                                                                              
         	case 1: //CR Al leer quita el CR
				BytesLeidos=ComRdTerm (PuertoCOM, CadenaRespuesta, STRLARGO,TERMINADOR_CR);//Lectura del puerto hasta CR
				if (BytesLeidos<0){ErrorRS232=BytesLeidos;display_error_RS232 (); return;}//                                            
				CadenaRespuesta[BytesLeidos]=TERMINADOR_CR; // Le a�ade un retorno de carro eliminanado el terminador que ha quedado
				CadenaRespuesta[++BytesLeidos]='\0'; // Le a�ade un fin de cadena                                                  
            break;                                                                                                              
         	case 2: //LF Al leer no quita el LF
				BytesLeidos=ComRdTerm (PuertoCOM, CadenaRespuesta, STRLARGO,TERMINADOR_LF);//Lectura del puerto hasta LF
				if (BytesLeidos<0){ErrorRS232=BytesLeidos;display_error_RS232 (); return;}//                                            
				CadenaRespuesta[BytesLeidos]=TERMINADOR_CR; // Le a�ade un retorno de carro eliminanado el terminador que ha quedado
				CadenaRespuesta[++BytesLeidos]='\0'; // Le a�ade un fin de cadena                                                  
            break;                                                                                                              
			case 3: //CR+LF al leer quita ambos caracteres  (Arduino)
				BytesLeidos=ComRdTerm (PuertoCOM, CadenaRespuesta, STRLARGO,TERMINADOR_CR);//Lectura del puerto hasta CR ya que hay un CR
				if (BytesLeidos<0){ErrorRS232=BytesLeidos;display_error_RS232 (); return;}//                                            
				CadenaRespuesta[BytesLeidos++]=TERMINADOR_CR; // Le a�ade un retorno de carro eliminanado el terminador que ha quedado
				CadenaRespuesta[BytesLeidos]='\0'; // Le a�ade un fin de cadena                                                  
			break;                                                                                                              			
        }                  
	SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, CadenaRespuesta);// Muestra la cadena                                 
	ErrorRS232 = ReturnRS232Err ();                                                                                    
	if (ErrorRS232) display_error_RS232 ();
	pcom_datos_recibidos();//Llama a la funci�n que se define en la aplicaci�n principal                                         
	FlagDatosRecibidos=SI;//Flag para lebantar cuando se recibe en autom�tico
	return;                                         	
}	                                                                                                                            
/*****************************************************************************************************                                 
	FUNCION QUE ABRE EL PUERTO SERIE
	Devuelve el resultado de ejecutar la apertura del puerto con OpenConfig
******************************************************************************************************/                                
int abrir_puerto(int PCOM, const char Nombre[],int Vel,int Par,int Bits,int Stop,int ColEn,int ColSal)
{
	ErrorRS232 = OpenComConfig (PCOM,Nombre,Vel,Par,Bits,Stop,ColEn,ColSal);//ABRE EL PUERTO	
	if(!ErrorRS232)
	{
		SetCtrlVal(MensajesHandle,MENSAJES_LEDCOM,1); 
		SetCtrlVal(ConfigHandle,CONFIG_LEDCONFIG,1); 
	}		
	else //Si hay error no ha abierto el puerto
	{
		FlagPuertoAbierto=0;
	}
	return ErrorRS232;
}
/*****************************************************************************************************
	FUNCION QUE CIERRA EL PUERTO SERIE
	Cierro el puerto pasado como argumento
	FlagPuertoAbierto lo tiene que poner a cero la funci�n llamante
******************************************************************************************************/                                
int cerrar_puerto(int PCOM)
{
	int Resultado;
	Resultado=CloseCom (PCOM);
	SetCtrlVal(MensajesHandle,MENSAJES_LEDCOM,0);
	SetCtrlVal(ConfigHandle,CONFIG_LEDCONFIG,0); 
	return Resultado;
}
/*****************************************************************************************************                                 
		Funci�n que busca el instrumento "IdentificacionDelInstrumento"en el 
		puerto seleccionado. Si lo encuentra puerto abierto y cierra el panel.Si
		no, deja el puerto cerrado.
******************************************************************************************************/                                
int CVICALLBACK buscar_instrumento (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int puerto;
	int status;
	char mensaje[STRMEDIO];
	if(event==EVENT_COMMIT)
	{
		if(FlagPuertoAbierto) //Si hay un puerto abierto lo cierra
		{
			cerrar_puerto(PuertoCOM); 	
			FlagPuertoAbierto=0;
		}	
		DisableBreakOnLibraryErrors (); // Funci�n obsoleta 					
		SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,"buscando una base en el puerto seleccionado: por favor espere..");	
		GetCtrlVal (ConfigHandle, CONFIG_PUERTOCOM, &puerto);//Lee el puerto 
		lee_parametros_com_del_panel();//Lee los par�metros del puerto por si los ha cambiado el usuario
		sprintf(mensaje,"probando con el COM:%u con baudrate=%u %s",puerto,VelocidadPuerto,"�Ignora los errores de timeout!");
		SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,mensaje);	
		ErrorRS232 = abrir_puerto (puerto, NombrePuertoCOM, VelocidadPuerto, Paridad,BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida);
		sprintf(VersionDelInstrumento,"el puerto no ha abierto");
		if (ErrorRS232>=0) // Si el puerto est� presente en el equipo y abre... 
		{
			FlagPuertoAbierto = 1; // Indica al sistema que el puerto est� abierto para poder transmitir
			//Termina de configurar el puerto
			SetXMode (puerto,ModoX);
            SetCTSMode (puerto, ModoCTS);
            SetComTime (puerto, Timeout);
	        FlushInQ (puerto);//Limpia las  colas de entada y salida
    	    FlushOutQ (puerto);                                                                                                            
			identifica_al_sistema();//Identifica el sistema en IDNinstrumento
			identifica_al_sistema();//Identifica el sistema en IDNinstrumento
			status = identifica_al_sistema();//Lee la identidad del sistema
			if(status<0) //Si hubo problemas al escribir en el puerto cierra y sale
			{
				 //MessagePopup ("error RS232","no se ha podido \nescribir en el puerto");
				sprintf(VersionDelInstrumento,"no hay una base en el puerto");
				cerrar_puerto (puerto);
				FlagPuertoAbierto=0;
				SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,VersionDelInstrumento);	
				return 0;
			}
			SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,IDNinstrumento);	
			if(!strcmp (IDNinstrumento, IdentificacionDelInstrumento))//Si son iguales devuelve 0
			{  
				//Pregunta a la base por la versi�n 
				sprintf(VersionDelInstrumento,"Base SPM versi�n desconocida");
				sprintf(CadenaComando,"%s","MOT:VER?");                                                                                                       
				pcom_enviar_datos( CadenaComando,MOSTRAR);
				Delay(Retardo);
				if(pcom_recibir_datos(VersionDelInstrumento,1)>=0)
				SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,VersionDelInstrumento);	
				//Sale dejando el puerto abierto oficialmente
				FlagPuertoAbierto=1;
				PuertoCOM=puerto;
				pcom_recepcion_automatica(SI);//Hay que activarlo al haber cerrado el puerto
				sprintf(CadenaComando,"%s","*CLS");//Limpia la pila de errores                                                                                                       
				ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)                                                                                                     
				SetActiveCtrl(ConfigHandle,CONFIG_QUITCONFIG);//Lleva el control al bot�n de salir
				//HidePanel (ConfigHandle);
				return 0;
			}
			else
			{
				// MessagePopup ("�FRACASO!", "No hay Base");
				SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,"el puerto ha abierto pero no hay una Base SPM conectada");	
			}
			cerrar_puerto (puerto); // Si no est� el sistema en el puerto lo cerramos
			FlagPuertoAbierto=0;
		}
		//Si el puerto no abre salimos avisando 
		else SetCtrlVal (ConfigHandle, CONFIG_IDENTIFICACION,"el puerto no ha abierto");	
		
		EnableBreakOnLibraryErrors (); // Habilita errores en tiempo de ejecuci�n
	}
	
	return 0;
}
/*****************************************************************************************************                                 
						FUNCION QUE OCULTA EL PANEL DE CONFIGURACI�N
******************************************************************************************************/                                
int CVICALLBACK cerrar_configuracion (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT)
	{
		HidePanel (panel);
	}
	return 0;
}
/*******************************************************************************************************
	funci�n para cargar el panel de los datos del puerto que queremos guardar en el fichero
********************************************************************************************************/
int CVICALLBACK fichero_com (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	#define TOP_LEVEL 0	//0 para que sea top level 1 para que no lo sea
	if(event==EVENT_COMMIT)
	{
		pcom_fichero_com();
	}
	return 0;
}
/*******************************************************************************************************
	funci�n para cargar el panel de los datos del puerto que queremos guardar en el fichero
********************************************************************************************************/
void pcom_fichero_com(void)
{
	//Antes de mostrar el panel tiene que ver si est� en memoria. Si no est� lo carga.

	
	
	if(FlagPuertoAbierto && PuertoCOM!=-1) 
	{
		identifica_al_sistema();
		if(!strcmp (IDNinstrumento, IdentificacionDelInstrumento))
		{
			if (! ConfirmPopup ("BaseSPM", "�Hay un puerto abierto\ny una base conectada!\n      �Quiere continuar?")) 
			return;
		}
			
	}

	if (!ConfirmPopup ("Base SPM",MENSAJE_AYUDA_FICHERO)) return;

	if(!FicheroHandle) FicheroHandle = LoadPanel (TOP_LEVEL, "PanelPuertoCOM.uir", FICHERO);
	InstallPopup (FicheroHandle);
	SetCtrlVal (FicheroHandle, FICHERO_IDENTIFICACION,"Base SPM");	
	SetCtrlAttribute (FicheroHandle, FICHERO_ABRIRCOM, ATTR_DIMMED,1);

	
	
	//SetPanelAttribute (FicheroHandle, ATTR_TOP,Top);
	//SetPanelAttribute (FicheroHandle, ATTR_LEFT,Left);
	
}
/*******************************************************************************************************
	funci�n para leer el panel de los datos del puerto que queremos guardar en el fichero
********************************************************************************************************/
int CVICALLBACK guardar_en_fichero (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
struct strPuertoCOM Datos;
if (event== EVENT_COMMIT)
{
	FILE *ptrFichero;
	char* DirAppData;//Path de %appdata%
	char Path[128];//Path del fichero
	DirAppData = getenv("APPDATA"); //Si falla retorna NULL	

	if(ConfirmPopup ("BaseSPM.dat", "Se va a crear/sobreescribir el fichero\n       �Quiere continuar?"))
	{		
		if(DirAppData==NULL) 
		{
			MessagePopup ("Error BaseSPM.dat", "No existe el directorio appdata");
			return -1;//Si no est� el directorio sale con error
		}
		sprintf(Path,"%s%s",DirAppData,"\\BaseSPM.dat");		
		ptrFichero=fopen(Path,"wb");// Crea el fichero
		if (!ptrFichero) 
		{
			MessagePopup ("Error BaseSPM.dat", "No se ha creado el fichero");
		
			return -1;//Si no habre sale con error
		}
		//Si abre el fichero rellena la estructura con los datos que queremos guardar
		strcpy(Datos.NombrePuertoCOM,"");//No es relevante
		GetCtrlVal (FicheroHandle, FICHERO_IDENTIFICACION,Datos.IDNinstrumento);	
		GetCtrlVal (FicheroHandle, FICHERO_PUERTOCOM, &Datos.PuertoCOM);//Este es el puerto que queremos utilizar	
		GetCtrlVal (FicheroHandle, FICHERO_BAUDIOS, &Datos.VelocidadPuerto);                                  	
		GetCtrlVal (FicheroHandle, FICHERO_PARIDAD, &Datos.Paridad);                                                  	
		GetCtrlVal (FicheroHandle, FICHERO_BITDATOS, &Datos.BitsDatos);                            			          	
		GetCtrlVal (FicheroHandle, FICHERO_BITSTOP, &Datos.BitsDeStop);                            	                  	
		GetCtrlVal (FicheroHandle, FICHERO_COLAENTRADA, &Datos.ColaDeEntrada);                     			          	
		GetCtrlVal (FicheroHandle, FICHERO_COLASALIDA, &Datos.ColaDeSalida);                       			          	
		GetCtrlVal (FicheroHandle, FICHERO_MODOXONOFF, &Datos.ModoX);                                                 	
		GetCtrlVal (FicheroHandle, FICHERO_MODOCTS, &Datos.ModoCTS);                                                  		
		GetCtrlVal (FicheroHandle, FICHERO_TIMEOUT, &Datos.Timeout);                                                  	
		GetCtrlIndex (FicheroHandle, FICHERO_TERMINADORESCRITURA, &Datos.TerminadorTransmision);                      	
		GetCtrlIndex (FicheroHandle, FICHERO_TERMINADORLECTURA, &Datos.TerminadorRecepcion);                          	
		GetCtrlVal (FicheroHandle, FICHERO_RETARDO, &Datos.Retardo);                                                  	
		// Si abre el fichero sin errores..guarda la estructura en el fichero.
		fwrite(&Datos,sizeof(struct strPuertoCOM),1,ptrFichero);
		fclose(ptrFichero);//Cierra el fichero2
		MessagePopup ("BaseSPM.dat", "fichero creado correctamente");
		SetCtrlAttribute (FicheroHandle, FICHERO_ABRIRCOM, ATTR_DIMMED,0);
		
	}

}
	return 0;
}
/*******************************************************************************************************
	funci�n para abrir el puerto con los datos del fichero
********************************************************************************************************/

int CVICALLBACK aplicar_fichero (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT)
	{
		pcom_abre_puerto_serie();
		if(!FlagPuertoAbierto)  MessagePopup ("Error Puerto COM", "No se ha abierto el puerto");
		else 
		{
			HidePanel (panel);
			DiscardPanel (FicheroHandle);
			FicheroHandle=0;
		}
	}
	return 0;
}
/*******************************************************************************************************
	funci�n para salir del panel de los datos del puerto que queremos guardar en el fichero
********************************************************************************************************/
int CVICALLBACK panel_fichero (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	if (event == EVENT_CLOSE)
	{
		HidePanel (panel);
		DiscardPanel (FicheroHandle);
		FicheroHandle=0;
	}
	return 0;
}
/*******************************************************************************************************
	funci�n para resetear el panel de los datos del puerto que queremos guardar en el fichero
********************************************************************************************************/
int CVICALLBACK reset_fichero (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT)
	{
	SetCtrlVal (FicheroHandle, FICHERO_IDENTIFICACION,"Base SPM");	
	SetCtrlVal (FicheroHandle, FICHERO_PUERTOCOM, 0);//Este es el puerto que queremos utilizar	
	SetCtrlVal (FicheroHandle, FICHERO_BAUDIOS, 115200);                                  	
	SetCtrlIndex (FicheroHandle, FICHERO_PARIDAD, 0);                                                  	
	SetCtrlIndex (FicheroHandle, FICHERO_BITDATOS, 3);                            			          	
	SetCtrlIndex (FicheroHandle, FICHERO_BITSTOP, 1);                            	                  	
	SetCtrlVal (FicheroHandle, FICHERO_COLAENTRADA, 512);                     			          	
	SetCtrlVal (FicheroHandle, FICHERO_COLASALIDA, 512);                       			          	
	SetCtrlVal (FicheroHandle, FICHERO_MODOXONOFF, 0);                                                 	
	SetCtrlVal (FicheroHandle, FICHERO_MODOCTS, 0);                                                  		
	SetCtrlVal (FicheroHandle, FICHERO_TIMEOUT, 1.0);                                                  	
	SetCtrlIndex (FicheroHandle, FICHERO_TERMINADORESCRITURA,1);                      	
	SetCtrlIndex (FicheroHandle, FICHERO_TERMINADORLECTURA,1);                          	
	SetCtrlVal (FicheroHandle, FICHERO_RETARDO, 0.2);                                                  	
		
	}
	return 0;
}
/*******************************************************************************************************
	funci�n para explicar el panel de los datos del puerto que queremos guardar en el fichero
********************************************************************************************************/
int CVICALLBACK ayuda_guardar_en_fichero (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event== EVENT_COMMIT)
	{
		MessagePopup ("Fichero BaseSPM.dat",MENSAJE_AYUDA_FICHERO);
	}
	return 0;
}
/*****************************************************************************************************                                 
						FUNCION QUE MUESTRA INFORMACION DEL AUTOR
******************************************************************************************************/                                

void CVICALLBACK autor (int menuBar, int menuItem, void *callbackData,
		int panel)                                                    
{                                                                     
	 MessagePopup ("Base SPM",MENSAJE_ABOUT_BASE_SPM);
	 
}                                                                     
/******************************************************************************************************/

