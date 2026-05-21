/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL_1                          1       /* callback function: mainpanelfunc */
#define  PANEL_1_EXITBUT                  2       /* control type: command, callback function: exitbutfunc */
#define  PANEL_1_STATBUT                  3       /* control type: command, callback function: statbutfunc */
#define  PANEL_1_NEWGAMEBUT               4       /* control type: command, callback function: newgamebutfunc */
#define  PANEL_1_TXT1                     5       /* control type: textMsg, callback function: (none) */
#define  PANEL_1_CANVAS                   6       /* control type: canvas, callback function: (none) */

#define  PANEL_2                          2
#define  PANEL_2_RETURNMAIN               2       /* control type: command, callback function: rtrndiffmainfunc */
#define  PANEL_2_STARTGAME                3       /* control type: command, callback function: startbutfunc */
#define  PANEL_2_TXT2                     4       /* control type: textMsg, callback function: (none) */
#define  PANEL_2_CONTROLLER               5       /* control type: ring, callback function: (none) */
#define  PANEL_2_DIFFRING                 6       /* control type: ring, callback function: (none) */
#define  PANEL_2_CANVAS                   7       /* control type: canvas, callback function: (none) */

#define  PANEL_3                          3
#define  PANEL_3_RETURNMAIN               2       /* control type: command, callback function: rtrnstatmainfunc */
#define  PANEL_3_DISPBUT                  3       /* control type: command, callback function: displaybuttfunc */
#define  PANEL_3_TXT2                     4       /* control type: textMsg, callback function: (none) */
#define  PANEL_3_CANVAS                   5       /* control type: canvas, callback function: (none) */

#define  PANEL_4                          4
#define  PANEL_4_CANVAS                   2       /* control type: canvas, callback function: (none) */

#define  PANEL_5                          5
#define  PANEL_5_RETURNMAIN               2       /* control type: command, callback function: rtnpausemainfunc */
#define  PANEL_5_SAVEBUTT                 3       /* control type: command, callback function: savebuttfunc */
#define  PANEL_5_RESUMEBUTT               4       /* control type: command, callback function: resumebuttfunc */
#define  PANEL_5_CANVAS                   5       /* control type: canvas, callback function: (none) */

#define  PANEL_6                          6
#define  PANEL_6_RETURNMAIN               2       /* control type: command, callback function: funkk */
#define  PANEL_6_CANVAS                   3       /* control type: canvas, callback function: (none) */
#define  PANEL_6_TEXTMSG                  4       /* control type: textMsg, callback function: (none) */
#define  PANEL_6_CANVAS_2                 5       /* control type: canvas, callback function: (none) */

#define  PANEL_7                          7
#define  PANEL_7_RETURNSTAT               2       /* control type: command, callback function: rtrnstat2 */
#define  PANEL_7_GRAPH                    3       /* control type: graph, callback function: (none) */
#define  PANEL_7_CANVAS                   4       /* control type: canvas, callback function: (none) */


     /* Control Arrays: */

#define  CTRLARRAY                        1

     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_MENU1                    2
#define  MENUBAR_MENU1_OPTIONES           3
#define  MENUBAR_MENU1_OPTIONES_SUBMENU   4
#define  MENUBAR_MENU1_OPTIONES_SETDIFF   5
#define  MENUBAR_MENU1_OPTIONES_CONT      6
#define  MENUBAR_MENU1_MUSIC              7       /* callback function: sound_enabler */
#define  MENUBAR_MENU1_SEPARATOR          8
#define  MENUBAR_MENU1_Quit               9       /* callback function: QuitMenufunc */
#define  MENUBAR_MENU2                    10
#define  MENUBAR_MENU2_HIGHSCORE          11      /* callback function: highscorefunc */


     /* Callback Prototypes: */

int  CVICALLBACK displaybuttfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK exitbutfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK funkk(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK highscorefunc(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK mainpanelfunc(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK newgamebutfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK QuitMenufunc(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK resumebuttfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK rtnpausemainfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK rtrndiffmainfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK rtrnstat2(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK rtrnstatmainfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK savebuttfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK sound_enabler(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK startbutfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK statbutfunc(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif