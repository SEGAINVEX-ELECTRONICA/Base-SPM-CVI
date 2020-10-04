/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2020. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  MAIN                             1       /* callback function: panel_main */
#define  MAIN_TAB                         2       /* control type: tab, callback function: cambia_tab */
#define  MAIN_TIMER                       3       /* control type: timer, callback function: timer_pc */
#define  MAIN_GX                          4       /* control type: numeric, callback function: (none) */
#define  MAIN_GZ                          5       /* control type: numeric, callback function: (none) */
#define  MAIN_GY                          6       /* control type: numeric, callback function: (none) */
#define  MAIN_FUERZALATERAL               7       /* control type: numeric, callback function: (none) */
#define  MAIN_FUERZANORMAL                8       /* control type: numeric, callback function: (none) */
#define  MAIN_HUMEDAD                     9       /* control type: numeric, callback function: (none) */
#define  MAIN_ACELEROMETRO                10      /* control type: command, callback function: recibir_acelerometro */
#define  MAIN_GAUTO                       11      /* control type: radioButton, callback function: acelerometro_auto */
#define  MAIN_TEMPERATURA                 12      /* control type: numeric, callback function: (none) */
#define  MAIN_SUMA                        13      /* control type: numeric, callback function: (none) */
#define  MAIN_HUMEDADTEMPERATURA          14      /* control type: command, callback function: recibir_ht */
#define  MAIN_SPLITTER_7                  15      /* control type: splitter, callback function: (none) */
#define  MAIN_FOTODIODO                   16      /* control type: command, callback function: recibir_fotodiodo */
#define  MAIN_FOTOAUTO                    17      /* control type: radioButton, callback function: fotodiodo_auto */
#define  MAIN_TEXTMSG_19                  18      /* control type: textMsg, callback function: (none) */
#define  MAIN_THAUTO                      19      /* control type: radioButton, callback function: temperatura_humedad_auto */
#define  MAIN_SPLITTER_5                  20      /* control type: splitter, callback function: (none) */
#define  MAIN_SPLITTER_4                  21      /* control type: splitter, callback function: (none) */
#define  MAIN_CONTADOR                    22      /* control type: numeric, callback function: (none) */
#define  MAIN_PASOS                       23      /* control type: numeric, callback function: (none) */
#define  MAIN_PARO                        24      /* control type: command, callback function: paro_emergencia */
#define  MAIN_RESETCONTADOR               25      /* control type: command, callback function: reset_contador */
#define  MAIN_LEEERPASOS_2                26      /* control type: command, callback function: leer_contador */
#define  MAIN_LEEERPASOS                  27      /* control type: command, callback function: leer_pasos */
#define  MAIN_MARCHAPAROAUTO              28      /* control type: radioButton, callback function: marchaparo_auto */
#define  MAIN_PASOSAUTO                   29      /* control type: radioButton, callback function: pasos_auto */
#define  MAIN_BARRA                       30      /* control type: scale, callback function: (none) */
#define  MAIN_LEDCONECTADO                31      /* control type: LED, callback function: (none) */
#define  MAIN_SPLITTER_6                  32      /* control type: splitter, callback function: (none) */
#define  MAIN_SPLITTER_8                  33      /* control type: splitter, callback function: (none) */
#define  MAIN_SPLITTER_9                  34      /* control type: splitter, callback function: (none) */
#define  MAIN_SPLITTER                    35      /* control type: splitter, callback function: (none) */
#define  MAIN_CHECK48V                    36      /* control type: radioButton, callback function: activa48V */

     /* tab page panel controls */
