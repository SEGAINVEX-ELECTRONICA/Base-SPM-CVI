/**************************************************************************
	PROYECTO: Base SPM
	FICHERO: BaseSPM.c                                                   
	
	Por Patricio Coronado Collado.12/04/2020
	Git: Guardado en branch:master
	
	Para comunicar con el puerto COM se utilizan funciones y variables 
	exportadas de PuertoCom.h y LocateCOMCVI.h
	Icono By Everaldo Coelho (YellowIcon); - All Crystal icons were 
	posted by the author as LGPL on kde-look;, LGPL, 
	https://commons.wikimedia.org/w/index.php?curid=569461
	Library for reading system ports COM: https://forums.ni.com  (msaxo)
***************************************************************************/
																								
/**************************************************************************
	Copyright © 2020 Patricio Coronado
	
	This file is part of Base SPM

    BaseSPM.c is free software: you  can redistribute  it and/or modify 
	itunder the terms of the GNU General Public License as published by
	the Free Software Foundation, either version  3 of  the License, or
	(at your option) any later version.
	
    Base SPM  is distributed in the  hope that it will be 
	useful,but WITHOUT ANY WARRANTY ; without even the implied warranty 
	of MERCHANTABILITY or FITNESS FOR A PARTICULARPURPOSE.  See the GNU 
	General Public License for more details.

    You  should have received a copy  of the GNU General Public License
    along with BaseSPM.c  If not, see <http://www.gnu.org/licenses/>

***************************************************************************/	
/*
	Esta cadena es la respuesta del sistema que queremos controlar al
	recibir un comando SCPI *IDN. Obligatorio rellenar la cadena 
	IdentificacionDelInstrumento que debe conincidir con
	NOMBRE_DEL_SISTEMA_64B(identifiacion del instrumento) en la
	aplicación del microprocesador.
*/
char const IdentificacionDelInstrumento[]="Base SPM";
char VersionDelInstrumento[64];//Aquí se lee la versión de la base
/**************************************************************************
 Ficheros include                                                          
***************************************************************************/
#include <ansi_c.h>
#include <cvirte.h>
#include <userint.h>
#include <rs232.h>
#include <utility.h>
#include <formatio.h>
#include <string.h>
#include "PanelPuertoCom.h"
#include <analysis.h>
#include "PuertoCOM.h"
#include "BaseSPM.h"
/**************************************************************************
 Constantes
***************************************************************************/
#define PP_TOP  60	  
#define PP_LEFT 60	
#define MARCHA 1
#define PARO 0
#define MODOCOMPUTADORA 1
#define MODOMANDO 0
#define TAB0 0
#define TAB1 1
#define TAB2 2
#define TAB3 3
#define NO 0
#define SI 1
#define PEDIR_DATO -1
#define  MENSAJE_ERROR "\n\
No se ha encontado la  base en el \n\
último puerto  COM  utilizado. Si \n\
sabes en que  puerto COM está bús-\n\
cala  manualmento anotandolo  en:\n\n\
(puerto COM seleccionado)\n\n\
Asegurate de  que la configuración\n\
del puerto es la correcta y pulsa:\n\n\
(buscar Base SPM)\n\n\
Si no encuentras la base en ese pu-\n\
erto pero estás seguro de  que está\n\
en ese puerto,pulsa\n\n\
(Aplicar)\n"

 
#define MENSAJE_ABOUT_BASE_SPM "\n\
-----------------------------------------------------------------------\n\
Copyright © 2020 Patricio Coronado\n\n\
Universidad Autónoma de Madrid-SEGAINVEX-Electrónica.\n\n\
This file is part of Base SPM\n\
BaseSPM.c is free software: you  can redistribute  it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version  3 of  the License, or\n\
(at your option) any later version.\n\
BaseSPM  is  distributed in the  hope  that it  will be\n\
useful,but WITHOUT ANY WARRANTY; without even the implied\n\
 warranty  of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n\
 PURPOSE. See the GNU General Public License for more details.\n\
You  should have received a copy  of the GNU General Public License\n\
along with BaseSPM.c If not, see <http://www.gnu.org/licenses/>\n\n\
Icono By Everaldo Coelho (YellowIcon)\n\n\
Library for reading system ports COM: https://forums.ni.com  (msaxo)\n\n\
Support: https://github.com/PatricioCoronado/BaseSPM_CVI\n\
------------------------------------------------------------------------\n"
/**************************************************************************
 Prototipos de funciones
***************************************************************************/
int carga_panel_del_mando(void);
void desactiva_controles_tabs(void);
void desactiva_controles_tab0(int,int);
void desactiva_controles_tab1(int,int);
void desactiva_controles_tab2(int,int);
void desactiva_controles_tab3(int,int);
void cambia_estado_marcha_paro(int,int,int);
void envia_comando_temperatura_humedad(void);
void envia_comando_fotodiodo(void);
void envia_comando_motor(int);
void envia_comando_sentido(int);
void envia_comando_frecuencia(int);
void envia_comando_resolucion(int);
void envia_comando_onda(int);
void envia_comando_pasos(int);
void envia_comando_marcha_paro(int);
void pedir_estado_actual(void);
void reset(void);
void asigna_tabHandles(void);
void actualiza_controles_main(void);
int test_compatibilidad_frecuencia_resolucion(int, int);
void filtro_acelerometro(float,float);
/**************************************************************************
 tipos
***************************************************************************/
/**************************************************************************
 Variables
***************************************************************************/
int MainHandle=0;
//Handles de los tabs
int TabInactivo;
int tab0Handle=0;
int tab1Handle=0;
int tab2Handle=0;
int tab3Handle=0;
int FotodiodoAuto = NO;//Indica si las señales del fotodiodo se piden automáticamente
int ThAuto = NO;//Indica si latemperatura humedad  se piden automáticamente
int MarchaParoAuto = NO;//Para lectura automática de si el sistema está en marcha
int PasosAuto=NO;//Para lectura automática del número de pasos que queda por dar
int AcelerometroAuto=NO;//Para lectura automática del acelerómetro
int AcelerometroRapido=NO;
int FotodiodoRapido=NO;
int EstadoMarchaParo=PARO; 
int BaseActiva=NO;
int CabezaActiva=NO;
int ModoContinuo=0;
int ModoMandoComputadora = MODOCOMPUTADORA; //COMPUTADORA o MANDO
int TabActivo;
int led=0;
int Estado48V=0;//Para saber si los 48V están activados o no
double Barra=0; //Barra de movimiento
int MotorActivo,Resolucion,Frecuencia,Sentido,Pasos;//Para leer el estado real de la base
//Filtro 
static float x1n=0, y1n=0,x1n_4=0, x1n_3=0, x1n_2=0, x1n_1=0;
static float x2n=0, y2n=0,x2n_4=0, x2n_3=0, x2n_2=0, x2n_1=0;

