/* ************************************************************************** */
/** Descriptive File Name

  @Company
    TecnoDalvo

  @Author:
        Ariel Scarafia

  @File Name
    include.h

  @Summary
        Include other files,
        Defines declarations,
        Function declarations,
        Variable declarations.

  @Description
    Include in this file global variables, global definitions, global file 
inclusions, and functions declarations.
 
 @Version_control:
 TDuC_v0.29:
 * Calculates temperature based on physical parameters from PT100 and LM334 measurements.
 * Adds calibration compensation (Ktemp2 and Ctemp2) to temperature calculation based on physical parameters.

 TDuC_v0.28:
 * Link with GitHub to start doing version control that way.
 * Merge Peltier and regular heating element code into one project, switched via defines.
 * ADC measuring is compensated by LM334 readout. LM334 average value during 
 calibration is stored during Calibration, and is used to compensate during each
 ADC reading.
 * Moved default values for startup on EEPROM wrong, to defines in this file (include.h)
 
 TDuC_v0.27:
 * Added Peltier control functionality.
 * Twitched a bit the code, remove some stuff from the 1ms timer interrupt.
 
 TDuC_v0.26:
 * Re-Write of the SmartPID calculations. Change to currentSlope detection to spot 
 oscillation instead of currentError, and to check on slope being near null to detect
 TempMax and TempMin for each period.
 * Corrected PID error calculation timing and the implications of that. Error is no longer multiplied by 10, is already a float value.
 * PID AutoTuning with NOAT (disabled calculation) doesn't stop. It has to be manually stopped.
 * Added test command, to turn every output and front panel LED ON for 10 seconds.
 
 TDuC_v0.25:
 * RGB LED shows color according to status.
 * Serial Number programmed with SQTP with IPE now working properly.
 * Hold button 3 seconds to turn on.
 * Add cancel button to exit calibrations.
 * Accepts to start process if current temperature is above set temperature.
 * Adds complex calculations and iterations when process starts with current 
 temperature above set temperature.
 
 TDuC_v0.24:
 * Datalog function: Array of up to 13k temperature samples, with pointer and 
 timeout for interval setting.
 * Datalog interface commands to download the data.
 * All variables are grouped into STRUCTs, to separate them into STRUCT groups
 
 TDuC_v0.23:
 * MultiPoint Calibration saves Temp conversion variables in ROM, uses linear
 regression parameters.
 * Ktemp and Ctemp (ADC temperature conversion parameters) now have a "factory
 reset" value, that updates after MultiPoint calibration.
 * "restore" command: Restores temperature calibration back to factory. 
 * RGB LED cicles through 5 values to eval the wanted colors. 
 
 TDuC_v0.21:
 * Check set temp against current temp before starting to heat up. Only accepts 
 if set temp is greater than current temp.
 * Buzzer programmed to OCM2, now sounds better.
 * Added counters for Buzzer to create different patterns.
 * Corrected Bug of "minus" sign when displaying "Cancelar"
 
 TDuC_v0.20:
 * DEVICE_POWER definition at compiler level, to set power percentages for 
 two point calibration and PIDAT purposes.
 * MultiPoint Calibration function prototype.
 
 TDuC_v0.19:
 * Two point temperature calibration
 
 TDuC_v0.18:
 * Self Cal function.
 * Light button with 10 seconds timeout
 
 TDuC_v0.17: 
 * Fixed flg_timer on restart, and blinking during debug.
 
 TDuC_v0.16:
 * Migration to board Rev.B
 * Buzzer works now
 
 TDuC_v0.15:
 * flg_borrar modified to be "safeguard". Turns off output when temperature exceeds
 setting point. Safeguard limit is set in 0.1º steps via USB.
 * Delete PID_calc.I_Term when process starts.
 
 
 TDuC_v0.14:
 * SmartPID: improvements in functioning. Reaching timeout improved, and continues
 to monitor and modifying parameters after it timeouts (if it hasn't reached yet)
 * PIDAutoTuning: Modified calculation for parameters, more accurate now.
 * SmartPID: overshoot flag, so it doesn't recalculate Kp or Ki twice per semi-cicle.
 * WarmUp PID_integer term multiplier modify
 * Modified initial PID constants to match new PID calculations
 * flg_borrar feature modified (3/4 instead of half), and copied to other lobes
 
 TDuC_v0.13:
 * Pre_warmup: starts the process for a calculated amount of time, at full power
 to speed heating process. It can be enabled/disabled via USB command.
 * WarmUp time multiplier added to USB configuration.
 * Saves PID parameters to EEPROM, and restores them when booting up.
 
 TDuC_v0.12:
 * Corrected tempSet and timeSet, now it works
 * Start and Stop process now inform so when done via USB command
 
 TDuC_v0.11:
 * Added firmware version 
 * Added serial number
 * PIDAT -> Reset parameters after finish, so it can be performed again
 * Change PID control period from 5 to 10 seconds
 
 TDuC_v0.10:
 * Added SmartPID -> checks for oscillation in temp regulation and processes PID parameters
 * Modify initial PID constants to match new PID formulae
 * Add START Process, STOP Process, SetTemp, SetTimer functions to UART
 * Added ProcessCounter -> counts the amount of seconds since process begins
 
 TDuC_v0.9:
 * Reformulate PID_Autotune to spot max slope, and calculate parameters 
 * Change PID timing from 100ms timer to 1ms timer.
 * Change PID parameters to be independent, not like the calculation before.
 
 TDuC_v0.8:
 * Revision of PID function operation.
  
 TDuC_v0.7:
 * Added PID_Autotune() with no AT. Doesn't compute PID coefficients, it only
 prints step response values for manual calculation.
 
 TDuC_v0.6:
 * Added USB UART interface!!
 * Added PID_Autotune(), to calculate PID coefficients. 
 
 TDuC_v0.5:
 * Added EEPROM memory retention for configuration parameters.
 * We start doing version control here. Most of the functionality is done
  
  
 */