#define  TAB0_FOTODIODO_DOWN              2       /* control type: toggle, callback function: toggle_button */
#define  TAB0_FOTODIODO_LEFT              3       /* control type: toggle, callback function: toggle_button */
#define  TAB0_FOTODIODO_RIGHT             4       /* control type: toggle, callback function: toggle_button */
#define  TAB0_Z3_DOWN                     5       /* control type: toggle, callback function: toggle_button */
#define  TAB0_Z3_UP                       6       /* control type: toggle, callback function: toggle_button */
#define  TAB0_Z2_DOWN                     7       /* control type: toggle, callback function: toggle_button */
#define  TAB0_Z2_UP                       8       /* control type: toggle, callback function: toggle_button */
#define  TAB0_Z123_DOWN                   9       /* control type: toggle, callback function: toggle_button */
#define  TAB0_ENVIARPASOS                 10      /* control type: command, callback function: cambiar_pasos */
#define  TAB0_Z123_UP                     11      /* control type: toggle, callback function: toggle_button */
#define  TAB0_LASER_UP                    12      /* control type: toggle, callback function: toggle_button */
#define  TAB0_Z1_DOWN                     13      /* control type: toggle, callback function: toggle_button */
#define  TAB0_LASER_DOWN                  14      /* control type: toggle, callback function: toggle_button */
#define  TAB0_LASER_LEFT                  15      /* control type: toggle, callback function: toggle_button */
#define  TAB0_MODO                        16      /* control type: binary, callback function: cambia_modo */
#define  TAB0_TEXTMSG_5                   17      /* control type: textMsg, callback function: (none) */
#define  TAB0_TEXTMSG_3                   18      /* control type: textMsg, callback function: (none) */
#define  TAB0_Z1_UP                       19      /* control type: toggle, callback function: toggle_button */
#define  TAB0_TEXTMSG_2                   20      /* control type: textMsg, callback function: (none) */
#define  TAB0_LASER_RIGHT                 21      /* control type: toggle, callback function: toggle_button */
#define  TAB0_TEXTMSG                     22      /* control type: textMsg, callback function: (none) */
#define  TAB0_TEXTMSG_4                   23      /* control type: textMsg, callback function: (none) */
#define  TAB0_TEXTMSG_6                   24      /* control type: textMsg, callback function: (none) */
#define  TAB0_TEXTMSG_22                  25      /* control type: textMsg, callback function: (none) */
#define  TAB0_TEXTMSG_21                  26      /* control type: textMsg, callback function: (none) */
#define  TAB0_DECORATION                  27      /* control type: deco, callback function: (none) */
#define  TAB0_FOTODIODO_UP                28      /* control type: toggle, callback function: toggle_button */
#define  TAB0_DECORATION_2                29      /* control type: deco, callback function: (none) */
#define  TAB0_ONDA                        30      /* control type: ring, callback function: (none) */
#define  TAB0_PASOS                       31      /* control type: numeric, callback function: (none) */
#define  TAB0_SPLITTER_3                  32      /* control type: splitter, callback function: (none) */
#define  TAB0_SPLITTER_2                  33      /* control type: splitter, callback function: (none) */
#define  TAB0_TEXTMSG_19                  34      /* control type: textMsg, callback function: (none) */
#define  TAB0_RESOLUCION                  35      /* control type: ring, callback function: (none) */
#define  TAB0_FRECUENCIA                  36      /* control type: numeric, callback function: (none) */
#define  TAB0_VELOCIDAD                   37      /* control type: scale, callback function: (none) */

     /* tab page panel controls */
#define  TAB1_TEXTMSG_17                  2       /* control type: textMsg, callback function: (none) */
#define  TAB1_MOTORACTIVO                 3       /* control type: ring, callback function: cambia_motor_activo */
#define  TAB1_RESOLUCION                  4       /* control type: ring, callback function: cambia_resolucion */
#define  TAB1_ONDA                        5       /* control type: ring, callback function: (none) */
#define  TAB1_PASOS                       6       /* control type: numeric, callback function: (none) */
#define  TAB1_FRECUENCIA                  7       /* control type: numeric, callback function: cambiar_frecuencia */
#define  TAB1_SENTIDO                     8       /* control type: binary, callback function: pc_cambia_sentido */
#define  TAB1_MARCHAPARO                  9       /* control type: toggle, callback function: pc_marcha_paro */
#define  TAB1_ENVIARPASOS                 10      /* control type: command, callback function: cambiar_pasos */
#define  TAB1_SPLITTER_2                  11      /* control type: splitter, callback function: (none) */
#define  TAB1_SPLITTER                    12      /* control type: splitter, callback function: (none) */
#define  TAB1_TEXTMSG_19                  13      /* control type: textMsg, callback function: (none) */

     /* tab page panel controls */