/*********************************************************************************************
		                   función main()
**********************************************************************************************/
 int main (int argc, char *argv[])
{
	char mensaje[512];
	int status;
	if (InitCVIRTE (0, argv, 0) == 0) return -1;
	FlagPuertoAbierto=0; //Empieza suponiendo que no está abierto el puerto
	pcom_modo_pcom(1);//Para poner el panel de configuración en modo master
	if(pcom_abre_puerto_serie()==-1) 
	{
		
		pcom_fichero_com();
	
		//sprintf(mensaje,"%s",MENSAJE_ERROR);
		//MessagePopup("Error:no se ha enontado la base",mensaje);
	}
	
	//Carga panel principal
	MainHandle=LoadPanel (0, "BaseSPM.uir", MAIN); // 0 significa TOP LEVEL 
	//Pide la versión de la base
	asigna_tabHandles();//Encuentra los handles de los tabs
	SetActiveTabPage (MainHandle, MAIN_TAB, TAB0);
	//Borra la pila de errores de la base
	DisplayPanel(MainHandle);	
	pcom_muestra_el_panel_de_mensajes(PP_TOP,PP_LEFT);
	if(FlagPuertoAbierto) 
	{
		sprintf(VersionDelInstrumento,"Base SPM versión desconocida");
		sprintf(CadenaComando,"%s" ,"MOT:VER?");
	    pcom_enviar_datos(CadenaComando,1);
	}
	//Limpa la pila de errores de la base
	sprintf(CadenaComando,"%s","*CLS");
	pcom_enviar_datos(CadenaComando,NO_MOSTRAR);
	//Inicia el timer
	SetCtrlAttribute(MainHandle,MAIN_TIMER,ATTR_ENABLED ,FlagPuertoAbierto);
	RunUserInterface(); 
	return 0;
}	
/**********************************************************************************************
							función del panel main
***********************************************************************************************/
int CVICALLBACK panel_main (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:
			actualiza_controles_main();
		break;
		//case EVENT_LOST_FOCUS:
		//break;
		case EVENT_CLOSE: //Para salir
			envia_comando_marcha_paro(PARO);//Para motores
			Delay(0.1);
			sprintf(CadenaComando,"MOT:FNA");//Para el acelerómetro
			pcom_enviar_datos(CadenaComando,1);
			Delay(0.1);
			sprintf(CadenaComando,"MOT:FIF");//Para el fotodiodo
			pcom_enviar_datos(CadenaComando,1);
			HidePanel(MainHandle);//Quita paneles
			if(MainHandle) {DiscardPanel (MainHandle);MainHandle=0;}
			pcom_cierra_puerto_serie(QUITAR_PANELES);//Cierra el puerto y sus paneles sus paneles
			QuitUserInterface (0); //cierra la aplicación
		break;
	}
	return 0;
}
/***********************************************************************************************
	FUNCION QUE DIMA O DESDIMA LOS CONTROLES SEGUN ESTE EL PUERTO ABIERTO O NO
************************************************************************************************/
void actualiza_controles_main(void)
{
	int arrayHandle;
	arrayHandle=GetCtrlArrayFromResourceID ( MainHandle, mainControlsArray); //Handle de todos los controles de MAIN
	
	SetCtrlAttribute(MainHandle,MAIN_TIMER,ATTR_ENABLED ,FlagPuertoAbierto);
	SetPanelAttribute(MainHandle, ATTR_TITLE, VersionDelInstrumento);
	SetCtrlArrayAttribute (arrayHandle, ATTR_DIMMED, !FlagPuertoAbierto);//Desactiva todos los checks del main
	/*
	if(AcelerometroRapido || FotodiodoRapido)//Si estamos en modo datos rápido dimamos los checks
	{
		arrayHandle=GetCtrlArrayFromResourceID ( MainHandle, mainChecksArray); //Handle del array de buttons
		SetCtrlArrayAttribute (arrayHandle, ATTR_DIMMED, 1);//Desactiva todos los checks del main
	}*/
}
/**********************************************************************************************
	funcion que atiende a la pulsación de botones en el tab2 (base)
***********************************************************************************************/
int CVICALLBACK tab2_toggle_button (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int estado,FrecuenciaAjustada;
	if(event == EVENT_COMMIT)
	{
		GetCtrlVal (panel, control, &estado); //Lee el toggle button pulsado
		//Hace que se vea como pulsado solo el button pulsado y el resto no pulsados.
		desactiva_controles_tab2(control, estado);
		if(estado) //Si se ha pulsado un botón...........
		{
			switch (control)
			{
				case TAB2_Z1_UP:
					MotorActivo = 1;
					Sentido= 1;
				break;
				case TAB2_Z1_DOWN:
					MotorActivo = 1;
					Sentido = 0;
				break;
				case TAB2_Z2_UP:
					MotorActivo = 2;
					Sentido = 1;
				break;
				case TAB2_Z2_DOWN:
					MotorActivo = 2;
					Sentido = 0;
				break;
				case TAB2_Z3_UP:
					MotorActivo = 3;
					Sentido = 1;
				break;
				case TAB2_Z3_DOWN:
					MotorActivo = 3;
					Sentido = 0;
				break;
				case TAB2_Z123_UP:
					MotorActivo = 7;
					Sentido = 1;
				break;
				case TAB2_Z123_DOWN:
					MotorActivo = 7;
					Sentido = 0;
				break;	
			}//Switch		
			BaseActiva=NO;
			CabezaActiva=SI;
			GetCtrlVal(panel,TAB2_FRECUENCIA,&Frecuencia);
			GetCtrlVal(panel,TAB2_RESOLUCION,&Resolucion);
			if(FrecuenciaAjustada=test_compatibilidad_frecuencia_resolucion(Resolucion,Frecuencia))
			{
				Frecuencia=FrecuenciaAjustada;
				SetCtrlVal(panel,TAB2_FRECUENCIA,Frecuencia);	
			}
			sprintf(CadenaComando,"%s %u %u %u %u","MOT:MM ", MotorActivo , Resolucion ,Frecuencia, Sentido);
			pcom_enviar_datos(CadenaComando,1);
			// Anuncia que el sistema está en marcha
			cambia_estado_marcha_paro(0,0,MARCHA);
		}//if estado
		else // Si se ha despulsado un botón
		{
			sprintf(CadenaComando,"%s","MOT:MP 0");
			pcom_enviar_datos(CadenaComando,1);
			cambia_estado_marcha_paro(0,0,PARO);	
		}

	}//if event
	return 0;
}
/**********************************************************************************************
	funcion que atiende a la pulsación de botones en el tab3 (cabeza)
***********************************************************************************************/
int CVICALLBACK tab3_toggle_button (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int estado,FrecuenciaAjustada;
	if(event == EVENT_COMMIT)
	{
		GetCtrlVal (panel, control, &estado); //Lee el toggle button pulsado
		//Hace que se vea como pulsado solo el button pulsado y el resto no pulsados.
		desactiva_controles_tab3(control, estado);
		if(estado) //Si se ha pulsado un botón...........
		{
			switch (control)
			{
				case TAB3_LASER_RIGHT:
				MotorActivo = 11;
				Sentido = 0;
			break;
			case TAB3_LASER_LEFT:
				MotorActivo = 11;
				Sentido = 1;
			break;
				case TAB3_LASER_UP:
				MotorActivo = 12;
				Sentido = 0;
			break;
			case TAB3_LASER_DOWN:
				MotorActivo = 12;
				Sentido = 1;
			break;
			case TAB3_FOTODIODO_UP:
				MotorActivo = 13;
				Sentido = 1;
				break;
			case TAB3_FOTODIODO_DOWN:			
				MotorActivo = 13;
				Sentido = 0;
			break;
			case TAB3_FOTODIODO_RIGHT:
				MotorActivo = 10;
				Sentido = 0;
				break;
			case TAB3_FOTODIODO_LEFT:
				MotorActivo = 10;
				Sentido = 1;
			break;
			}//Switch		
			BaseActiva=SI;
			CabezaActiva=NO;
			GetCtrlVal(panel,TAB3_FRECUENCIA,&Frecuencia);
			GetCtrlVal(panel,TAB3_RESOLUCION,&Resolucion);
			if(FrecuenciaAjustada=test_compatibilidad_frecuencia_resolucion(Resolucion,Frecuencia))
			{
				Frecuencia=FrecuenciaAjustada;
				SetCtrlVal(panel,TAB2_FRECUENCIA,Frecuencia);	
			}
			sprintf(CadenaComando,"%s %u %u %u %u","MOT:MM ", MotorActivo , Resolucion ,Frecuencia, Sentido);
			pcom_enviar_datos(CadenaComando,1);
			// Anuncia que el sistema está en marcha
			cambia_estado_marcha_paro(0,0,MARCHA);
		}//if estado
		else // Si se ha despulsado un botón
		{
			sprintf(CadenaComando,"%s","MOT:MP 0");
			pcom_enviar_datos(CadenaComando,1);
			cambia_estado_marcha_paro(0,0,PARO);	
		}

	}//if event
	return 0;
}
/***********************************************************************************************
				función que atiende a la pulsación de un toggle button DEL tab0	   
************************************************************************************************/
int CVICALLBACK toggle_button (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int estado,FrecuenciaAjustada;
	if(event==EVENT_COMMIT)
	{
	GetCtrlVal (panel, control, &estado); //Lee el toggle button pulsado
	desactiva_controles_tab0(control,estado); //Desactiva los demás
	if(estado)
	{
		switch (control)
		{
			case TAB0_Z1_UP:
				MotorActivo = 1;
				Sentido= 1;
			break;
			case TAB0_Z1_DOWN:
				MotorActivo = 1;
				Sentido = 0;
			break;
			case TAB0_Z2_UP:
				MotorActivo = 2;
				Sentido = 1;
			break;
			case TAB0_Z2_DOWN:
				MotorActivo = 2;
				Sentido = 0;
			break;
			case TAB0_Z3_UP:
				MotorActivo = 3;
				Sentido = 1;
			break;
			case TAB0_Z3_DOWN:
				MotorActivo = 3;
				Sentido = 0;
			break;
			case TAB0_Z123_UP:
				MotorActivo = 7;
				Sentido = 1;
			break;
			case TAB0_Z123_DOWN:
				MotorActivo = 7;
				Sentido = 0;
			break;	
			case TAB0_LASER_RIGHT:
				MotorActivo = 11;
				Sentido = 0;
			break;
			case TAB0_LASER_LEFT:
				MotorActivo = 11;
				Sentido = 1;
			break;
				case TAB0_LASER_UP:
				MotorActivo = 12;
				Sentido = 0;
			break;
			case TAB0_LASER_DOWN:
				MotorActivo = 12;
				Sentido = 1;
			break;
			case TAB0_FOTODIODO_UP:
				MotorActivo = 13;
				Sentido = 1;
				break;
			case TAB0_FOTODIODO_DOWN:			
				MotorActivo = 13;
				Sentido = 0;
			break;
			case TAB0_FOTODIODO_RIGHT:
				MotorActivo = 10;
				Sentido = 0;
				break;
			case TAB0_FOTODIODO_LEFT:
				MotorActivo = 10;
				Sentido = 1;
			break;
		}//switch
		switch(MotorActivo)//Determina si está activa la base o la cabeza
		{
			case  10: case 11: case 12: case 13:
				BaseActiva=NO;
				CabezaActiva=SI;
			break;
		
			case  1: case 2: case 3:case 4: case 5: case 6: case 7:
				
				BaseActiva=SI;
				CabezaActiva=NO;
			break;
			case 0:
				BaseActiva=NO;
				CabezaActiva=NO;
			break;
		}
		//Envía el comando de marcha
		if(ModoMandoComputadora==MODOCOMPUTADORA)//Si está en modo  computadora
		{
			GetCtrlVal(panel,TAB0_FRECUENCIA,&Frecuencia);
			GetCtrlVal(panel,TAB0_RESOLUCION,&Resolucion);
			if(FrecuenciaAjustada=test_compatibilidad_frecuencia_resolucion(Resolucion,Frecuencia))
			{
				Frecuencia=FrecuenciaAjustada;
				SetCtrlVal(panel,TAB0_FRECUENCIA,Frecuencia);	
			}
			sprintf(CadenaComando,"%s %u %u %u %u","MOT:MM ", MotorActivo , Resolucion ,Frecuencia, Sentido);
		}
		if(ModoMandoComputadora==MODOMANDO)//Si está en modo  mando
		{
			//Selecciona la frecuencia correcta
			GetCtrlVal(panel,TAB0_VELOCIDAD,&Frecuencia);
			switch (Frecuencia)
			{
				case 1:
					Frecuencia=1;
				break;
				case 2:
					Frecuencia=10;
				break;
				case 3:
					Frecuencia=30;
				break;
				case 4:
					Frecuencia=45;
				break;
				case 5:
					Frecuencia=60;
				break;
				default:
					Frecuencia=1;	
				break;
			}
			
			GetCtrlVal(panel,TAB0_PASOS,&Pasos);
			sprintf(CadenaComando,"%s %u %u %u %u","BTH:MARCHA ", MotorActivo , Frecuencia, Sentido, Pasos);
			//Una vez enviados los pasos pone el control a 0
			Pasos=0;
			GetCtrlVal(panel,TAB0_PASOS,&Pasos);
		}
		//Envía el comando
		pcom_enviar_datos(CadenaComando,1);
		// Anuncia que el sistema está en marcha
		cambia_estado_marcha_paro(0,0,MARCHA);
	}//if(estado)
	else // Si se ha soltado una tecla se envia el comando de paro 
	{
		if(ModoMandoComputadora==MODOCOMPUTADORA)
			sprintf(CadenaComando,"%s","MOT:MP 0");
		if(ModoMandoComputadora==MODOMANDO)
			sprintf(CadenaComando,"%s","BTH:PARO 0");	
		pcom_enviar_datos(CadenaComando,1);
		cambia_estado_marcha_paro(0,0,PARO);	
	}	
	}//if(event==EVENT_COMMIT)
return 0;
}
/************************************************************************************************
					función para hacer un paro de emergencia
************************************************************************************************/
int CVICALLBACK paro_emergencia (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event == EVENT_COMMIT)
	{
			sprintf(CadenaComando,"%s","MOT:MP 0");
			pcom_enviar_datos(CadenaComando,1);
			cambia_estado_marcha_paro(0,0,PARO);
			desactiva_controles_tabs();
	}
	return 0;
}
/************************************************************************************************
					función para pedir el contador a la base
	El contador se pone a cero cada vez que un motor se pone en marcha y cuenta los pasos
************************************************************************************************/
int CVICALLBACK leer_contador (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT)
	{
		sprintf(CadenaComando,"%s","MOT:CO ?");
		pcom_enviar_datos(CadenaComando,1);
	}
	return 0;
}
/************************************************************************************************
					función para poner el contador a cero
	El contador se pone a cero cada vez que un motor se pone en marcha y cuenta los pasos
************************************************************************************************/

int CVICALLBACK reset_contador (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int Check48V=0;
	if (event == EVENT_COMMIT)
	{
		sprintf(CadenaComando,"%s","MOT:CO 0");
		pcom_enviar_datos(CadenaComando,1);
	}
	
	if(event==EVENT_RIGHT_DOUBLE_CLICK)//ActiVa el check de los 48V
	{
		if(Check48V)
			Check48V=0;
		else 
			Check48V=1;
		SetCtrlAttribute(MainHandle,MAIN_CHECK48V,ATTR_VISIBLE,Check48V);
	}
	return 0;
}

/************************************************************************************************
		función para habilitar temperatura y humedad se actualice automáticamente cada 5' 
************************************************************************************************/
int CVICALLBACK temperatura_humedad_auto (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT){GetCtrlVal ( panel, control, &ThAuto);}return 0;
	return 0;
}
/************************************************************************************************
		función para habilitar que los valores del acelerómetro se actualicen automáticamente
************************************************************************************************/
int CVICALLBACK acelerometro_auto (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
		if(event==EVENT_COMMIT){GetCtrlVal ( panel, control, &AcelerometroAuto);}return 0;
}	
/************************************************************************************************
		función para habilitar que las señales del fotodiodo se actualicen automáticamente
************************************************************************************************/
int CVICALLBACK fotodiodo_auto (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT)
	GetCtrlVal ( panel, control, &FotodiodoAuto);
	return 0;
}
/************************************************************************************************
		función para habilitar la recepción de pasos periodicamente
************************************************************************************************/
int CVICALLBACK pasos_auto (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT){GetCtrlVal ( panel, control, &PasosAuto);}return 0;
}
/************************************************************************************************
		función para habilitar la recepción períodica del estado de marcha
************************************************************************************************/
int CVICALLBACK marchaparo_auto (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{if(event == EVENT_COMMIT){GetCtrlVal ( panel, control, &MarchaParoAuto);}return 0;}

/**********************************************************************************************
		Función que ajusta la frecuencia en función de la resolución
		¿Como se actua? se lee el control de resolución y luego el de frecuencia y se 
		llama a la función. Si no devuelve 0 frecuencia cambia su valor por lo devuelto.
***********************************************************************************************/
int test_compatibilidad_frecuencia_resolucion(int Res, int Frec)
{
	switch(Res)
	{
		case 256: //Con esta resolución la frecuencia no puede ser mayor de 60KHz
			if(Frec>60)
			{
				Frec = 60;
				return Frec;
			}
		break;
		case 512: //Con esta resolución la frecuencia no puede ser mayor de 120KHz
			if(Frec>120)
			{
				Frec = 120;
				return Frec;
			}
		break;
	}
	return 0;
}
/*********************************************************************************************
			Función que marca el tab activo. Se ejecuta cuando cambias el tab 
			Se para motor y se actualizan los botones de los tabs
**********************************************************************************************/
int CVICALLBACK cambia_tab (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int tabHandle,TabIndex,arrayHandle;
	int index;
	
	/*
	if(event==EVENT_MOUSE_POINTER_MOVE)
	{
		actualiza_controles_main();
	}
	*/
	if(event==EVENT_ACTIVE_TAB_CHANGE)
	{
		pcom_borra_cola_salida(0,0,0,0,0,0);//Limpia la cola de salida del puerto serie
		pcom_borra_cola_entrada(0,0,0,0,0,0);//Limpia la cola de entrada del puerto serie
		//Para motor
		envia_comando_marcha_paro(PARO);//Al cambiar de Tab paramos motor
		cambia_estado_marcha_paro(panel, control,PARO);
		arrayHandle=GetCtrlArrayFromResourceID ( MainHandle, mainChecksArray); //Handle del array de buttons
		SetCtrlArrayAttribute (arrayHandle, ATTR_DIMMED, 0);//Activa controles del panel MAIN que podrían estar dimados
		FotodiodoRapido=NO;
		sprintf(CadenaComando,"MOT:FNA");pcom_enviar_datos(CadenaComando,1);//Para acelerómetro rápido
		Delay(0.1);	
		AcelerometroRapido=NO;
		sprintf(CadenaComando,"MOT:FIF");pcom_enviar_datos(CadenaComando,1);//Para fotodiodo rápido
		Delay(0.1);	
		sprintf(CadenaComando,"MOT:VAR?");pcom_enviar_datos(CadenaComando,1); //Lee el estado del sistema	
		GetActiveTabPage (panel, control, &TabIndex);//Averiguo cual es el tabIndex
		TabActivo=TabIndex;//Nuevo tab
		//Acciones a realizar en función del nuevo tab
		desactiva_controles_tabs();
		switch (TabActivo)
		{
			case TAB0:
			break;
			case TAB1:
			break;
			case TAB2:
			break;
			case TAB3:
			break;
			default:
			break;
		}
	}	
	return 0;
}
/**********************************************************************************************
			función que encuentra los tabHandle de los tab 0, 1 y 2.
***********************************************************************************************/
void asigna_tabHandles(void)
{
	GetPanelHandleFromTabPage (MainHandle,MAIN_TAB,TAB0,&tab0Handle);
	GetPanelHandleFromTabPage (MainHandle,MAIN_TAB,TAB1,&tab1Handle);
	GetPanelHandleFromTabPage (MainHandle,MAIN_TAB,TAB2,&tab2Handle);
	GetPanelHandleFromTabPage (MainHandle,MAIN_TAB,TAB3,&tab3Handle);
}
/***********************************************************************************************
		Función para cambiar entre modo mando y computadora
************************************************************************************************/
int CVICALLBACK cambia_modo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_VAL_CHANGED)
	{
		GetCtrlVal(panel, control, &ModoMandoComputadora);
	
		switch (ModoMandoComputadora)
		{
			case MODOMANDO://lo ponE en modo mando
				ModoMandoComputadora=MODOMANDO;
				SetCtrlAttribute(tab0Handle,TAB0_VELOCIDAD,ATTR_VISIBLE,1);
				SetCtrlAttribute(tab0Handle,TAB0_FRECUENCIA,ATTR_VISIBLE,0);
				SetCtrlAttribute(tab0Handle,TAB0_RESOLUCION,ATTR_VISIBLE,0);
				SetCtrlAttribute (tab0Handle,TAB0_ENVIARPASOS, ATTR_DIMMED, 1);
			break;
			case MODOCOMPUTADORA://lo pone en modo computadora
				ModoMandoComputadora=MODOCOMPUTADORA;
				SetCtrlAttribute(tab0Handle,TAB0_VELOCIDAD,ATTR_VISIBLE,0);
				SetCtrlAttribute(tab0Handle,TAB0_FRECUENCIA,ATTR_VISIBLE,1);
				SetCtrlAttribute(tab0Handle,TAB0_RESOLUCION,ATTR_VISIBLE,1);
				SetCtrlAttribute (tab0Handle,TAB0_ENVIARPASOS, ATTR_DIMMED, 0);
			break;
			default:
			break;
		}
	}
	return 0;
}
/***********************************************************************************************
		Función para cargar el panel que muestra los mensajes del puerto
************************************************************************************************/
void CVICALLBACK ver_trafico_puerto (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	pcom_muestra_el_panel_de_mensajes(PP_TOP,PP_LEFT);
}
/***********************************************************************************************
		Función para cargar el panel de configuración del puerto
************************************************************************************************/
void CVICALLBACK ver_configuracion_puerto (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	//Por precaución hay que parar motores
	envia_comando_marcha_paro(PARO);
	//Todos los paneles en modo paro
	cambia_estado_marcha_paro(0,0,PARO);
	desactiva_controles_tabs(); 
	SetCtrlAttribute (tab1Handle, TAB1_MARCHAPARO, ATTR_LABEL_TEXT, "pulsar para marcha");
	SetCtrlVal (tab1Handle, TAB1_MARCHAPARO,0);
	envia_comando_pasos(PEDIR_DATO);			
	pcom_muestra_el_panel_de_configuracion(PANEL_MODO_HIJO,PP_TOP,PP_LEFT);
}
/**********************************************************************************************
		Función para crear un fichero con los datos del puerto
***********************************************************************************************/
void CVICALLBACK crear_fichero_puerto_COM (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	pcom_fichero_com();
}