/* ************************************************************************** */

#ifndef _EXAMPLE_FILE_NAME_H    /* Guard against multiple inclusion */
#define _EXAMPLE_FILE_NAME_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdint.h>

#include "mcc_generated_files/usb/usb_device.h"
#include "mcc_generated_files/usb/usb.h"
#include "mcc_generated_files/adc1.h"

#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/interrupt_manager.h"

/* This section lists the other files that are included in this file.
 */

#define versionA    0
#define versionB    29

#define DEFAULT_TempMax     3000                //Max temperature (300ºC)

#define DEFAULT_KTEMP       0.001257//0.001098;//0.001161;//0.001206;//0.001098;//0.000916662;//0.000730519;
#define DEFAULT_CTEMP       3029.254639//2549.830078;//2746.878174;//2900.055176;//2549.830078;//2597.403;//2664.1806089812;//
#define DEFAULT_LM334Comp   689250//2637100
#define DEFAULT_KTEMP2      1
#define DEFAULT_CTEMP2      0


#define DEFAULT_KP          0.06959//0.005487;//0.019358;//151.7;//25.9;
#define DEFAULT_KI          0.01350//0.000123;//0.00007;//1333.432;//833.58;//189.5;
#define DEFAULT_KD          1.255911//1.097152;//170.5;//38.76;


#define CONVERT_A           738.63636364//147.727272727//2954.54545455//
#define CONVERT_B           2597.4025974

//MainOutput to control heating element
#define MainOutput  BUS595_data.OUT_AC2

//Light Output controls inner light
#define LightOutput BUS595_data.OUT_AC3


//POWER_OUTPUT_A for 100W power oven, and POWER_OUTPUT_B for 1kW power oven
//POWER_OUTPUT_C for Peltier oven
#define POWER_OUTPUT_B

#ifdef POWER_OUTPUT_A       //100 W Power

#define TwoPointCal_Output  4           // 1 /(N+1)  % -> 33%
#define PIDAT_Output    7               //70% power output during PIDAT
#define Cal_PreHeat     600         //Pre heat 60 segundos

#define SModel     0x0A00

#endif

#ifdef POWER_OUTPUT_B         // 1kW Power

#define TwoPointCal_Output  4       // 1 /(N+1)  % -> 20%
#define PIDAT_Output    1           //10% power output during PIDAT
#define Cal_PreHeat     600         //Pre heat 60 segundos

#define SModel     0x0B00

#endif

#ifdef POWER_OUTPUT_C           //Peltier Output

#define TwoPointCal_Output  7           // 1 /(N+1)  % -> 33%
#define PIDAT_Output    7               //70% power output during PIDAT
#define Cal_PreHeat     600         //Pre heat 60 segundos

#define SModel     0x0C00

#endif

//Serial Number
struct {
    union {
        unsigned int i;
        unsigned char c[4];
        unsigned long L:32;
    };
    uint8_t string[15];
}SN;

unsigned long* SNPointer;

//*************************Defines          EEPROM Memory Map
#define SerialNumber        0x1D03F000                      //parte B del número de serie (parte A es fija y depende del modelo del equipo)
#define EEPROM_Data         SerialNumber+4//0x1D03F000              //8 bytes de secuencia de chequeo de EEPROM
#define EEPROM_Time         EEPROM_Data+8
#define EEPROM_Temp         EEPROM_Time+4
#define EEPROM_KTEMP        EEPROM_Temp+4
#define EEPROM_KTEMPFact    EEPROM_KTEMP+4
#define EEPROM_CTEMP        EEPROM_KTEMPFact+4
#define EEPROM_CTEMPFact    EEPROM_CTEMP+4    
#define EEPROM_KLM334       EEPROM_CTEMPFact+4
#define EEPROM_KTEMP2       EEPROM_KLM334+4
#define EEPROM_CTEMP2       EEPROM_KTEMP2+4
#define EEPROM_TMAX         EEPROM_CTEMP2+4
#define EEPROM_PID_Kp       EEPROM_TMAX+4
#define EEPROM_PID_Ki       EEPROM_PID_Kp+4
#define EEPROM_PID_Kd       EEPROM_PID_Ki+4
#define EEPROM_VariosFlgs   EEPROM_PID_Kd+4
#define EEPROM_Margin       EEPROM_VariosFlgs+4

/* TODO:  Include other files here if needed. */
unsigned char uc_USBCounter;
#define BT_Mid      2048

#define FALSE       0
#define TRUE        !FALSE

#define	uno         0b01000001
#define dos         0b00111011
#define tres        0b01101011
#define cuatro      0b01001101
#define cinco       0b01101110
#define seis        0b01111110
#define siete       0b01000011
#define ocho    	0b01111111
#define nueve       0b01101111
#define cero        0b01110111
#define nada        0

#define letra_a     0b01011111//0b01111011
#define letra_b     0b01111100
#define letra_o     0b01111000
#define letra_r     0b00011000
#define letra_t     0b00111100
#define signo_pregunta  0b11000111

#define letra_c     0b00110110
#define letra_n     0b01011000
#define letra_e     0b00111110
#define letra_l     0b00110100

#define signo_menos 0b00001000


#define dig_1           BUS595_data.digit1=1
#define dig_2           BUS595_data.digit2=1
#define dig_3           BUS595_data.digit3=1
#define dig_4           BUS595_data.digit4=1
#define clear_digits()  BUS595_data.digits=0

#define TEMP_SET_Led        BUS595_data.LED_TempSet
#define TIME_Led            BUS595_data.LED_TimeSet

#define CW          1
#define CCW         0

#define NO_ERROR    0
#define ERROR_TURN_ON   1
#define ERROR_MCP       2

#define Tiempo_Seteado  1
#define Tiempo_Restante 2
#define Temp_Set        3
#define Temp_Cal        4

#define state_StandBy       1
#define state_Config        2
#define state_Process       3
#define state_PIDAT         4
#define state_TwoPointCal   5
#define state_MultiPointCal 6

//**********************************************
//************ Peltier control
#define CoolDown_EN    CCP1CON2bits.OCEEN
#define HeatUp_EN      CCP7CON2bits.OCAEN//CCP3CON2bits.OCAEN//

#define CoolLAT     LATCbits.LATC4
#define HeatLAT     LATAbits.LATA9

#define CoolingPWM  CCP1RB
#define HeatingPWM  CCP7RB//CCP3RB//

long l_PeltierAux;
unsigned int Peltier_HowMuch;

//**********************************************


//**********************************************
// RGB LED colors define:

#define DigitsBlue_R    0x00
#define DigitsBlue_G    0x92
#define DigitsBlue_B    0xFF

#define AzulTD_R        0x00
#define AzulTD_G        0x80
#define AzulTD_B        0xFF

#define White_R         0xFF
#define White_G         0xFF
#define White_B         0xFF

#define Naranja_R       0xFF
#define Naranja_G       0x80
#define Naranja_B       0x00

#define Rojo_R          0xCC
#define Rojo_G          0x00
#define Rojo_B          0x00


char timeout;
char Error_Code;
unsigned int ui_test;

//**********************************Variables relacionadas a la EEPROM emulada
union {
    unsigned char data[4];
    unsigned long ul_data;
    long    l_data;
}EEPROM;

unsigned char EE_verify[8], EE_pointer, flg_EE_fault, EE_error;

//****************************************************************************

//*********************************BUZZER

#define BUZZER_ON       CCP2CON2bits.OCDEN=1
#define BUZZER_OFF      CCP2CON2bits.OCDEN=0

#define singleBeep      1
#define doubleBeep      2
#define shortAndLong    3
#define singleLong      4
#define turnoff         5


//#define BUZZER_Freq(int freq)   CCP2PR=freq//;CCP2RB=CCP2PR/2
struct {
    unsigned char flg:1;
    unsigned char Beeptype:7;
    
    unsigned int ta;
    unsigned int tb;
    unsigned int tc;
    unsigned int td;
    unsigned int te;
    
    unsigned int counter;
} BUZZER;

/*unsigned char flg_buzzer;
unsigned char uc_buzzer;

unsigned int ui_BZ_tA, ui_BZ_tB, ui_BZ_tC, ui_BZ_tD, ui_BZ_tE;
unsigned int ui_BZ_counter;*/

//****************************************************************************

//*********************************Variables relacionadas a la luz Interna
/*unsigned char fl_luzTimeout;

unsigned int ui_luztimeout;*/

struct {
    unsigned int ui_timeout:15;
    unsigned char flg_timeout:1;
} luz;

//****************************************************************************