#define  TAB2_Z3_DOWN                     2       /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_GRAFICO                     3       /* control type: graph, callback function: (none) */
#define  TAB2_Z3_UP                       4       /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_Z2_DOWN                     5       /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_ENVIOS                      6       /* control type: numeric, callback function: (none) */
#define  TAB2_Z2_UP                       7       /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_TEXTMSG_3                   8       /* control type: textMsg, callback function: (none) */
#define  TAB2_Z123_DOWN                   9       /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_Z1_UP                       10      /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_TEXTMSG_2                   11      /* control type: textMsg, callback function: (none) */
#define  TAB2_ENVIARPASOS                 12      /* control type: command, callback function: cambiar_pasos */
#define  TAB2_Z123_UP                     13      /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_TEXTMSG                     14      /* control type: textMsg, callback function: (none) */
#define  TAB2_TEXTMSG_4                   15      /* control type: textMsg, callback function: (none) */
#define  TAB2_Z1_DOWN                     16      /* control type: toggle, callback function: tab2_toggle_button */
#define  TAB2_DECORATION_2                17      /* control type: deco, callback function: (none) */
#define  TAB2_ONDA                        18      /* control type: ring, callback function: (none) */
#define  TAB2_PASOS                       19      /* control type: numeric, callback function: (none) */
#define  TAB2_TEXTMSG_19                  20      /* control type: textMsg, callback function: (none) */
#define  TAB2_FRECUENCIA                  21      /* control type: numeric, callback function: (none) */
#define  TAB2_RESOLUCION                  22      /* control type: ring, callback function: (none) */
#define  TAB2_BOTON                       23      /* control type: command, callback function: acelerometro_rapido */

     /* tab page panel controls */
#define  TAB3_FOTODIODO_DOWN              2       /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_GRAFICO                     3       /* control type: graph, callback function: (none) */
#define  TAB3_FOTODIODO_LEFT              4       /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_FOTODIODO_RIGHT             5       /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_ENVIOS                      6       /* control type: numeric, callback function: (none) */
#define  TAB3_LASER_DOWN                  7       /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_LASER_LEFT                  8       /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_TEXTMSG_5                   9       /* control type: textMsg, callback function: (none) */
#define  TAB3_LASER_RIGHT                 10      /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_TEXTMSG_6                   11      /* control type: textMsg, callback function: (none) */
#define  TAB3_TEXTMSG_21                  12      /* control type: textMsg, callback function: (none) */
#define  TAB3_ENVIARPASOS                 13      /* control type: command, callback function: cambiar_pasos */
#define  TAB3_LASER_UP                    14      /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_FOTODIODO_UP                15      /* control type: toggle, callback function: tab3_toggle_button */
#define  TAB3_TEXTMSG_19                  16      /* control type: textMsg, callback function: (none) */
#define  TAB3_FRECUENCIA                  17      /* control type: numeric, callback function: (none) */
#define  TAB3_ONDA                        18      /* control type: ring, callback function: (none) */
#define  TAB3_DECORATION_2                19      /* control type: deco, callback function: (none) */
#define  TAB3_RESOLUCION                  20      /* control type: ring, callback function: (none) */
#define  TAB3_BOTON                       21      /* control type: command, callback function: fotodiodo_rapido */
#define  TAB3_PASOS                       22      /* control type: numeric, callback function: (none) */


     /* Control Arrays: */

#define  mainChecksArray                  1
#define  mainControlsArray                2
#define  tab0ButtonsArray                 3
#define  tab2ButtonsArray                 4
#define  tab3ButtonsArray                 5

     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_MENU1                    2
#define  MENUBAR_MENU1_TRAFICO            3       /* callback function: ver_trafico_puerto */
#define  MENUBAR_MENU1_CONFIGURACION      4       /* callback function: ver_configuracion_puerto */
#define  MENUBAR_MENU1_FICHERO            5       /* callback function: crear_fichero_puerto_COM */
#define  MENUBAR_MENU1_2                  6       /* callback function: about_base_spm */


     /* Callback Prototypes: */

void CVICALLBACK about_base_spm(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK acelerometro_auto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK acelerometro_rapido(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK activa48V(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cambia_modo(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cambia_motor_activo(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cambia_resolucion(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cambia_tab(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cambiar_frecuencia(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cambiar_pasos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK crear_fichero_puerto_COM(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK fotodiodo_auto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK fotodiodo_rapido(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK leer_contador(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK leer_pasos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK marchaparo_auto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panel_main(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK paro_emergencia(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK pasos_auto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK pc_cambia_sentido(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK pc_marcha_paro(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK recibir_acelerometro(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK recibir_fotodiodo(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK recibir_ht(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK reset_contador(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK tab2_toggle_button(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK tab3_toggle_button(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK temperatura_humedad_auto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK timer_pc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK toggle_button(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK ver_configuracion_puerto(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK ver_trafico_puerto(int menubar, int menuItem, void *callbackData, int panel);


#ifdef __cplusplus
    }
#endif