/**********************************************************************************************
		función que cambia el flag que indica si el sistema está en marcha EstadoMarchaParo
***********************************************************************************************/
void cambia_estado_marcha_paro(int panel, int control, int estado)
{
	if (estado ==MARCHA)
	{
		EstadoMarchaParo=MARCHA;
	}
	else 
	{
		EstadoMarchaParo=PARO;
		// resetea la barra de movimiento
		Barra=0.0;
		SetCtrlVal(MainHandle,MAIN_BARRA,0.0);
	}
}
/*********************************************************************************************
			Función que pide a la base las variables de estado
**********************************************************************************************/
void pedir_estado_actual(void)
{
	sprintf(CadenaComando,"MOT:VAR?");	
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
		funcion para pedir al pc la humedad y temperatura
***********************************************************************************************/
int CVICALLBACK recibir_ht (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{if(event==EVENT_COMMIT) envia_comando_temperatura_humedad(); return 0;}

/***********************************************************************************************
		función para enviar el comando que pide la temperatura y humedad a la base
************************************************************************************************/
void envia_comando_temperatura_humedad(void)
{
	sprintf(CadenaComando,"MOT:TH?");pcom_enviar_datos(CadenaComando,1);
}	
/***********************************************************************************************
		función para enviar el comando que las señales del fotodiodo
************************************************************************************************/
void envia_comando_fotodiodo(void)
{
	if(ModoMandoComputadora==MODOMANDO) sprintf(CadenaComando,"BTH:EST?");
	else sprintf(CadenaComando,"MOT:FOT?");	
	
	pcom_enviar_datos(CadenaComando,1);
}	
/**********************************************************************************************
				Funciones para programar un numero de pasos
***********************************************************************************************/
int CVICALLBACK cambiar_pasos (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{//
	int pasos;
	if(event==EVENT_COMMIT)
	{
		if((TabActivo == TAB0) && (ModoMandoComputadora == MODOCOMPUTADORA))
		{	
			GetCtrlVal(tab0Handle, TAB0_PASOS, &pasos);
			envia_comando_pasos(pasos);
			SetCtrlVal(tab0Handle, TAB0_PASOS, 0);
		}
		if(TabActivo == TAB1)
		{	
			GetCtrlVal(tab1Handle, TAB1_PASOS, &pasos);
			envia_comando_pasos(pasos);
			SetCtrlVal(tab1Handle, TAB1_PASOS, 0);
		}
		if(TabActivo == TAB2)
		{	
			GetCtrlVal(tab2Handle, TAB2_PASOS, &pasos);
			envia_comando_pasos(pasos);
			SetCtrlVal(tab2Handle, TAB2_PASOS, 0);
		}
		if(TabActivo == TAB3)
		{	
			GetCtrlVal(tab3Handle, TAB3_PASOS, &pasos);
			envia_comando_pasos(pasos);
			SetCtrlVal(tab3Handle, TAB3_PASOS, 0);
		}
		
	}
	return 0;
}
/**********************************************************************************************
			Función para pedir el numero de pasos que quedan por dar
***********************************************************************************************/
int CVICALLBACK leer_pasos (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	unsigned int valor;
		if (event== EVENT_COMMIT) {envia_comando_pasos(-1);}
	return 0;
}
/**********************************************************************************************
	Función que envía por el puerto el comando para programar un número de pasos
***********************************************************************************************/
void envia_comando_pasos(int pas)
{
	if (pas==PEDIR_DATO) 
		sprintf(CadenaComando,"%s","MOT:AN ?");//Pregunta por los pasos que quedan
	else 
		sprintf(CadenaComando,"%s %u","MOT:AN ",pas); //Programa el número de pasos
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
			función para pedir las señales del fotodiodo
***********************************************************************************************/
int CVICALLBACK recibir_fotodiodo (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{if(event==EVENT_COMMIT){envia_comando_fotodiodo();}return 0;}
/**********************************************************************************************
				función que responde al botón de marcha/paro 	
***********************************************************************************************/
int CVICALLBACK pc_marcha_paro (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT)
	{
		if (EstadoMarchaParo == PARO) // Lo pone en marcha
		{
			cambia_estado_marcha_paro(panel, control,MARCHA);
			SetCtrlAttribute (panel, control, ATTR_LABEL_TEXT, "pulsar para paro");
		}
		else
		{
			cambia_estado_marcha_paro(panel, control,PARO);
			SetCtrlAttribute (panel, control, ATTR_LABEL_TEXT, "pulsar para marcha");

		}	
		envia_comando_marcha_paro(EstadoMarchaParo);
		//envia_comando_marcha_paro(-1);
	}
	return 0;
}
/**********************************************************************************************
	Función que envía por el puerto el comando para cambiar el sentido
***********************************************************************************************/
void envia_comando_marcha_paro(int mp)
{
	if (mp==-1) sprintf(CadenaComando,"%s","MOT:MP?");//Pregunta por el motor activo
	
	else sprintf(CadenaComando,"%s %u","MOT:MP ",mp); //Programa el motor activo
	
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
				Función que atiende al control de motor activo
***********************************************************************************************/
int CVICALLBACK cambia_motor_activo (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//unsigned int valor;
	if(event== EVENT_VAL_CHANGED)
	{
		GetCtrlVal(panel,control, &MotorActivo);
		switch(MotorActivo)//Determina si está activa la base o la cabeza
		{
			case  10: case 11: case 12: case 13:
				BaseActiva=NO;
				CabezaActiva=SI;
			break;
		
			case  1: case 2: case 3:case 4: case 5: case 6: case 7:
				
				BaseActiva=SI;
				CabezaActiva=NO;
			break;
			case 0:
				BaseActiva=NO;
				CabezaActiva=NO;
			break;
		}
		envia_comando_motor(MotorActivo);
	}
	return 0;
}
/**********************************************************************************************
	Función que envía por el puerto el comando para programar el motor activo
***********************************************************************************************/
void envia_comando_motor(int mot)
{
	if (mot==-1) sprintf(CadenaComando,"%s","MOT:MA?");//Pregunta por el motor actual
	else sprintf(CadenaComando,"%s %u","MOT:MA ",mot); //Programa el motor actual
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
	Función que atiende a los controles de cambio de sentido
***********************************************************************************************/
int CVICALLBACK pc_cambia_sentido (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	unsigned int valor;
	//	SCPI_COMANDO(SENT,SE,pc_sentido)//Para cambiar el sentido
	if(event==EVENT_VAL_CHANGED)
	{
		GetCtrlVal(panel, control, &valor);
		envia_comando_sentido(valor);
	}
	return 0;
}
/**********************************************************************************************
	Función que envía por el puerto el comando para cambiar el sentido
***********************************************************************************************/
void envia_comando_sentido(int sen)
{
	if (sen==-1) sprintf(CadenaComando,"%s","MOT:SE?");//Pregunta por el sentido actual
	else sprintf(CadenaComando,"%s %u","MOT:SE ",sen); //Programa el sentido
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
	Función que atiende a los controles onda
***********************************************************************************************/
int CVICALLBACK pc_cambia_onda (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	unsigned int valor;
	//	SCPI_COMANDO(ONDA,ON,pc_modo_de_onda)
	if(event==EVENT_VAL_CHANGED)
	{
		GetCtrlVal(panel,control, &valor);
		envia_comando_onda(valor);
	}
	return 0;
}
/**********************************************************************************************
	Función que envía por el puerto el comando para programar la onda
***********************************************************************************************/
void envia_comando_onda(int onda)
{
	if (onda==-1) sprintf(CadenaComando,"%s","MOT:ON?");//Pregunta por la onda actual
	else sprintf(CadenaComando,"%s %u","MOT:ON ",onda); //Programa la onda actual
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
				Función que atiende a los controles resolución
***********************************************************************************************/
int CVICALLBACK cambia_resolucion (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	if (event== EVENT_VAL_CHANGED)
	{
		GetCtrlVal(panel,control, &Resolucion);
		envia_comando_resolucion(Resolucion);
	}
	return 0;
}
/**********************************************************************************************
	Función que envía por el puerto el comando para programar la resolución
***********************************************************************************************/
void envia_comando_resolucion(int res)
{
	if (res==-1) sprintf(CadenaComando,"%s","MOT:RE?");//Pregunta por la resolución actual
	else sprintf(CadenaComando,"%s %u","MOT:RE ",res); //Programa la resolucón actual
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
	Función que envía por el puerto el comando para programar la frecuencia
***********************************************************************************************/
void envia_comando_frecuencia(int frec)
{
	if (frec==-1) sprintf(CadenaComando,"%s","MOT:FR?");//Pregunta por la frecuencia actual
	else sprintf(CadenaComando,"%s %u","MOT:FR ",frec); //Programa la frecuencia actual
	pcom_enviar_datos(CadenaComando,1);
}
/**********************************************************************************************
				Función para cambiar la frecuencia de la base
***********************************************************************************************/
int CVICALLBACK cambiar_frecuencia (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int FrecuenciaAjustada;
	if(event==EVENT_VAL_CHANGED)
	{
		GetCtrlVal(panel, control, &Frecuencia);
		if(FrecuenciaAjustada=test_compatibilidad_frecuencia_resolucion(Resolucion,Frecuencia))
			{
				Frecuencia=FrecuenciaAjustada;
				SetCtrlVal(panel,control,Frecuencia);	
			}
		envia_comando_frecuencia(Frecuencia);	
	}
	return 0;
}
/*********************************************************************************************
			Función que muestra la información de copyright
**********************************************************************************************/
int CVICALLBACK recibir_acelerometro (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT)
	{
		sprintf(CadenaComando,"%s","MOT:AC?");
		pcom_enviar_datos(CadenaComando,1);
	}
	return 0;
}
/*********************************************************************************************
			Función pone la base en su estado inicial
**********************************************************************************************/
void reset(void)
{
	sprintf(CadenaComando,"MOT:RST");	
	pcom_enviar_datos(CadenaComando,1);
}
/*********************************************************************************************
			Función que muestra la información de copyright
**********************************************************************************************/

void CVICALLBACK about_base_spm (int menuBar, int menuItem, void *callbackData,
		int panel)
{
		 MessagePopup ("Base SPM",MENSAJE_ABOUT_BASE_SPM);
}
/***********************************************************************************************
 		Función que procesa los datos recibidos	
		esta función que se ejecuta automáticamente cuando se recibe un dato
***********************************************************************************************/
 void pcom_datos_recibidos(void)
{
	char firma[32];
	char char1,char2;
	int MarchaParo,Contador;
	float fuerzaNormal,fuerzaLateral,suma,temperatura,humedad;
	float g_x,g_y,g_z;//Señales del acelerómetro
//...........................................................................................	
	//Primero mira a ver si es una indicación de paro para el mando
	if(CadenaRespuesta[0]=='1' && ModoMandoComputadora == MODOMANDO)
	{
		desactiva_controles_tabs(); 
		cambia_estado_marcha_paro(0,0, PARO);
		return;
	}
//...........................................................................................	
// Ahora mira a ver si es la cadena del estado de los fotodiodos pedido por el PC	
	if (CadenaRespuesta[0]=='F' && CadenaRespuesta[1]=='T')
	{	
		sscanf(CadenaRespuesta,"%s %f %f %f",firma,&fuerzaNormal,&fuerzaLateral,&suma);
		if(FotodiodoRapido)
		{
			x2n=fuerzaNormal;x1n=fuerzaLateral;
			//Filtro fir1(4,0.1)//Cuarto orden ,wn=0.1*pi/2 
			y1n=0.0338*(x1n+x1n_4)+0.2401*(x1n_1+x1n_3)+0.4521*x1n_2;
			y2n=0.0338*(x2n+x2n_4)+0.2401*(x2n_1+x2n_3)+0.4521*x2n_2;
			x1n_4=x1n_3;
			x1n_3=x1n_2;
			x1n_2=x1n_1;
			x1n_1=x1n;
			x2n_4=x2n_3;
			x2n_3=x2n_2;
			x2n_2=x2n_1;
			x2n_1=x2n;
	 		PlotPoint (tab3Handle, TAB3_GRAFICO, y1n, y2n, VAL_SOLID_DIAMOND, VAL_LT_GRAY);
			SetCtrlVal(MainHandle,MAIN_FUERZANORMAL,fuerzaNormal);
			SetCtrlVal(MainHandle,MAIN_FUERZALATERAL,fuerzaLateral);
			SetCtrlVal(MainHandle,MAIN_SUMA,suma);
		}
		else
		{
			SetCtrlVal(MainHandle,MAIN_FUERZANORMAL,fuerzaNormal);
			SetCtrlVal(MainHandle,MAIN_FUERZALATERAL,fuerzaLateral);
			SetCtrlVal(MainHandle,MAIN_SUMA,suma);
		}
		return;
	}
//...........................................................................................	
		//Comprueba si han recibido las señales del acelerómetro
	
	if (CadenaRespuesta[0]=='L' && CadenaRespuesta[1]=='C')
	{	
		sscanf(CadenaRespuesta,"%s %f %f",firma,&x1n,&x2n);
		
		if(AcelerometroRapido)
		{
			
			/*
			//Filtro fir1(4,0.1)//Cuarto orden ,wn=0.1*pi/2 
			y1n=0.0338*(x1n+x1n_4)+0.2401*(x1n_1+x1n_3)+0.4521*x1n_2;
			y2n=0.0338*(x2n+x2n_4)+0.2401*(x2n_1+x2n_3)+0.4521*x2n_2;
			x1n_4=x1n_3;
			x1n_3=x1n_2;
			x1n_2=x1n_1;
			x1n_1=x1n;
			x2n_4=x2n_3;
			x2n_3=x2n_2;
			x2n_2=x2n_1;
			x2n_1=x2n;
	 		  */
			
			//Sin filtro
			y1n=x1n;
			y2n=x2n;
			//
			PlotPoint (tab2Handle, TAB2_GRAFICO, y1n, y2n, VAL_SOLID_DIAMOND, VAL_LT_GRAY);
			SetCtrlVal(MainHandle,MAIN_GX,y1n);
			SetCtrlVal(MainHandle,MAIN_GY,y2n);
		}
		else
		{
			SetCtrlVal(MainHandle,MAIN_GX,x1n);
			SetCtrlVal(MainHandle,MAIN_GY,x2n);
		}
		return;
	}
	//...........................................................................................	
	// Ahora mira a ver si es la cadena de estado pedido por el mando, si lo es refresca el estado del sistema	
	//Firma VAR
	if (CadenaRespuesta[0]=='B' && CadenaRespuesta[1]=='L')
	{
		sscanf(CadenaRespuesta,"%s %u %u %u %u %u %u",firma,&MotorActivo,&Resolucion,&Frecuencia,&Sentido,&Pasos,&MarchaParo);	
		//Actualiza  controles
		SetCtrlVal(tab1Handle,TAB1_MOTORACTIVO,MotorActivo);
		SetCtrlVal(tab1Handle,TAB1_RESOLUCION,Resolucion);
		SetCtrlVal(tab1Handle,TAB1_FRECUENCIA,Frecuencia);
		SetCtrlVal(tab1Handle,TAB1_SENTIDO,Sentido);
		SetCtrlVal(MainHandle,MAIN_PASOS,Pasos);
		cambia_estado_marcha_paro(0,0,MarchaParo);
		if(!MarchaParo) desactiva_controles_tabs(); 
		SetCtrlVal(MainHandle,MAIN_PASOS,Pasos);
		return;
	}
//...........................................................................................		
	if (CadenaRespuesta[0]=='S' && CadenaRespuesta[1]=='Z')
	{
		sscanf(CadenaRespuesta,"%s %u" ,firma,&Pasos);	
		//Actualiza  controles
		SetCtrlVal(MainHandle,MAIN_PASOS,Pasos);
		return;
	}
//...........................................................................................		
	if (CadenaRespuesta[0]=='P' && CadenaRespuesta[1]=='M')//Respuesta a "MP ?" = "MP 0"
	{
		sscanf(CadenaRespuesta,"%s %u" ,firma,&MarchaParo);	
		
		if(!MarchaParo)
		{
			cambia_estado_marcha_paro(0,0,PARO);
			desactiva_controles_tabs(); 
			//Pulsador de marcha a estado paro
			SetCtrlAttribute (tab1Handle, TAB1_MARCHAPARO, ATTR_LABEL_TEXT, "pulsar para marcha");
			SetCtrlVal (tab1Handle, TAB1_MARCHAPARO,0);
			envia_comando_pasos(PEDIR_DATO);			
		}
		return;
	}
//...........................................................................................	
	//Ahora comprueba si es el estado del fotodiodo enviado para el mando
	if (CadenaRespuesta[0]=='Y' && CadenaRespuesta[1]=='Y')
	{	
		sscanf(CadenaRespuesta,"%s %f %f %f %u %u",firma,&fuerzaNormal,&fuerzaLateral,&suma,&MarchaParo,&Pasos);
		
		if (MarchaParo==10) //Cambia los controles en función de si la  base está parada o no
		{
			cambia_estado_marcha_paro(0,0,MARCHA);
			}
		if (MarchaParo==5)
		{
			desactiva_controles_tabs(); 
			cambia_estado_marcha_paro(0,0,PARO);
		}
		//Actualiza los controles que indican las señales del fotodiodo
		SetCtrlVal(MainHandle,MAIN_FUERZANORMAL,fuerzaNormal);
		SetCtrlVal(MainHandle,MAIN_FUERZALATERAL,fuerzaLateral);
		SetCtrlVal(MainHandle,MAIN_SUMA,suma);
		return;
	}
//...........................................................................................
	//Comprueba si la base ha enviado la temperatura-humedad
	if (CadenaRespuesta[0]=='T')
	{	
		sscanf(CadenaRespuesta,"%c %f %c %f",&char1,&temperatura,&char2,&humedad);
		SetCtrlVal(MainHandle,MAIN_TEMPERATURA,temperatura);
		SetCtrlVal(MainHandle,MAIN_HUMEDAD,humedad);
		return;
	}
//...........................................................................................	
		//Comprueba si ha enviado la versión de la base
	if (CadenaRespuesta[0]=='K' && CadenaRespuesta[1]=='K')
	{
		
		strcpy(VersionDelInstrumento,CadenaRespuesta+2);//+2 para quitar KK
		SetPanelAttribute(MainHandle, ATTR_TITLE, VersionDelInstrumento);
		return;
	}
//...........................................................................................	
		//Comprueba si la base ha enviado el contador
	if (CadenaRespuesta[0]=='X' && CadenaRespuesta[1]=='T' )
	{	
		Contador=0;
		sscanf(CadenaRespuesta,"%s %u",firma, &Contador);
		SetCtrlVal(MainHandle,MAIN_CONTADOR,Contador);
		return;
	}
//...........................................................................................	
		//Comprueba si la base ha enviado stop
	if (CadenaRespuesta[0]=='Z' && CadenaRespuesta[1]=='P' )//Si recibimos un STOP
	{	
		cambia_estado_marcha_paro(0,0,PARO);//Cambiamos el estado a parado
		desactiva_controles_tabs(); 
		//Pulsador de marcha a estado paro
		SetCtrlAttribute (tab1Handle, TAB1_MARCHAPARO, ATTR_LABEL_TEXT, "pulsar para marcha");
		SetCtrlVal (tab1Handle, TAB1_MARCHAPARO,0);
		//envia_comando_pasos(PEDIR_DATO);	
		return;
	}	
	
		return;
}
/**********************************************************************************************
		función del tick del timer 
***********************************************************************************************/
int CVICALLBACK timer_pc (int panel, int control, int event,void *callbackData, int eventData1, int eventData2)
{
	// Las acciones se ejecutan cuando el contador tiene un valor multiplo de algún primo
	//Primos, (x) usados
	//(2) 3 (5) 7 11 (13) (17) 19 23 29.31 37 41 43 47 53 59 (61) 67 71.73 79 83 89 97 101 

	static int ContadorTicks=1;
	if (event==EVENT_TIMER_TICK)
	{
		SetCtrlVal (MainHandle, MAIN_LEDCONECTADO,FlagPuertoAbierto);	//Actualiza el led
		if(++ContadorTicks==67)ContadorTicks=1;; //Contador de ticks
		
		//Barra de progreso
		if(EstadoMarchaParo)
		{
			if(Sentido)
			{	
				Barra=Barra+4.0;
				if (Barra>=200.0) Barra=1.0;
				SetCtrlVal(MainHandle,MAIN_BARRA,Barra);
			}	
			else
			{
				Barra=Barra-4.0;
				if (Barra<=0.00) Barra=200.0;
				SetCtrlVal(MainHandle,MAIN_BARRA,Barra);
			}
		}
// Los siguientes if se verifican cada n ticks del timer siendo n primo para que no se 
// ejecuten 2 en el mismo tick		
//...........................................................................................			
		//Pide estado del acelerómetro
		if(ContadorTicks%3==0 && EstadoMarchaParo  && FlagPuertoAbierto && AcelerometroAuto && BaseActiva)
		{
			sprintf(CadenaComando,"MOT:AC?");pcom_enviar_datos(CadenaComando,1);return 0;	
		}
//...........................................................................................			
		//Pide las señales del fotodiodo
		if(ContadorTicks%3==0 && EstadoMarchaParo  && FlagPuertoAbierto && FotodiodoAuto && CabezaActiva)
		{
			sprintf(CadenaComando,"MOT:FOT?");pcom_enviar_datos(CadenaComando,1);return 0;	
		}
//...........................................................................................			
		//Pide los pasos que restan por dar
		if(ContadorTicks%5==0 && EstadoMarchaParo  && FlagPuertoAbierto && PasosAuto)
		{
			sprintf(CadenaComando,"MOT:AN ?");pcom_enviar_datos(CadenaComando,1);return 0;	
		}
//...........................................................................................			
		//Estado marcha paro
		if(ContadorTicks%13==0 && EstadoMarchaParo  && FlagPuertoAbierto && MarchaParoAuto)
		{
			sprintf(CadenaComando,"MOT:MP ?");pcom_enviar_datos(CadenaComando,1);return 0;	
		}
//...........................................................................................			
		//Pide humedad y temperatura
		//if(ContadorTicks%61==0 && !EstadoMarchaParo  && FlagPuertoAbierto && ThAuto)
		if(ContadorTicks%61==0 && !EstadoMarchaParo  && FlagPuertoAbierto && ThAuto)	
		{
			sprintf(CadenaComando,"MOT:TH?");pcom_enviar_datos(CadenaComando,1);return 0;	
			//El último
		}
	}
	return 0;	
}
/**********************************************************************************************
función que desactiva los controles de todos los tabs
***********************************************************************************************/
void desactiva_controles_tabs(void)
{
	desactiva_controles_tab0(0,0);
	desactiva_controles_tab1(0,0);
	desactiva_controles_tab2(0,0);
	desactiva_controles_tab3(0,0);
}
/**********************************************************************************************
		función que actualiza el estado de las teclas del tab1
		Los argumentos de entrada son el control pulsado y el estado de ese control
		Si se quiere desactivar todos (0,0)
***********************************************************************************************/
void desactiva_controles_tab0(int cntrl,int est)
{
	int arrayHandle; 
	arrayHandle=GetCtrlArrayFromResourceID ( tab0Handle, tab0ButtonsArray); //Handle del array de buttons
	SetCtrlArrayVal (arrayHandle,0);//Desactiva todos los toggle buttons
	if(cntrl) SetCtrlVal (tab0Handle, cntrl, est);//Activa solo el pulsado
}
/**********************************************************************************************
		función que actualiza el estado de los controles del tab 2
		Los argumentos de entrada son el control pulsado y el estado de ese control
		Si se quiere desactivar todos (0,0)
***********************************************************************************************/
void desactiva_controles_tab2(int cntrl,int est)
{
	int arrayHandle; 
	arrayHandle=GetCtrlArrayFromResourceID ( tab2Handle, tab2ButtonsArray); //Handle del array de buttons
	SetCtrlArrayVal (arrayHandle,0);//Desactiva todos los toggle buttons
	if(cntrl) SetCtrlVal (tab2Handle, cntrl, est);//Activa solo el pulsado
}
/**********************************************************************************************
		función que actualiza el estado de los controles del tab 3
		Los argumentos de entrada son el control pulsado y el estado de ese control
		Si se quiere desactivar todos (0,0)
***********************************************************************************************/
void desactiva_controles_tab3(int cntrl,int est)
{
	int arrayHandle; 
	arrayHandle=GetCtrlArrayFromResourceID ( tab3Handle, tab3ButtonsArray); //Handle del array de buttons
	SetCtrlArrayVal (arrayHandle,0);//Desactiva todos los toggle buttons
	if(cntrl) SetCtrlVal (tab3Handle, cntrl, est);//Activa solo el pulsado
}

/**********************************************************************************************
		función que actualiza el estado de los controles del tab 0
		Los argumentos de entrada son el control pulsado y el estado de ese control
		Si se quiere desactivar todos (0,0)
***********************************************************************************************/
void desactiva_controles_tab1(int cntrl,int est)
{
	SetCtrlAttribute (tab1Handle, TAB1_MARCHAPARO, ATTR_LABEL_TEXT, "pulsar para marcha");
	SetCtrlVal (tab1Handle, TAB1_MARCHAPARO,0);
}
/**********************************************************************************************
	Activa o desactiva los 48V
***********************************************************************************************/
int CVICALLBACK activa48V (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event==EVENT_COMMIT)
	{
		GetCtrlVal(MainHandle,MAIN_CHECK48V,&Estado48V);//Lee el estado del checkbox 48V..
		sprintf(CadenaComando,"%s %u","MOT:AV",Estado48V);//Envía el comando	
		pcom_enviar_datos(CadenaComando,1);
			
	}
	return 0;
}
/**********************************************************************************************
	función para activar o desactivar el estado del acelerómetro enviando datos cada
	100ms
***********************************************************************************************/
int CVICALLBACK acelerometro_rapido (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	
	int arrayHandle,Envios; 
	if(event==EVENT_COMMIT)
	{
		//arrayHandle=GetCtrlArrayFromResourceID ( MainHandle, mainChecksArray); //Handle del array de buttons
		//GetCtrlVal ( panel, control, &AcelerometroRapido);
		
		//if(AcelerometroRapido==NO)
		//{
			//SetCtrlArrayVal (arrayHandle,0);//Desactiva todos los checks del main
			//SetCtrlArrayAttribute (arrayHandle, ATTR_DIMMED, 1);//Desactiva todos los checks del main
			//FotodiodoAuto = NO;
			//ThAuto = NO;
			//PasosAuto = NO;
			//AcelerometroAuto=NO;//Para lectura automática del acelerómetro
			AcelerometroRapido=SI;
			FotodiodoRapido=NO;
			Envios=1;//Mínimo
			GetCtrlVal(tab2Handle,TAB2_ENVIOS,&Envios);
			sprintf(CadenaComando,"%s %u","MOT:IAC",Envios);			
		//}
		//else
		//{
		//	sprintf(CadenaComando,"MOT:FNA");
		//	//SetCtrlArrayAttribute (arrayHandle, ATTR_DIMMED, 0);//Desactiva todos los checks del main
		//	AcelerometroRapido=NO;
		//}
		pcom_enviar_datos(CadenaComando,1);
	}
	return 0;
}
/**********************************************************************************************
	función para activar o desactivar el estado del fotodiodo enviando datos cada
	100ms

***********************************************************************************************/
int CVICALLBACK fotodiodo_rapido (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{	
	int arrayHandle,Envios; 
	if(event==EVENT_COMMIT)
	{
	//	arrayHandle=GetCtrlArrayFromResourceID ( MainHandle, mainChecksArray); //Handle del array de buttons
	//	GetCtrlVal ( panel, control, &FotodiodoRapido);
	//	if(FotodiodoRapido==NO)
	//	{
			//FotodiodoRapido==SI;
	//		SetCtrlArrayVal (arrayHandle,0);//Desactiva todos los checks del main
	//		SetCtrlArrayAttribute (arrayHandle, ATTR_DIMMED, 1);//Desactiva todos los checks del main
			//FotodiodoAuto = NO;
			//ThAuto = NO;
			//PasosAuto = NO;
			//AcelerometroAuto=NO;//Para lectura automática del acelerómetro
			AcelerometroRapido=NO;
			FotodiodoRapido=SI;
			Envios=1;//Mínimo
			GetCtrlVal(tab3Handle,TAB3_ENVIOS,&Envios);
			sprintf(CadenaComando,"%s %u","MOT:IFO",Envios);			
	//	}
	//	else
	//	{
	//		sprintf(CadenaComando,"MOT:FIF");
	//		SetCtrlArrayAttribute (arrayHandle, ATTR_DIMMED, 0);//Desactiva todos los checks del main
	//		FotodiodoRapido=NO;
	//	}
		pcom_enviar_datos(CadenaComando,1);
	}
	return 0;
}
/***************************************************************************************************/