//*********************************Variables relacionadas a la calibración

struct {
    unsigned char flg_1er:1;
    unsigned char flg_calibrate:1;
    unsigned char flg_SH:1;
    unsigned char flg_newData:1;
    unsigned char flg_CalHeat:1;
    unsigned char flg_dataB:1;
    unsigned char flg_CalEnd:1;
    
    unsigned char uc_CalHeatTimer;
    unsigned char uc_NPoints;
    
    unsigned int ui_PreHeatTimeout;
    
    unsigned long ul_mcpA;
    unsigned long ul_mcpB;
    
    long l_lm334A;
    long l_lm334B;
    
    float lm334Acum;
    
    float f_tempA;
    float f_tempB;
    float f_pendiente;
    float f_pendAnt;
    float f_Ktemp;
    float f_Cslope;                     //Ordenada al origen de ecuación de recta de inflexion
    
    float f_tempA2;                     //Temp2 calibration variables
    float f_tempB2;
    float f_tempA_b2;
    float f_tempB_b2;
    
    float f_pendAcum;
    float f_CAcum;
    float f_Ctemp;
    float f_Cant;
    float f_newData;
    
    float f_sumX;
    float f_sumY;
    float f_sumXY;
    float f_sumX2;
    float f_CalAux;
    float f_CalAux2;
} calibration;

/*
unsigned char flg_1er, flg_calibrate, flg_SH, flg_newData;
unsigned char flg_CalHeat, flg_dataB, flg_CalEnd;

unsigned char uc_CalHeatTimer;
unsigned char uc_NPoints;

unsigned int ui_PreHeatTimeout;

unsigned long ul_mcpA, ul_mcpB;

float f_tempA, f_tempB, f_pendiente, f_Ktemp, f_pendAnt;
float f_PendAcum, f_CAcum;
float f_Ctemp, f_Cant, f_newData;

float f_sumY, f_sumX, f_sumXY, f_sumX2, f_CalAux, f_CalAux2;
 * */
//****************************************************************************

//********************************   W2812
#ifndef WS2812_H
#define	WS2812_H

#define W2812 LATAbits.LATA8

int rgb_timeout;

typedef struct {
    unsigned char   r;
    unsigned char   b;
    unsigned char   g;
} ws2812_ptr;

ws2812_ptr led;
ws2812_ptr led2;
ws2812_ptr led3;
// transmit a single WS2812
void ws2812_send(ws2812_ptr* led);
void ws2812_Rst (void);
unsigned long int bitflip(unsigned char b);

#endif	/* WS2812_H */

//**************************** RGB loop
unsigned char RGB_counter, RGB_led;
//******************************************************************************


//****************************************************************************

//********************Variables asociadas al PID_Autotune
#define SLOPE_MIN   5               //Pendiente mínima para determinar que llegó a temperatura de equilibrio

struct {
    unsigned char flg_ONPID:1;
    unsigned char flg_slope:1;            //Vale FALSE mientras busco la inflexion, TRUE una vez encontrada
    unsigned char flg_1sec:1;             //Pasó un segundo, procesa el AT
    unsigned char flg_NOAT:1;             //No auto tune, para config manual
    
    unsigned char uc_secAT;             //Cantidad de segundos de intervalo para cálculo de AT
    unsigned char ui_outpercent;        //Porcentaje de señal de salida, múltiplo de 0,1 (10%)
    

    unsigned int ui_secCount;          //Contador de segundos transcurridos. Si uc_secCount>uc_secAT toma muestra
    unsigned int ui_1sec;               //Contador de 0 a 999 para contar 1 segundo (1000 x 1ms)
    unsigned int ui_AT_end;             //Contador de finalización de AutoTune
    unsigned int ui_tiempo_actual;      //Tiempo actual, en segundos, desde que inició el AutoTune
    unsigned int ui_tiempoAnterior;     //Tiempo de la última medición
//unsigned int ui_PIDAT_Aux;          //Auxiliar para operaciones de PIDAT
    unsigned int ui_t_inflexion;        //Tiempo en el que ocurre la inflexion
    unsigned int ui_t_inflexion_1;      //Tiempo de inflexión-1
    unsigned int ui_t_END;              //Cantidad de pendientes menores a la máxima admisible para terminar el PIDAT

//float power;                        //Potencia del calentador presente
//float volume;                       //Volumen de la cámara

    float f_tempInflexion;              //Temperatura donde ocurre la inflexion
    float f_tempInflexion_1;            //Temperatura de t-1 de inflexión
    float f_tempPIDautotune;            //Temperatura actual en el instante "ui_tiempo_actual"
    float f_tempPIDlast;                //Temperatura anterior, en "ui_tiempo_actual-1"
    float f_slope;                      //Pendiente instantánea
    float f_slopeLast;                  //Pendiente anterior, para comparación
    float f_Cslope;                     //Ordenada al origen de ecuación de recta de inflexion
    float f_PIDATslope;                 //Pendiente de inflexion
    float f_PIDATaux;                   //Variable auxiliar de PID AT
    float f_Td;                         //Variable auxiliar para calcular parámetros de PID
    float f_Rr;                         //Variable auxiliar para calcular parámetros de PID

}PIDAT;
/*
unsigned char flg_slope;            //Vale FALSE mientras busco la inflexion, TRUE una vez encontrada
unsigned char flg_1sec;             //Pasó un segundo, procesa el AT

unsigned char uc_secAT;             //Cantidad de segundos de intervalo para cálculo de AT
unsigned char ui_outpercent;        //Porcentaje de señal de salida, múltiplo de 0,1 (10%)
unsigned char flg_NOAT;             //No auto tune, para config manual

unsigned int ui_secCount;          //Contador de segundos transcurridos. Si uc_secCount>uc_secAT toma muestra
unsigned int ui_1sec;               //Contador de 0 a 999 para contar 1 segundo (1000 x 1ms)
unsigned int ui_AT_end;             //Contador de finalización de AutoTune
unsigned int ui_tiempo_actual;      //Tiempo actual, en segundos, desde que inició el AutoTune
unsigned int ui_tiempoAnterior;     //Tiempo de la última medición
unsigned int ui_PIDAT_Aux;          //Auxiliar para operaciones de PIDAT
unsigned int ui_t_inflexion;        //Tiempo en el que ocurre la inflexion
unsigned int ui_t_inflexion_1;      //Tiempo de inflexión-1
unsigned int ui_t_END;              //Cantidad de pendientes menores a la máxima admisible para terminar el PIDAT

float power;                        //Potencia del calentador presente
float volume;                       //Volumen de la cámara

float f_tempInflexion;              //Temperatura donde ocurre la inflexion
float f_tempInflexion_1;            //Temperatura de t-1 de inflexión
float f_tempPIDautotune;            //Temperatura actual en el instante "ui_tiempo_actual"
float f_tempPIDlast;                //Temperatura anterior, en "ui_tiempo_actual-1"
float f_slope;                      //Pendiente instantánea
float f_slopeLast;                  //Pendiente anterior, para comparación
float f_Cslope;                     //Ordenada al origen de ecuación de recta de inflexion
float f_PIDATslope;                 //Pendiente de inflexion
float f_PIDATaux;                   //Variable auxiliar de PID AT
float f_Td;                         //Variable auxiliar para calcular parámetros de PID
float f_Rr;                         //Variable auxiliar para calcular parámetros de PID*/

//****************************************************************************

//********************Estructura del estado de la tecla de interfaz
struct {
    unsigned int tiempo;            //Tiempo que lleva apretada la tecla, en cuentas de 0.1s
    unsigned char cual:3;           //Cual presioné
    unsigned char estado:1;         //Estado de la tecla -> 1=presionada; 0=soltó
    unsigned char procesado:1;      //Si alguna función registró la tecla -> 1=por procesar; 0=ya procesada
    unsigned char flg_encoder:1;    //Soft debounce for encoder
} tecla;

//Posición de la tecla
#define BT_Encoder      1
#define BT_Cancel       2
#define BT_Motor        3
#define BT_Luz          4
//*****************************************************************

//**************************Datalogging
#define DATALOG_Max         14000

struct {
    int16_t data[DATALOG_Max];
    unsigned int pointer;
    unsigned int pointer2;
    unsigned int tiempo;
    unsigned int timeout;
    float convert;
    
    unsigned char download:1;
    unsigned char record:1;
    unsigned char flg_send:1;
    unsigned char next:1;
} datalog;

unsigned char st_download[20];


//*****************************************************************

//**************************USB UART interface
unsigned char st_UART[50], st_UART_Rx[25], st_RX_Buffer[25];
unsigned char st_UART2[50];
unsigned char st_aux[25], st_aux2[25];
unsigned char uc_RX_bytes, uc_RX_Pointer, uc_RX_Pointer2;
unsigned char flg_parameter, flg_tempTx;
unsigned char flg_send, flg_send2;
unsigned char uc_debug, flg_debug;

unsigned int ui_Pointer;

unsigned int ui_USBinitDelay;

//****************************************************************************

//***************** Constantes de conversión de temperatura

struct {
    union {
        float K_temp;
        unsigned char c_kTemp[4];
        unsigned long KtempL:32;
    };
    
    union {
        float C_temp;
        unsigned char c_cTemp[4];
        unsigned long CtempL:32;
    };
    
    union {
        float K_tempFactory;
        unsigned char c_kTempFactory[4];
        unsigned long KtempFL:32;
    };
    
    union {
        float C_tempFactory;
        unsigned char c_cTempFactory[4];
        unsigned long CtempFL:32;
    };
    
    union {
        unsigned long LM334_comp;
        unsigned char c_LM334_Comp[4];
    };
    
    union {
        float K_LM334;
        unsigned char c_kLM334[4];
        unsigned long KLM334L:32;
    };
    
    union {
        float K_temp2;
        unsigned char c_kTemp2[4];
        unsigned long KtempL2:32;
    };
    
    union {
        float C_temp2;
        unsigned char c_cTemp2[4];
        unsigned long CtempL2:32;
    };
    
    union {
        unsigned int TEMP_MAX:16;
        unsigned long TempMaxL:32;
    };
    
    unsigned char flg_lm334:1;
    
}Convert;


//****************************************************************************

//***************** Constantes de conversión de PID

struct{
    union{
        float   Kp;
        unsigned char  c_Kp[4];
        unsigned long KPL:32;
    };
    union{
        float   Ki;
        unsigned char  c_Ki[4];
        unsigned long KIL:32;
    };
    union{
        float   Kd;
        unsigned char  c_Kd[4];
        unsigned long KDL:32;
    };
    
    union{
        float   Kp2;
        unsigned char  c_Kp2[4];
        unsigned long KP2L:32;
    };
    union{
        float   Ki2;
        unsigned char  c_Ki2[4];
        unsigned long KI2L:32;
    };
    union{
        float   Kd2;
        unsigned char  c_Kd2[4];
        unsigned long KD2L:32;
    };
}K_PID;

//******************************************************************************

//**************************** PID variables y resultado + SmartPID ************
#define signChange  3
#define processOvershoot        1000
#define SPID_loop               5000
#define e_permitido             150             //15.0ºC
#define e_permitido2            5               //00.5ºC

//************************************ PID Calculations
struct {
    int P_term;
    
    float I_term;
    float I_termMAX;
    float I_termMin;
    
    float D_term;
    float D_lastError;
    
    float aux;
    float error;
    float result;
} PID_calc;
//*****************************************************************************

//********************* Various working flags
union{
    struct {
        unsigned char SmartPID:1;           //SmartPID enable flag -> monitors PID and self adjust Kp, Ki and Kd
        unsigned char WrmpUP:1;             //WarmUp enable flag
        unsigned char SafeGuard:1;          //Safeguard enable flag
        unsigned char Dterm:1;              //Dterm enable flag
        unsigned char flgs:4;               //room for 4 more flags
      
    };
    
    unsigned char c[4];
    
    unsigned long L:32;
    
} VARIOS_flgs;
//***************************************************************************

//************************************ WARMUP Variables
struct {
    unsigned char flg:1;            //Warm up = pre-heating
    unsigned long ul_timer:31;      //Tiempo de precalentamiento
} warmup;
//***************************************************************************

//unsigned char flg_smartPID;         //Activa/desactiva SmartPID

//*********************Regulation and PID variables
struct {
    unsigned char flg_inicio:1;           //Inicia proceso de calentar.flg_smartPID Chequea si hay overshoot
    unsigned char flg_llego:1;            //llegó a la temperatura seteada
    unsigned char flg_ONPID:1;              //Enable output or not, according to PID result and current timer
    unsigned char flg_timer:1;              //There's a timeout set
    unsigned char enableOutput:1;           //Whether the output should be on or not, during heating process
    unsigned char flg_cambio:1;
    unsigned char flg_inicioArriba:1;       //Flag to indicate that the process starts when current temperature is above set temperature.
    unsigned char flg_otro:1;           //extra room for another process flag.

    unsigned char uc_process1sec;       //contador de 0 a 9 para 1 segundo en contador de proceso
   
//unsigned int ui_PIDcounter;         //PID timer counter
    unsigned int ui_PIDrefresh;         //PID output refresh value
    
//unsigned long ul_counter;
    unsigned long ul_processCount;      //Contador de segundos desde inicio de proceso
    
    int cuentaminuto;                 //Counter for 60 sec -> 1 minute for timeleft
    int tempSet;
    int tempRead;
    int timeSet;
    int timeLeft;                     //Remaining process countdown time 
    
    //int i_llego;                        //temperature at witch the oven considers it's on regulation (SetPoint - i_SetPointMargin)
    
    float f_iterm;    
    
    union {
        struct {
            int i_safelimit:16;                    //SafeGuard limit
            int i_SetPointMargin:16;                 //Dif. de temperatura a la que considera entrar en régimen
        };
        unsigned char uc_safeLimitAndSPMargin[4];       //char array for EEPROM storage
        unsigned long ul_LimitAndSPMargin:32;
    };
    
} calentar;
//*****************************************************************************

float f_aux;

//******************SmartPID variables
//If slope is below SlopeChange value, it's because it's reaching a positive or negativve
//peak in the oscillation.
#define SlopeChange     3           


struct {
    unsigned char flg_overshoot:1;    
    unsigned char flg_overshootCorrection:1;      //Corrigió o no el overshoot
    unsigned char flg_rising:1;                 //current slope rising or falling
    unsigned char flg_checkReach:1;             //whether to check if oven reached temperature
    
    unsigned char uc_lobeCount;         //Contador de posición dentro de la oscilación
    unsigned char uc_signCount;         //Cuenta cantidad de errores de signo opuesto (oscilación)

    unsigned int ui_SPIDcounter;        //Contador de loop de smartPID
    
    unsigned int ui_t1, ui_t2;             //Períodos de oscilación
//unsigned int ui_dif;

    unsigned long ul_tMaxRising;        //time of temperature max peak
    unsigned long ul_tMaxFalling;       //time of temperature min peak
    
    unsigned long ul_tMax3;
    unsigned long ul_tMax4;
    unsigned long ul_tMax5;
    unsigned long ul_tMax6;       //tiempos de picos de error en la oscilación

    unsigned long ul_t_lobeAstart;
//unsigned long ul_t_lobeBstart;
    unsigned long ul_overshoot_timeout;     //Overshoot timeout
    unsigned long ul_overshoot_MAX;
//unsigned long ul_reachCount;            //Cuenta si llegó a temperatura
    unsigned long ul_reachTIMEOUT;          //Límite de tiempo para modificar parámetros
    unsigned long ul_reachTIMEOUT_default;  //default reach timeout value, because the otherone gets moddified until oven reachs temperature.
    unsigned long ul_errorAvg;              //Average error for reach control section
    unsigned long ul_lastReachCheck;        //Last time reach check was performed.
    unsigned long ul_aux;
   
    unsigned long ul_tOvershoot;        //Tiempo en el cual ocurrió el pico de overshoot

    float f_peakMaxRising;          //peak temperature at rising lobe
    float f_peakMaxFalling;         //peak temperaturea at falling lobe
    
    float e_overshoot;              //Error durante overshoot
    float e_max3;                   //Máximo error durante oscilación num 3
    float e_max4;                   //Máximo error durante oscilación num 4
    float e_max5;                   //Máximo error durante oscilación num 5
    float e_max6;                   //Máximo error durante oscilación num 6
    
    float f_currentSlope;           //ccurrent slope calculation
    float f_tempAnt;                //Previous temperature for slope computation
    
    float f_aux,f_aux2;
} smartPID;
//*****************************************************************************

/*unsigned char flg_inicio;           //Inicia proceso de calentar.flg_smartPID Chequea si hay overshoot
unsigned char flg_llego;            //llegó a la temperatura seteada
unsigned char uc_lobeCount;         //Contador de posición dentro de la oscilación
unsigned char uc_signCount;         //Cuenta cantidad de errores de signo opuesto (oscilación)
unsigned char uc_process1sec;       //contador de 0 a 9 para 1 segundo en contador de proceso
unsigned long ul_processCount;      //Contador de segundos desde inicio de proceso
unsigned int ui_SPIDcounter;        //Contador de loop de smartPID*/
//unsigned long ul_counter;
//unsigned long ul_tOvershoot;        //Tiempo en el cual ocurrió el pico de overshoot
//unsigned long ul_tMax3, ul_tMax4, ul_tMax5, ul_tMax6;       //tiempos de picos de error en la oscilación
//unsigned int ui_t1, ui_t2;             //Períodos de oscilación
//unsigned int ui_dif;
//unsigned char flg_ONPID, include_D;

//unsigned char flg_safeguard;            //Safeguard function
//unsigned int i_safelimit;               //Safeguard limit (tempset-i_safelimit)


//unsigned char flg_EnableWarmup;         //Habilita o deshabilita warmup

float f_warmupK;
#define warmup_K    0.1             //warmup_K * error = tiempo de precalentamiento en seg

/*
unsigned char flg_overshoot;    
unsigned char flg_overshootCorrection;      //Corrigió o no el overshoot
float e_overshoot;              //Error durante overshoot
float e_max3;                   //Máximo error durante oscilación num 3
float e_max4;                   //Máximo error durante oscilación num 4
float e_max5;                   //Máximo error durante oscilación num 5
float e_max6;                   //Máximo error durante oscilación num 6
float f_aux,f_aux2;

unsigned long ul_t_lobeAstart, ul_t_lobeBstart;
unsigned long ul_overshoot_timeout;
unsigned long ul_overshoot_MAX;
unsigned long ul_reachCount;            //Cuenta si llegó a temperatura
unsigned long ul_reachTIMEOUT;          //Límite de tiempo para modificar parámetros
float f_iterm;
*/


unsigned int PID_TIME;          //Tiempo de intervalo para cálculo de PID

//unsigned int 
//*****************************************************************************

unsigned char uc_estado;
//unsigned char uc_595[4];

union {
    unsigned char uc_595[4];

    struct {
        //uc_595[0]
        unsigned char LED_TimeSet:1;
        unsigned char OUT_DC:1;
        unsigned char NC:1;
        unsigned char OUT_DC2:1;
        unsigned char LED_TempSet:1;
        unsigned char OUT_AC1:1;
        unsigned char OUT_AC2:1;
        unsigned char OUT_AC3:1;
        
        //uc_595[1]        
        union {
            struct {
                unsigned char RGB_B:1;
                unsigned char RGB_G:1;
                unsigned char RGB_R:1;
                unsigned char RGB_K:1;
                unsigned char digit1:1;
                unsigned char digit2:1;
                unsigned char digit3:1;
                unsigned char digit4:1;
            };
            struct {
                unsigned char RGB:4;
                unsigned char digits:4;
            };
        };
            
        //uc_595[2]
        union {
            unsigned char D2:8;
            struct {
                unsigned char D2_B:1;
                unsigned char D2_A:1;
                unsigned char D2_F:1;
                unsigned char D2_G:1;
                unsigned char D2_E:1;
                unsigned char D2_D:1;
                unsigned char D2_C:1;
                unsigned char D2_DP:1;
            };
        };
        
        //uc_595[3]
        union {
            unsigned char D1:8;
            struct {
                unsigned char D1_B:1;
                unsigned char D1_A:1;
                unsigned char D1_F:1;
                unsigned char D1_G:1;
                unsigned char D1_E:1;
                unsigned char D1_D:1;
                unsigned char D1_C:1;
                unsigned char D1_DP:1;
            };
        };
    };
} BUS595_data;

//unsigned char flg_encoder, flg_readADC, flg_rotateRight;
unsigned char flg_dimup, flg_turnoff, flg_abort;
unsigned char uc_encoder, uc_encodertime;
unsigned char flg_dimupRT, uc_dimRT, uc_delayOFFRT;
unsigned char flg_StdBy;//
unsigned char flg_muestro, flg_estable;
unsigned char flg_dimout, flg_tecla, flg_tecla_extra, flg_dimin;
unsigned char uc_error, uc_display;
//unsigned char uc_BT, uc_BT_Extra;

struct {
    unsigned char readADC:1;
    unsigned char rotateRight:1;
    
} operationFlgs;

unsigned char uc_turnoff;

unsigned char flg_tempNeg;  //temperatura medida es negativa, muestro signo "-"
unsigned char flg_print;

int Bt_Rt, Bt_Other;
unsigned int i_tiempotecla;

int uc_dimtopRT;
int uc_dimup, uc_dimuptop, uc_dimupRT, i_dimupRT_top;


unsigned char uc_loop, uc_counter, uc_digito;
unsigned char uc_uni, uc_dec, uc_cen, uc_mil;
unsigned char uc_uni2, uc_dec2, uc_cen2, uc_mil2;
unsigned char uc_adc_counter;
unsigned char uc_sequence, uc_aux;

int ui_mcpindex;

int i_rotary, i_aux, i_counter, i_aux2;

int i_adcBuf[16];
int i_mcp, i_mcp2;


int dim, dimer;

float f_mcp, f_mcp2, f_temp, f_tempSet, f_lm334;
long l_334, l_temp, l_compensation;

/*union {
    float f_temp;  
    unsigned char temp_bytes[4];
    unsigned long L:32;
} temp_PT100;*/

union {
    int i:16;
    unsigned char uchar[4];
    unsigned long L:32;
} TempDefault;

union {
    int i:16;
    unsigned char uchar[4];
    unsigned long L:32;
} TimeDefault;

unsigned long ul_mcp, ul_mcp2;

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    /* ************************************************************************** */
    /** Descriptive Constant Name

      @Summary
        Brief one-line summary of the constant.
    
      @Description
        Full description, explaining the purpose and usage of the constant.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
      @Remarks
        Any additional remarks
     */
#define EXAMPLE_CONSTANT 0


    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    // *****************************************************************************

    /** Descriptive Data Type Name

      @Summary
        Brief one-line summary of the data type.
    
      @Description
        Full description, explaining the purpose and usage of the data type.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Remarks
        Any additional remarks
        <p>
        Describe enumeration elements and structure and union members above each 
        element or member.
     */
    


    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    /*  A brief description of a section can be given directly below the section
        banner.
     */

    // *****************************************************************************
    /**
      @Function
        int ExampleFunctionName ( int param1, int param2 ) 

      @Summary
        Brief one-line description of the function.

      @Description
        Full description, explaining the purpose and usage of the function.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param param1 Describe the first parameter to the function.
    
        @param param2 Describe the second parameter to the function.

      @Returns
        List (if feasible) and describe the return values of the function.
        <ul>
          <li>1   Indicates an error occurred
          <li>0   Indicates an error did not occur
        </ul>

      @Remarks
        Describe any special behavior not described above.
        <p>
        Any additional remarks.

      @Example
        @code
        if(ExampleFunctionName(1, 2) == 0)
        {
            return 3;
        }
     */
    int ExampleFunction(int param1, int param2);


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
