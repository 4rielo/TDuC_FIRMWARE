/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    OMBAS

  @File Name
    main.c

  @Summary
    TDuC controller designed by OMBAS for TECNODALVO.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC32MM MCUs - 1.168.0
        Device            :  PIC32MM0256GPM048
    The generated drivers are tested against the following:
        Compiler          :  XC32 v2.40
        MPLAB 	          :  MPLAB X v5.40
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/
/******************************************************************************/
/**  Section: Included Files                                                  */
/******************************************************************************/

#include "mcc_generated_files/system.h"

#include <stdint.h>         /* For uint32_t definition                        */
#include <stdbool.h>
#include <string.h>        /* For true/false definition                      */

#include "include.h"
#include "MCP_3913.h"

//#include "string.h"
#include "stdio.h"

#include "dee.h"
#include "mcc_generated_files/mccp2_compare.h"
#include "mcc_generated_files/mccp3_compare.h"
#include "mcc_generated_files/mccp1_compare.h"


/******************************************************************************/
/**  Section: Function prototypes                                             */
/******************************************************************************/
void StandBy(void);                             //Handles StandBy state
void Config(void);                              //Handles Config state
void Process(void);                             //Handles Process (heating) state
void ChangeState(int change);                   //Handles state changes
void CalTwoPoints (void);                       //Handles Two Point Temperature Calibration
void CalMultiPoints(void);                      //Handles MultiPoint Temperature Calibration

void PID(void);                                 //PID Controller. Computes heating power
void SmartPID(void);                            //PID Supervisor. 
void CheckOvershoot(void);
void PID_Autotune(void);                        //PID Autotune. Computes PID parameters

void Delayms(int tiempo);                       //Small delay
void Refresh_digits(void);                      //Refresh display data
void Rotate_LEDs (unsigned char sentido);       //Handles rotation of encoder LEDs

void Process_number(int num);                   //Separates digits from number to decode
char Decode_7seg(char num);                     //Decode digit to 7segments

void ADC_CallBack(void);

//Grabo en EEPROM emulada parámetros del equipo
void Check_EEPROM(void);                        //Verify EEPROM at startup, and gets stored parameters
void Write_EEPROM(unsigned long Address, unsigned long Value);      //Writes EEPROM value to Address
void Save_ConfigEEPROM(void);                   //Saves configuration parameters to EEPROM
void Save_KPID_EEPROM(void);                    //Saves PID parameters to EEPROM
void Save_TempCorrection_EEPROM(void);          //Saves ADC to Temperature conversion constants
void Save_TempFactCorrection_EEPROM(void);      //Saves ADC to Temperature FACTORY conversion constants

void ProcessUART(void);                         //Processes USB-UART commands

//Peltier cell functions
void PeltierOff(void);                          //Turns OFF peltier module and Fans                    
void CoolDown(unsigned int HowMuch);            //Sets H Bridge to cooling, with "HowMuch" PWM
void HeatUp(unsigned int HowMuch);              //Sets H Bridge to heating, with "HowMuch" PWM

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/
int32_t main(void)
{

#ifndef PIC32_STARTER_KIT
    /*The JTAG is on by default on POR.  A PIC32 Starter Kit uses the JTAG, but
    for other debug tool use, like ICD 3 and Real ICE, the JTAG should be off
    to free up the JTAG I/O */
//    DDPCONbits.JTAGEN = 0;
#endif

    /* TODO Add user clock/system configuration code if appropriate.  */
    SYSTEM_Initialize();

    /* TODO <INSERT USER APPLICATION CODE HERE> */

    //****************************Start by cleaning up the display
    SPI1BUF=nada;
    while(SPI1STATbits.SPIBUSY){};
    SPI1BUF=nada;
    while(SPI1STATbits.SPIBUSY){};
    SPI1BUF=nada;
    while(SPI1STATbits.SPIBUSY){};
    SPI1BUF=nada;
    while(SPI1STATbits.SPIBUSY){};
    Refresh_SetHigh();
    Delayms(1);
    Refresh_SetLow();
    
    //***************************Turn Peltier module OFF
#ifdef POWER_OUTPUT_C    
    PeltierOff();
#endif
    //********************************Change USB Serial Number    
    SNPointer=SerialNumber+0x80000000;
    SN.L=*SNPointer;                    
    //sprintf(st_aux,"%04X%08X",SModel,SN.L);
    sprintf(st_aux,"%04X%08X",SModel,SN.L);
    for(i_aux=0;i_aux<13;i_aux++) {
        SN.string[i_aux]=st_aux[i_aux];
    }
    
    USBDeviceInit();
    //Attach USB
    USBDeviceAttach();
    INTERRUPT_GlobalEnable();
    
    /*if(SPI1STATbits.FRMERR) {
       SPI1STATbits.FRMERR=0;
    }*/
    
    RST_MCP_SetLow();

    //***************Turns front panel LEDs OFF
    LED_Cancel_SetLow();
    LED_Luz_SetLow();
    LED_USB_SetLow();
    LED_Motor_SetLow();
    led.r=0;
    led.g=0;
    led.b=0;
    ws2812_send(&led);
    //LED_Temp_SetLow();
    //Buzzer_SetLow();
    //SN=SerialNumber;
    uc_error=ERROR_TURN_ON;     //Se enciende por primera vez, prende led rojo de logo
    
    //**************************************Check EEPROM at startup
    //*****************retrieves stored parameters and calibration constants
    Check_EEPROM();
    
    //************************ External ADC Configuration
    timeout=0;
    LED_Cancel_SetHigh();
    do{
        RST_MCP_SetLow();
        Delayms(200);
        RST_MCP_SetHigh();
        Delayms(100);
        timeout++;
        if(timeout>20) break;
    } while(Config_MCP());               //Configura MCP3911

    if(timeout>20) {
        Error_Code=ERROR_MCP;
    } else LED_Cancel_SetLow();

    operationFlgs.rotateRight=1;        //Por defecto gira a la derecha
    
    flg_dimup=1;

    //**********************Buzzer setting
    CCP2PR=6000;
    CCP2RB=3000;
    BUZZER.flg=0;
    BUZZER.counter=0;
    MCCP2_COMPARE_Start();
    BUZZER_OFF;
    //*************************************************************************

    PID_TIME=99;            //Cada 10 segundos
    f_warmupK=0.25;
    smartPID.ul_overshoot_MAX=49;
    //smartPID.ul_reachTIMEOUT=999;
    calentar.timeSet=TimeDefault.i;    //Tiempo seteado por defecto
    calentar.tempSet=TempDefault.i;

    PID_calc.I_termMAX=500000;              //Max accumulated Iterm is 50.000 ºC
    PID_calc.I_termMin=500;                 //Min accumulated Iterm is 50ºC
    calentar.i_safelimit=3;              //0.3 grados por debajo del lï¿½mite
   
    flg_StdBy=1;                //inicia en Stand By
    uc_estado=state_StandBy;

    tecla.tiempo=0;
    tecla.cual=nada;

    //*********************************Datalog disable
    datalog.pointer=0;
    datalog.timeout=0;
    datalog.tiempo=0;
    //************************************************

#ifdef POWER_OUTPUT_C    
    //********************************** Cooling and Heating MAX PWM
    CCP1RA=0;
    CCP1PR=0xFFFF;                      //Colling PWM Period
    //CCP3RA=0;
    //CCP3PR=0xFFFF;
    
    CCP7RA=0;
    CCP7PR=0xFFFF;                      //Heating PWM Period
    //MCCP1_COMPARE_Start();
    //MCCP3_COMPARE_Start();
    
    PeltierOff();
    /*CCP3RB=0x8000;
    HeatingPWM=0x10;
    CoolingPWM=0x10;
    HeatUp_EN=1;
    CoolDown_EN=1;*/

    //***************************************************************
#endif
    
    TMR3_Start();
    TMR2_Start();
    flg_muestro=Temp_Set;


    //*************************************END of POST. Start MAIN Loop
    while(1)
    {
        //****************************Reads ADC value
        if(operationFlgs.readADC){        //Lee ADC MCP
            Read_MCP();
            operationFlgs.readADC=0;
        }
        //*******************************************
        
        
        //***************************************What is doing
        switch(uc_estado) {
            case state_StandBy:
                StandBy();
                break;
            case state_Config:
                Config();
                break;
            case state_Process:
                Process();
                break;
            case state_PIDAT:
                PID_Autotune();
                break;
            case state_TwoPointCal:
                CalTwoPoints();
                break;
            case state_MultiPointCal:
                CalMultiPoints();
                break;
            default:
                ChangeState(state_StandBy); //If there was an error in state, goes to StandBy
        }
        
        //*********************************************************

        
        //***************************Is USB attached?
        if(U1OTGSTATbits.VBUSVD) LED_USB_SetHigh();			//checks VBus level, and if high, turns USB LED on
        else LED_USB_SetLow();			//Otherwise, is not connected, turns USB LED Off
        
        //*************************************Checks for USB connection and checks if there's a new command.
        if(USBGetDeviceState()>30) {

            CDCTxService();

            if(ui_USBinitDelay>100){
                uc_RX_bytes=getsUSBUSART(st_RX_Buffer,25);
                if(uc_RX_bytes) {
                    //if(USBUSARTIsTxTrfReady()) putsUSBUSART(st_RX_Buffer);
                    for(uc_RX_Pointer2=0;uc_RX_Pointer2<uc_RX_bytes;uc_RX_Pointer2++){
                        if(st_RX_Buffer[uc_RX_Pointer2]>13 && uc_RX_Pointer<25){
                            //uc_RX_bytes=strlen(st_UART_Rx);
                            st_UART_Rx[uc_RX_Pointer]=st_RX_Buffer[uc_RX_Pointer2];
                            //strcat(st_UART_Rx,st_RX_Buffer);
                            //st_UART_Rx[uc_RX_Pointer]=st_RX_Buffer[0];
                            uc_RX_Pointer++;
                        } else {
                            ProcessUART();
                            break;
                        }
                    }
                }
            }
        } 

        //*****************************************If USB is attached, and ready to send
        //**************Checks whether there's a message to be sent
        if(USBUSARTIsTxTrfReady()){
            if(datalog.flg_send) {
                putsUSBUSART(st_download);
                datalog.flg_send=0;
            } else if(flg_send) {
                putsUSBUSART(st_UART);
                flg_send=0;
                
            } else if(flg_send2) {
                putsUSBUSART(st_UART2);
                flg_send2=0;                
            }
        }
        //*********************************************************************************

        //asm ("wait");
        //_wait();
        
    }
}


//******************************************************************************
//                     Two Point Calibration State routine
//******************************************************************************
void CalTwoPoints (void) {
    if(calibration.flg_newData) {   ///If there's a new data received via USB
        calibration.flg_newData=0;              
        if(calibration.flg_1er) {               //Is it the first data point?
            calibration.f_tempA=calibration.f_newData;      //Current temp is received data
            calibration.ul_mcpA=f_mcp;                      //Current ADC Value
            calibration.flg_1er=0;                          //done with first data
        } else {                                //Second temperature data
            calibration.f_tempB=calibration.f_newData;          //Current temp is new data
            calibration.ul_mcpB=f_mcp;                          //Point B temp. ADC value
            //*************Computes slope
            calibration.f_pendiente=calibration.f_tempB-calibration.f_tempA;            
            calibration.f_CalAux=calibration.ul_mcpB-calibration.ul_mcpA;
            if(calibration.f_CalAux) {
                calibration.f_pendiente/=calibration.f_CalAux;
            } else uc_error=ERROR_MCP;
            
            //***********Computes "y-intercept"
            calibration.f_CalAux=calibration.f_pendiente*calibration.ul_mcpB;
            calibration.f_Cslope=calibration.f_CalAux-calibration.f_tempB;

            //******************Prints calculated value to USB
            sprintf(st_UART,"K:%.6f:C:%.6f",calibration.f_pendiente,calibration.f_Cslope);
            flg_send=1;
            
            //*******************Stores calculated values to ADC-Conversion parameters
            Convert.C_temp=calibration.f_Cslope;
            Convert.K_temp=calibration.f_pendiente;

            Save_TempCorrection_EEPROM();            //Stores new data to EEPROM

            ChangeState(state_Config);                  //Returns to Config State
        }
    }

    //**************************************************************************
    //Waits for pre-heating at full power, and then asks for Point B
    
    if(!calibration.ui_PreHeatTimeout) {            //Cuando termina de pre-calentar
        if(calibration.flg_dataB) {
            sprintf(st_UART,"Cal2pt_B\n\r");        //Pide Punto B
            flg_send=1;
            calibration.flg_dataB=0;                //En tmr1 vuelve a pedir luego de 10 segundos, por las dudas
        }
    }
}

//******************************************************************************
//                     MultiPoint Calibration State routine
//******************************************************************************
void CalMultiPoints (void) {
    if(calibration.flg_newData) {               //There's a new data received
        calibration.flg_newData=0;

        calibration.uc_NPoints++;               //Cantidad de valores recibidos hasta el momento

        calibration.f_sumY+=calibration.f_newData;          //Sumatoria Y
        calibration.f_sumX+=f_mcp;              //Sumatoria X
        calibration.f_CalAux=calibration.f_newData*f_mcp;   //Cï¿½lculo Auxiliar XY
        calibration.f_sumXY+=calibration.f_CalAux;          //Sumatoria XY
        calibration.f_CalAux=f_mcp*f_mcp;       //Cï¿½lculo auxiliar X^2
        calibration.f_sumX2+=calibration.f_CalAux;          //Sumatoria X^2

        if(calibration.flg_1er) {                   //First value received
            calibration.f_tempA=calibration.f_newData;          //Temperatura ingresada por USB-UART
            calibration.ul_mcpA=f_mcp;                          //Temperatura ADC
            calibration.flg_1er=0;
            calibration.flg_dataB=1;
        } else if (calibration.flg_dataB) {
            calibration.flg_dataB=0;                            //primer pendiente calculada
            calibration.f_tempB=calibration.f_newData;          //Temperatura ingresada por USB-UART
            calibration.ul_mcpB=f_mcp;                          //Temperatura ADC

            calibration.f_pendiente=calibration.f_tempB-calibration.f_tempA;//Diferencia de temperatura medida
            calibration.f_CalAux=calibration.ul_mcpB-calibration.ul_mcpA;      //Diferencia de temperatura ADC
            if(calibration.f_CalAux) {                 //Checkea para evitar divisiï¿½n por cero
                calibration.f_pendiente/=calibration.f_CalAux;     //Calcula pendiente actual
            } else uc_error=ERROR_MCP;
            calibration.f_CalAux=calibration.f_pendiente*calibration.ul_mcpB;  //auxiliar para cï¿½lculo de constante C
            calibration.f_Cslope=calibration.f_CalAux-calibration.f_tempB;     //Constante C

            calibration.f_pendAcum+=calibration.f_pendiente;
            calibration.f_CAcum+=calibration.f_Cslope;

            calibration.f_pendAnt=calibration.f_pendiente;      //Guarda pendiente en valor anterior para promediar
            calibration.f_Cant=calibration.f_Cslope;            //Guarda constante C en valor anterior para promediar
            calibration.f_tempA=calibration.f_tempB;            //Guarda temperatura actual para nueva pendiente
            calibration.ul_mcpA=calibration.ul_mcpB;            //Guarda temp. ADC actual para nueva pendiente
        } else {
            calibration.f_tempB=calibration.f_newData;          //Temperatura ingresada por USB-UART
            calibration.ul_mcpB=f_mcp;              //Temperatura ADC

            calibration.f_pendiente=calibration.f_tempB-calibration.f_tempA;//Diferencia de temperatura medida
            calibration.f_CalAux=calibration.ul_mcpB-calibration.ul_mcpA;      //Diferencia de temperatura ADC
            if(calibration.f_CalAux) {                 //Checkea para evitar divisiï¿½n por cero
                calibration.f_pendiente/=calibration.f_CalAux;     //Calcula pendiente actual
            } else uc_error=ERROR_MCP;
            calibration.f_CalAux=calibration.f_pendiente*calibration.ul_mcpB;  //auxiliar para cï¿½lculo de constante C
            calibration.f_Cslope=calibration.f_CalAux-calibration.f_tempB;     //Constante C

            calibration.f_pendAcum+=calibration.f_pendiente;
            calibration.f_CAcum+=calibration.f_Cslope;

            calibration.f_pendAnt=calibration.f_pendiente;      //Guarda pendiente en valor anterior para promediar
            calibration.f_Cant=calibration.f_Cslope;            //Guarda constante C en valor anterior para promediar
            calibration.f_tempA=calibration.f_tempB;            //Guarda temperatura actual para nueva pendiente
            calibration.ul_mcpA=calibration.ul_mcpB;            //Guarda temp. ADC actual para nueva pendiente

            /*sprintf(st_UART,"K:%.6f:C:%.6f",f_pendiente,f_Cslope);
            flg_send=1;
            Convert.C_temp=f_Cslope;
            Convert.K_temp=f_pendiente;

            //Convert.K_temp=0.000916662;//0.435423541;//0.000730519;//0.0007333294;
            for(uc_aux=0;uc_aux<4;uc_aux++) EEPROM.data[uc_aux]=Convert.c_kTemp[uc_aux];
            flg_EE_fault=DEE_Write(EEPROM_KTEMP,EEPROM.ul_data);

            //Convert.C_temp=2597.403;//2664.1806089812;
            for(uc_aux=0;uc_aux<4;uc_aux++) EEPROM.data[uc_aux]=Convert.c_cTemp[uc_aux];
            flg_EE_fault=DEE_Write(EEPROM_CTEMP,EEPROM.ul_data);

            ChangeState(state_Config);*/
        }
    }

    if(calibration.flg_CalEnd) {                                //End of Calibration command received.
        calibration.f_CalAux=calibration.uc_NPoints-1;              //Computes linear regression
        if(calibration.f_CalAux) {                              //Verifies that there are more than 1 point taken
            calibration.f_pendAcum/=calibration.f_CalAux;     //Promedio tradicional de Pendiente
            calibration.f_CAcum/=calibration.f_CalAux;        //Promedio tradicional de C
        } else {
            calibration.f_pendAcum=0;
            calibration.f_CAcum=0;
        }

        if(calibration.f_sumX) {                        //Cï¿½lculo por regresion lineal
            calibration.f_CalAux=calibration.uc_NPoints * calibration.f_sumX2;
            calibration.f_CalAux/=calibration.f_sumX;
            calibration.f_CalAux2=calibration.f_sumX-calibration.f_CalAux;
            calibration.f_CalAux=calibration.f_sumY*calibration.f_sumX2;
            calibration.f_CalAux/=calibration.f_sumX;
            calibration.f_Cslope=calibration.f_sumXY-calibration.f_CalAux;
            if(calibration.f_CalAux2) calibration.f_Cslope/=calibration.f_CalAux2;
            else calibration.f_Cslope=0;

            calibration.f_CalAux=calibration.uc_NPoints*calibration.f_Cslope;
            calibration.f_CalAux2=calibration.f_sumY-calibration.f_CalAux;
            calibration.f_pendiente=calibration.f_CalAux2/calibration.f_sumX;
        } else {
            calibration.f_pendiente=0;
            calibration.f_Cslope=0;
        }

        //sprintf(st_UART,"A1:%.6f-B1:%.6f\n\rA2:%.6f-B2:%.6f\n\r",f_PendAcum,f_CAcum,f_pendiente,f_Cslope);  //Pide Punto B
        //flg_send=1;

        //***************************************** Sets ADC conversion constants with calculated values
        Convert.K_temp=calibration.f_pendiente;
        Convert.C_temp=-1*calibration.f_Cslope;

        //***************************************** Sets ADC conversion FACTORY constants with calculated values
        Convert.K_tempFactory=Convert.K_tempFactory;
        Convert.C_tempFactory=Convert.C_temp;
        
        //***************************************** Saves data to EEPROM
        Save_TempCorrection_EEPROM();           //saves temp correction to Emulated EEPROM
        Save_TempFactCorrection_EEPROM();       //saves factory correction to emulated EEPROM

        sprintf(st_UART,"MPtCalibration Ends\n\r");             //Informs via USB that calibration finished
        flg_send=1;
        ChangeState(state_Config);                              //returns to Config State
    }
}


//******************************************************************************
//                     Stand By State routine
//
//Checks for encoder Switch to turn on.
//******************************************************************************

void StandBy (void) {
    if(!flg_turnoff){
        if(tecla.estado && tecla.cual==BT_Encoder){             //Está siendo presionada la tecla del encoder
            if(tecla.tiempo>20) {                           //Durante más de 2 segundos
                tecla.procesado=1;                          //Registra la pulsación
                tecla.cual=nada;
                ChangeState(state_Config);                  //Y enciende el equipo
            }
        }
    }
}


//******************************************************************************
//                     Config State routine
//
//Checks for encoder Switch and rotation value to set temperature and time
//******************************************************************************
void Config (void) {

    /*if(flg_turnoff&&!flg_tecla) {
        flg_turnoff=0;
        i_tiempotecla=0;
    }*/

    if(tecla.estado && tecla.cual==BT_Encoder) {    //Si estï¿½ presionando el encoder

        if(tecla.tiempo>30) {      //Si mantengo presionado el Encoder mas de 3 seg
            tecla.cual=nada;
            tecla.procesado=1;
            flg_turnoff=1;
            ChangeState(state_StandBy);
        }

    }

    if(tecla.tiempo&&!tecla.estado) {        //Si suelto el pulsador
        if((tecla.cual==BT_Encoder)&&(tecla.tiempo<20)) {      //Habï¿½a apretado el encoder
                           //Estaba encendido
            tecla.procesado=1;
            tecla.cual=nada;
            if(flg_muestro==Temp_Set) {
                flg_muestro=Tiempo_Seteado;
            } else if(flg_muestro==Tiempo_Seteado) {        //Le doy OK nuevamente para comenzar
                ChangeState(state_Process);     //Comienza a calentar
            }
        }

        if((tecla.cual==BT_Cancel)&&(tecla.tiempo<20)) {      //Habï¿½a apretado el cancel
            if(flg_muestro==Tiempo_Seteado) {       //Si estaba mostrando el tiempo para configurar
                flg_muestro=Temp_Set;               //Vuelve a la configuraciï¿½n de temperatura
            } else if(flg_muestro==Temp_Set) {      //Si estaba mostrando la temperatura de seteo
                /*if(i_timeSet>0) {
                    i_timeLeft=i_timeSet;
                    flg_muestro=Tiempo_Restante;
                    flg_timer=1;
                }*/
            }
            //i_tiempotecla=0;        //Reinicio tiempo tecla
            tecla.cual=nada;
            tecla.procesado=1;
        }
    }
}


//******************************************************************************
//                     Process State routine
//
//Handles Heating process
//******************************************************************************
void Process (void) {

    if(tecla.tiempo&&!tecla.estado) {        //Si suelto el pulsador
        if((tecla.cual==BT_Encoder)&&(tecla.tiempo<20)) {      //Habï¿½a apreatado el encoder
            tecla.procesado=1;
            tecla.cual=nada;
            if(flg_abort){
                calentar.enableOutput=0;
                flg_abort=0;
                ChangeState(state_Config);
            } else {
                if(flg_muestro==Tiempo_Restante){
                    flg_muestro=Temp_Set;
                    //flg_muestro=nada;
                } else if(flg_muestro==Temp_Set){
                    flg_muestro=Tiempo_Restante;
                }
            }
        }
        else if((tecla.cual==BT_Cancel)&&(tecla.tiempo<20)) {      //Habï¿½a apreatado cancel
            flg_abort=0;            //cancelo el deseo de abortar proceso
            tecla.procesado=1;
            tecla.cual=nada;
        }

    } else if((tecla.cual==BT_Cancel)&&(tecla.tiempo>30)) {      //Mantengo apretado Cancel
        flg_abort=1;                //Quiero cancelar el proceso?
        tecla.procesado=1;
        tecla.cual=nada;
    }

    
    //*******************************PID callback every 10 seconds
    if(calentar.ui_PIDrefresh>PID_TIME){           //Cada (PID_TIME/10) (10) segundos 
        calentar.ui_PIDrefresh=0;
        PID();

#ifdef POWER_OUTPUT_C
        //******************* Peltier element, HeatsUp or CoolsDown according to PID_calc.result
        if(PID_calc.result>=0) {            //if PID_Calc Result is positive, heats up
            l_PeltierAux=(int) PID_calc.result;
            if(l_PeltierAux>0xFFFF) Peltier_HowMuch=0xFFFF;
            else Peltier_HowMuch=l_PeltierAux;
            HeatUp(Peltier_HowMuch);
        } else {    //if PID_Calc Result is negative, cools down
            l_PeltierAux=(-1)*(int) PID_calc.result;
            if(l_PeltierAux>0xFFFF) Peltier_HowMuch=0xFFFF;
            else Peltier_HowMuch=l_PeltierAux;
            CoolDown(Peltier_HowMuch);
        }
#endif
        
        if(VARIOS_flgs.SmartPID) SmartPID();
    }
}


//*****************************************************************************
//
//                  void ChangeState(int change)
//
// int change -> nuevo estado al cual debo ir. Variable global "uc_state" me dice
//en que estado estoy actualmente.
//
// Se encarga de hacer el paso de un estado a otro
// Apaga o prende LEDs segï¿½n sea necesario
//
//*****************************************************************************
void ChangeState (int change) {

    if(uc_estado!=change) {
        switch(uc_estado) {
//*****************************************************************************
// Estado de StandBy
        case state_StandBy:
            if(change==state_Config){           //y se enciende
#ifdef POWER_OUTPUT_C
                    PeltierOff();
#endif
                MainOutput=0;      //Apago salida
                i_aux=24000/4;
                CCP2PR=i_aux;
                CCP2RB=i_aux/2;
                //printf("StdBy -> Config\n\r");
                //LED_Temp_SetHigh();
                BUS595_data.LED_TempSet=1;
                //uc_BT=nada;                 //Borro el hecho de que estaba presionado
                i_tiempotecla=0;
                BUZZER.Beeptype=singleLong;               //Emito un pitido largo
                calentar.tempSet=TempDefault.i;    //Temperatura seteada por defecto
                calentar.enableOutput=0;                   //Por defecto NO estï¿½ calentando
                flg_muestro=Temp_Set;       //por defecto muestra temperatura
                calentar.timeSet=TimeDefault.i;    //Tiempo seteado por defecto
                flg_StdBy=0;                //Y enciendo
                TMR2_Start();
                TMR3_Stop();                //Detengo timer 3, de dimerizado RT
                flg_dimin=1;                //Dimerizo nï¿½meros en startup
                flg_turnoff=1;
                uc_estado=state_Config;     //cambio de estado
                if(uc_error==ERROR_TURN_ON) uc_error=0;
            }
            break;
//*****************************************************************************
// Estado de Config
        case state_Config:                  //Estaba configurando el equipo

            if(change==state_Process) {     //Y el nuevo estado es calentar

#ifndef POWER_OUTPUT_C          
                if( (int) f_temp > calentar.tempSet) {              //La temperatura actual es mayor que la seteada
                    calentar.flg_inicioArriba=1;            //activo flag de InicioArriba para funcionamiento
                } else {
                    calentar.flg_inicioArriba=0;
                }
#endif

                if(calentar.timeSet>0) {                    //There's a timeout set
                    calentar.timeLeft=calentar.timeSet-1;     //Copy the value to the countdown timer
                    calentar.flg_timer=1;                   //Set the timer flag, so the oven knows it has to countdown
                } else {
                    calentar.flg_timer=0;                   //Clears timer flag, so the oven knows not to countdown
                }
                TempDefault.i=calentar.tempSet;             //New default temperature is set temp
                TimeDefault.i=calentar.timeSet;             //New default timeout is timeout set
                
                flg_turnoff=1;
                PID_calc.I_term=0;                          //Iterm cumulative value starts at zero
                flg_abort=0;

                //****************SmartPID variables conf at startup
                smartPID.uc_lobeCount=0;                //starts at lobe 0
                smartPID.uc_signCount=0;                //sign counts starts at 0
                smartPID.ui_SPIDcounter=0;              //
                smartPID.e_overshoot=0;
                smartPID.ul_overshoot_timeout=0;
                smartPID.flg_overshoot=0;
                smartPID.ul_lastReachCheck=0;            //begins at time 0
                smartPID.ul_reachTIMEOUT=1000;          //reachTIMEOUT -> default time expected to have reached regulation
                smartPID.ul_reachTIMEOUT_default=1500;  //reachTIMEOUT -> default time expected to have reached regulation
                smartPID.f_tempAnt=f_temp;              //first "tempAnt" is current temperature when process starts
                if(calentar.tempSet!=0) f_tempSet=calentar.tempSet;                //Set temperature
                else f_tempSet=1;
                smartPID.ul_errorAvg=0;                 //Avg erro is 0 at the beggining
                        
                smartPID.f_peakMaxRising=0;         //starts with a null value to be easily overwritten
                smartPID.f_peakMaxFalling=5000;     //starts with a very high value to be easily overwritten

#ifndef POWER_OUTPUT_C                
                if(calentar.flg_inicioArriba) {     //if process starts with current temp above set temp.
                    smartPID.flg_checkReach=0;          //Check reach is null, because it is above current temp.
                    smartPID.flg_rising=0;              //starts falling, since current temp. is above set temp.
                    PID_calc.error=f_temp - (float) calentar.tempSet;         //computes init error
                    //PID_calc.error*=10;
                } else {
                    smartPID.flg_checkReach=1;          //enabless reach timeout check, because it starts below set temp.
                    smartPID.flg_rising=1;              //starts below set temp, so it will start to heat up, it'll be rising.
                    PID_calc.error=(float) calentar.tempSet - f_temp;         //computes init error
                    //PID_calc.error*=10;
                }
                //**********************************
                
                //****************Precalentamiento
                if(!calentar.flg_inicioArriba) {        //Starts heating with current temperature below set temp.
                    if(VARIOS_flgs.WrmpUP) { 
                        warmup.ul_timer=calentar.tempSet-f_temp;
                        warmup.ul_timer*=f_warmupK;//warmup_K;
                        warmup.flg=1;
                    }
                } else {                                //Starts heating with current temp. above set temp.
                    f_aux=f_temp - (float) calentar.tempSet;        //check initial temperature diference
                    if(f_aux>0) {                 //if positive, and non-zero
                        f_aux/=100;             //divides over 10º difference
                        f_aux=1-f_aux;
                        if(f_aux<0) f_aux=0;
                        PID_calc.I_term=PID_calc.I_termMAX/2;           //I term equals half of the max
                        PID_calc.I_term*=f_aux;                         //divided by temperature difference. If temp difference is 2 degrees, i_term=I_TermMax/4
                        if(PID_calc.I_term>PID_calc.I_termMAX) PID_calc.I_term=PID_calc.I_termMAX;
                    }
                }
#endif
                //*********************************
                calentar.flg_llego=0;               //It hasn't reached temperature yet. That's later to define
                calentar.enableOutput=1;                   //Enable output (Output power will depend on PID calculations)
                calentar.ul_processCount=0;                 //Overall process second counter
                if(!calentar.timeLeft) flg_muestro=Temp_Set;       //Si no se configurï¿½ tiempo, muestra temperatura
                else flg_muestro=Tiempo_Restante;   //Caso contrario, se muestra el tiempo restante
                uc_estado=change;
            }
            else if(change==state_PIDAT){           //New State is PID Autotuning
                PIDAT.f_tempPIDlast=f_temp;
                PIDAT.ui_outpercent=PIDAT_Output;
#ifdef POWER_OUTPUT_C
                Peltier_HowMuch=CCP2PR/10;
                Peltier_HowMuch*=PIDAT_Output;
                HeatUp(Peltier_HowMuch);
#endif
                PIDAT.ui_tiempo_actual=0;
                PIDAT.flg_slope=0;
                PIDAT.ui_secCount=0;
                PIDAT.flg_1sec=0;
                PIDAT.f_slopeLast=0;
                PIDAT.f_PIDATslope=0;
                PIDAT.ui_AT_end=0;
                PIDAT.ui_t_END=3000/PID_TIME;
                sprintf(st_UART,"Comienzo de AutoTuning: %d\n\r",PIDAT_Output);
                flg_send=1;
                uc_estado=state_PIDAT;
            }
            //*************************************************
            //Cambio a TwoPointCal
            //*************************************************
            else if(change==state_TwoPointCal){             //TwoPoint Calibration 
                calibration.flg_calibrate=1;        // Estï¿½ calibrando temperatura
                calibration.flg_1er=1;              //El dato que llegue va a ser el primero
                calibration.flg_newData=0;
                flg_muestro=Temp_Cal;
                calibration.flg_dataB=1;
                calibration.ui_PreHeatTimeout=Cal_PreHeat;      //10 segundos de pre-calentamiento rï¿½pido
                uc_estado=state_TwoPointCal;
            }
            //*************************************************
            //Cambio a MultiPointCal
            //*************************************************
            else if(change==state_MultiPointCal){
                calibration.flg_calibrate=1;        // Estï¿½ calibrando temperatura
                calibration.flg_1er=1;              //El dato que llegue va a ser el primero
                calibration.flg_newData=0;
                flg_muestro=Temp_Cal;
                calibration.ui_PreHeatTimeout=Cal_PreHeat;
                //Inicializo variables
                calibration.f_sumX=0;
                calibration.f_sumY=0;
                calibration.f_sumXY=0;
                calibration.f_sumX2=0;
                calibration.uc_NPoints=0;
                calibration.f_pendAcum=0;
                calibration.f_CAcum=0;
                calibration.flg_CalEnd=0;
                //ui_PreHeatTimeout=Cal_PreHeat;      //10 segundos de pre-calentamiento rï¿½pido
                uc_estado=state_MultiPointCal;
                sprintf(st_UART,"CalMPnt\n\r");
                flg_send=1;
            //*************************************************
            //El nuevo estado es StandBy, o hubo un error
            //*************************************************
            } else {                      //
#ifdef POWER_OUTPUT_C                
                PeltierOff();
#endif
                uc_estado=state_StandBy;
                MainOutput=0;      //Apago salida
                //printf("Config -> StandBy\n\r");
                flg_StdBy=1;                    //Entro en StandBy y apago todo
                LED_Cancel_SetLow();
                LED_Luz_SetLow();
                LED_USB_SetLow();
                LED_Motor_SetLow();
                BUS595_data.LED_TempSet=0;
                BUS595_data.LED_TimeSet=0;
                BUZZER.Beeptype=turnoff;//doubleBeep;
                //LED_Temp_SetLow();
                flg_turnoff=1;        //Flg de apagado, para ignorar el pulsador hasta que se suelte
                //TMR2_Stop();        //Detengo timers para que no moleste
                TMR3_Start();       //Enciendo timer 3 de dim
                //i_tiempotecla=0;        //Reinicio contador de tecla
                flg_dimout=1;       //Dimerizo de apagado los nï¿½meros
            }

        //}
            break;
//*****************************************************************************
// Estado de Process
        case state_Process:
        //else if(uc_estado==state_Process) {           //Si estaba calentando
            if(change==state_Config) {              //Y terminï¿½, o se cancelï¿½
#ifdef POWER_OUTPUT_C
                PeltierOff();
#endif
                //printf("Process -> Config\n\r");
                MainOutput=0;
                calibration.flg_CalHeat=0;
                PIDAT.flg_ONPID=0;
                calentar.flg_ONPID=0;
                
                BUS595_data.LED_TempSet=1;
                i_tiempotecla=0;
                flg_abort=0;
                calentar.flg_timer=0;
                BUZZER.Beeptype=shortAndLong;               //Emito un pitido
                calentar.tempSet=TempDefault.i;    //Temperatura seteada por defecto
                calentar.enableOutput=0;                   //Por defecto NO estï¿½ calentando
                flg_muestro=Temp_Set;       //por defecto muestra temperatura
                calentar.timeSet=TimeDefault.i;    //Tiempo seteado por defecto
                flg_StdBy=0;                //Y enciendo
                TMR2_Start();
                TMR3_Stop();                //Detengo timer 3, de dimerizado RT
                uc_estado=change;     //cambio de estado
            } else {                //hubo un error de cambio
#ifdef POWER_OUTPUT_C
                PeltierOff();
#endif
                MainOutput=0;
                uc_estado=state_StandBy;
                LED_Cancel_SetLow();
                LED_Luz_SetLow();
                LED_USB_SetLow();
                LED_Motor_SetLow();
                //LED_Temp_SetLow();
                BUS595_data.LED_TempSet=0;
                BUS595_data.LED_TimeSet=0;
                TMR3_Start();       //Enciendo timer 3 de dim
                flg_dimout=1;       //Dimerizo de apagado los nï¿½meros
            }
        //}
            break;

//*****************************************************************************
// Estado de PIDAT
        case state_PIDAT:                   //Estaba haciendo un AutoTuning
#ifdef POWER_OUTPUT_C
            PeltierOff();
#endif
            MainOutput=0;
            //LED_Temp_SetHigh();
            BUS595_data.LED_TempSet=1;
            //uc_BT=nada;                 //Borro el hecho de que estaba presionado
            i_tiempotecla=0;
            BUZZER.Beeptype=singleBeep;               //Emito un pitido
            calentar.tempSet=TempDefault.i;    //Temperatura seteada por defecto
            calentar.enableOutput=0;                   //Por defecto NO estï¿½ calentando
            flg_muestro=Temp_Set;       //por defecto muestra temperatura
            calentar.timeSet=TimeDefault.i;    //Tiempo seteado por defecto
            flg_StdBy=0;                //Y enciendo
            TMR2_Start();
            TMR3_Stop();                //Detengo timer 3, de dimerizado RT
            flg_dimin=1;                //Dimerizo nï¿½meros en startup
            flg_turnoff=1;
            sprintf(st_UART,"Fin de AutoTuning\n\r");
            flg_send=1;
            uc_estado=state_Config;     //cambio de estado
        //}
            break;
//*****************************************************************************
// Estado de TwoPointCal
        case state_TwoPointCal:             //Estaba haciendo calibración de 2 puntos
#ifdef POWER_OUTPUT_C
            PeltierOff();
#endif
            MainOutput=0;
            BUS595_data.LED_TempSet=1;
            //uc_BT=nada;                 //Borro el hecho de que estaba presionado
            i_tiempotecla=0;
            calibration.flg_calibrate=0;
            BUZZER.Beeptype=singleBeep;               //Emito un pitido
            calentar.tempSet=TempDefault.i;    //Temperatura seteada por defecto
            calentar.enableOutput=0;                   //Por defecto NO estï¿½ calentando
            flg_muestro=Temp_Set;       //por defecto muestra temperatura
            calentar.timeSet=TimeDefault.i;    //Tiempo seteado por defecto
            flg_StdBy=0;                //Y enciendo
            TMR2_Start();
            TMR3_Stop();                //Detengo timer 3, de dimerizado RT
            flg_dimin=1;                //Dimerizo nï¿½meros en startup
            flg_turnoff=1;
            sprintf(st_UART2,"Fin de Calibración\n\r");
            flg_send2=1;
            uc_estado=state_Config;     //cambio de estado
        //}
            break;
//*****************************************************************************
// Estado de MultiPoint Calibration
        case state_MultiPointCal:               //Was doing MultiPoint Calibration
#ifdef POWER_OUTPUT_C
            PeltierOff();
#endif
            MainOutput=0;      //Apago salida
            BUS595_data.LED_TempSet=1;
            //uc_BT=nada;                 //Borro el hecho de que estaba presionado
            i_tiempotecla=0;
            calibration.flg_calibrate=0;
            BUZZER.Beeptype=singleBeep;               //Emito un pitido
            calentar.tempSet=TempDefault.i;    //Temperatura seteada por defecto
            calentar.enableOutput=0;                   //Por defecto NO estï¿½ calentando
            flg_muestro=Temp_Set;       //por defecto muestra temperatura
            calentar.timeSet=TimeDefault.i;    //Tiempo seteado por defecto
            flg_StdBy=0;                //Y enciendo
            TMR2_Start();
            TMR3_Stop();                //Detengo timer 3, de dimerizado RT
            flg_dimin=1;                //Dimerizo nï¿½meros en startup
            flg_turnoff=1;
            sprintf(st_UART2,"Fin de Calibración\n\r");
            flg_send2=1;
            uc_estado=state_Config;     //cambio de estado
        //}
            break;
//*****************************************************************************
// Estado errï¿½neo
        //else {                //Se ingresï¿½ por error, manda a StandBy
        default:
            MainOutput=0;
#ifdef POWER_OUTPUT_C
            PeltierOff();
#endif
            uc_estado=state_StandBy;
        //}
            break;
        }
    }

}


//******************************************************************************
//                     Timer 1 CallBack routine
//
//Timing for 100 ms mark
//******************************************************************************
void TMR1_CallBack(void){       //100ms - Base temporal

    if(ui_USBinitDelay<2000) ui_USBinitDelay++;         //USB Init delay

    //*************** Checks current state 
    
    //If it's heating and regulating temperature
    if(uc_estado==state_Process) {                  
        
        calentar.uc_process1sec++;
        if(calentar.uc_process1sec>9) {          //One second passed
            calentar.ul_processCount++;         //total seconds since oven starts to heat
            //ul_reachCount++;
            smartPID.ui_SPIDcounter++;              
            calentar.uc_process1sec=0;          //restarts 1sec counter
            
            if(calentar.flg_inicioArriba) {
                PID_calc.error= f_temp - (float) calentar.tempSet;       //calculates negated error (see inside PID function to see real calculation)
                //PID_calc.error*=10;         //moves the decimal point of the error
            } else {
                PID_calc.error= (float) calentar.tempSet - f_temp;       //calculates negated error (see inside PID function to see real calculation)
                //PID_calc.error*=10;         //moves the decimal point of the error
            }
            
            if(PID_calc.error<calentar.i_SetPointMargin) {                       //Temperature is above SetPoint-SetPointMargin
                calentar.flg_llego=1;                              //it has entered regulation            
                if(calentar.flg_inicioArriba) calentar.flg_inicioArriba=0;
            }            
            
            if(VARIOS_flgs.SmartPID) {                          //if SmartPID is enabled
                smartPID.ul_errorAvg+=(int) PID_calc.error;          //stores a quarter of the error
            }
            
            //Timeleft Check
            if(calentar.flg_llego&&calentar.flg_timer) {            //If oven has arrived at the desired temperature
                calentar.cuentaminuto++;                            //A second has elapsed
                if(calentar.cuentaminuto>59) {                      //1 minute elapsed
                    calentar.cuentaminuto=0;                        //rst minute counter
                    if(calentar.timeLeft) calentar.timeLeft--;      //if there's some time left, decrease it
                    else {                                          //Reached 0 minutes left, turn output off
                        //BUZZER.Beeptype=shortAndLong;
                        //BUZZER.counter=0;
                        //BUZZER.flg=1;                        
                        calentar.enableOutput=0;
                        ChangeState(state_Config);                  //Finish heating process
                    }
                }
            }
            
            if(datalog.record) {            //Si tiene que registrar valores
                datalog.timeout--;
                if(!datalog.timeout) {          //pasó el tiempo determinado
                    if(datalog.pointer<DATALOG_Max) {             //Si no se llenó la memoria
                        datalog.data[datalog.pointer]=(int) f_temp;         //registro la temperatura
                        datalog.pointer++;                  
                        datalog.timeout=datalog.tiempo;             //reseteo el contador de intervalo
                    } else {                    //Se llenó la memoria, deja de registrar datos.
                        datalog.record=0;
                    }
                }
            }
        }
        
        //*************************************************************************
        
        calentar.ui_PIDrefresh++;
        if(warmup.flg) {
            calentar.flg_ONPID=1;
            if(calentar.ul_processCount>warmup.ul_timer) {
                warmup.flg=0;
                calentar.flg_ONPID=0;
                //Suma un error acumulado equivalente a una fracciï¿½n del tiempo de warmup
                //multiplicado por el error actual.
                f_aux=warmup.ul_timer*2;//5;
                f_aux*=PID_calc.error;
                PID_calc.I_term+=f_aux;
            }
        } else {
            if(calentar.ui_PIDrefresh<PID_calc.result) {
                calentar.flg_ONPID=1;
                //BUS595_data.OUT_AC2=~BUS595_data.OUT_AC2;
                if(VARIOS_flgs.SafeGuard) {
                    //calentar.f_iterm=calentar.tempSet-calentar.i_safelimit;
                    if(PID_calc.error<calentar.i_safelimit) calentar.flg_ONPID=0;
                }
            }
            else {
                calentar.flg_ONPID=0;
            }
        }
    }    
    //**************************************PID AutoTuning timing
    else if(uc_estado==state_PIDAT) {
        if(PIDAT.ui_1sec<PIDAT_Output) PIDAT.flg_ONPID=1;
        else PIDAT.flg_ONPID=0;

        PIDAT.ui_1sec++;
        if(PIDAT.ui_1sec>9) {        //cada 1 segundos
            PIDAT.ui_tiempo_actual++;           //A second has passed during PIDAT
            PIDAT.ui_1sec=0;
        }
        
        PIDAT.ui_secCount++;
        if(PIDAT.ui_secCount>PID_TIME) {            //Every 10 seconds (PID_TIME * 100 ms) (PID_TIME=99)
            PIDAT.f_tempPIDautotune=f_temp;         //Takes new temperature value for AT
            PIDAT.flg_1sec=1;                       //There's new data for calculations in PIDAT     
            PIDAT.ui_secCount=0;
        }
    }  
    //State -> Temperature Calibration state
    else if(uc_estado==state_TwoPointCal||uc_estado==state_MultiPointCal) {
        if(calibration.ui_PreHeatTimeout){      //Precalentar (calienta durante un tiempo inical
#ifdef POWER_OUTPUT_C
                Peltier_HowMuch=0xFFFF;
                HeatUp(Peltier_HowMuch);
#endif
                calibration.ui_PreHeatTimeout--;    // al 100% para lograr una temperatura alta
                calibration.flg_CalHeat=1;          // en poco tiempo).
            } else {                    //********************** Una vez pasado el tiempo de precalentar
#ifdef POWER_OUTPUT_C
                Peltier_HowMuch=6553;           //0xFFFF/10
                Peltier_HowMuch*=TwoPointCal_Output;         //Calienta al TwoPointCal_Output% (20%)
                HeatUp(Peltier_HowMuch);
#else 
                if(calibration.uc_CalHeatTimer) {   
                    calibration.uc_CalHeatTimer--;  // continua calentando al 10% de la potencia
                    calibration.flg_CalHeat=0;
                } else {
                    calibration.uc_CalHeatTimer=TwoPointCal_Output;      //Calienta al 1/(N+1) % (N=4 -> 20%)
                    calibration.flg_CalHeat=1;          //Indica que se encienda la resistencia
                    //flg_dataB=1;            //Vuelve a pedir Data B, por las dudas
                } 
#endif
                //*********************
            }
    }
    //******************************************************END of state timing

    //************************************************************************
    //          Ignoro pulsaciï¿½n durante el cambio de estado
    if(flg_turnoff) {               //Si estoy cambiando de estado
        tecla.cual=nada;             //La tecla registrada es nula
        tecla.tiempo=0;        //Y el contador de tecla tambiï¿½n
        uc_turnoff++;           //contador de delay en cambio de estado
        if((uc_turnoff>10)&&!tecla.estado) {   //Si pasï¿½ 1 seg desde el cambio de estado, y se soltï¿½ la tecla
            flg_turnoff=0;          //Terminï¿½ el cambio de estado
            uc_turnoff=0;           //y reseteo el contador de delay para otro cambio de estado
        }
    }
    //***********************************************************************

    flg_print++;

    //************************* Velocity control para el encoder timeout 1 seg
    if(uc_encodertime<10) {
        uc_encodertime++;
    } else {
        uc_encoder=0;
    }
    //*************************************************************************


    //******************************* Prendo o apago las luces segï¿½n el estado
    if(!flg_StdBy) {
        LED_Luz_SetHigh();
        LED_Motor_SetHigh();
        LED_Cancel_SetHigh();
        LEDs_Rt_SetLow();
        LEDs_Rt2_SetLow();              //Si estï¿½ encendido, apago leds RT
        LEDs_Rt3_SetLow();              //por el efecto que giran mientras giro el RT
        LEDs_Rt4_SetLow();
    } else {
        LED_Luz_SetLow();
        LED_Motor_SetLow();
        LED_Cancel_SetLow();
    }
    //************************************************************************

        //********************Chequeo que pulsador presionï¿½
    //if(ADC1_IsConversionComplete())

    //*****************Cancel Button pressed
    if(!Bt_Cancel_GetValue()){           //Pulsa Cancel
        if(!tecla.estado&&!flg_StdBy){ //Si es la primera vez q presiono
            LED_Cancel_SetLow();    //destello de apagado para indicar que se detectï¿½ la tecla
            BUZZER.Beeptype=singleBeep;           //y emito un pitido
        }
        tecla.cual=BT_Cancel;
        tecla.estado=1;                //Asiento que se estï¿½ presionando una tecla
        tecla.tiempo++;
    }
    //*************Rotary encoder Switch pressed
    else if(!Bt_Sw_GetValue()){                              //Pulsa Encoder
        if(!tecla.estado&&!flg_StdBy) BUZZER.Beeptype=singleBeep;            //emito un pitido para indicar que se detectï¿½ la pulsaciï¿½n
        tecla.cual=BT_Encoder;           //Registro que se trata del encoder
        tecla.estado=1;                //Asiento que se presionï¿½ una tecla
        tecla.tiempo++;            //Y comienzo a contar cuï¿½nto tiempo lleva presionada
    } 
    //*************** Light Switch pressed
    else if(!Bt_Luz_GetValue()){         //Pulsa Cancel
        if(!tecla.estado&&!flg_StdBy){ //Si es la primera vez q presiono
            LED_Luz_SetLow();    //destello de apagado para indicar que se detectï¿½ la tecla
            BUZZER.Beeptype=singleBeep;           //y emito un pitido
            luz.flg_timeout=1;
            luz.ui_timeout=100;
        }
        tecla.cual=BT_Luz;
        tecla.estado=1;                //Asiento que se estï¿½ presionando una tecla
        tecla.tiempo++;
    } 
    //**************** Nothing pressed, so, switch is released (whichever it was)
    else {
        tecla.estado=0;                //Registro que se soltï¿½ la tecla Cancel o Encoder
    }

    if(!tecla.estado && tecla.procesado) {        //Si soltï¿½ el pulsador y ya se procesï¿½ la tecla
        tecla.cual=nada;
        tecla.tiempo=0;                            //Reinicio el contador de pulsador
        tecla.procesado=0;
    } else if(!tecla.estado && tecla.tiempo>100) { //Si soltï¿½ el pulsador y pasaron mas de 20 segundos
        tecla.cual=nada;
        tecla.tiempo=0;                            //Reinicio el contador de pulsador
        tecla.procesado=0;
    }

    //********************Chequeo que pulsador presionï¿½ - FIN *****************

    
    //*************************************************************************
    //RGB LED status
    if(uc_error!=0) {           //Hay un error, RGB muestra color ROJO
        led.r=Rojo_R;
        led.g=Rojo_G;
        led.b=Rojo_B;
        led2=led;
        led3=led;
    } else {                    //No hay ningún error, RGB muestra estado horno
        if(uc_estado==state_StandBy) {      //Stand By -> RGB apagado
            led.r=nada;
            led.g=nada;
            led.b=nada;
            led2=led;
            led3=led;
        } else if(uc_estado==state_Process) {       //Process -> Está en proceso
            if(calentar.flg_llego) {        //flg_llegó: llegó a la temperatura seteada, RGB Azul
                led.r=AzulTD_R;                
                led.g=AzulTD_G;
                led.b=AzulTD_B;                
            } else {                        //flg_llego = 0 -> Está calentando, RGB Naranja
                led.r=Naranja_R;
                led.g=Naranja_G;
                led.b=Naranja_B;
            }
            led2=led;
            led3=led;
        } else if(uc_estado==state_Config) { //Está en modo de configuración de temperatura y tiempo, RGB Blanco
            led.r=White_R;
            led.g=White_G;
            led.b=White_B;
            led2=led;
            led3=led;
        } else if(uc_estado>=state_PIDAT) {
            if(RGB_counter){
                RGB_counter=0;
                if(RGB_led==1){
                    led.r=White_R;
                    led.g=White_G;
                    led.b=White_B;
                    
                    led2.r=Naranja_R;
                    led2.g=Naranja_G;
                    led2.b=Naranja_B;
                    led3=led2;
                    RGB_led++;
                } else if(RGB_led==2) {
                    led2.r=White_R;
                    led2.g=White_G;
                    led2.b=White_B;
                    
                    led.r=Naranja_R;
                    led.g=Naranja_G;
                    led.b=Naranja_B;
                    led3=led;
                    RGB_led++;
                } else if(RGB_led==3) {
                    led3.r=White_R;
                    led3.g=White_G;
                    led3.b=White_B;
                    
                    led2.r=Naranja_R;
                    led2.g=Naranja_G;
                    led2.b=Naranja_B;
                    led=led2;
                    RGB_led=1;
                } else RGB_led=1;
            } else RGB_counter++;
        }
    }
    
    ws2812_send(&led);
    ws2812_send(&led2);
    ws2812_send(&led3);
    //FIN RGB LEDs status
    //*************************************************************************

    //*************************************************************************
    // Hago sonar el buzzer segï¿½n la accion que corresponda
    switch(BUZZER.Beeptype) {
        case singleBeep:
            BUZZER.flg=1;
            BUZZER.counter=0;
            BUZZER.ta=2;     //suena durante 200 ms
            BUZZER.tb=0;
            BUZZER.tc=0;
            BUZZER.td=0;
            BUZZER.te=0;
            break;
        case doubleBeep:
            BUZZER.flg=1;
            BUZZER.counter=0;
            BUZZER.ta=2;     //suena durante 200 ms
            BUZZER.tb=2;
            BUZZER.tc=2;
            BUZZER.td=0;
            BUZZER.te=0;
            break;
        case shortAndLong:
            BUZZER.flg=1;
            BUZZER.counter=0;
            BUZZER.ta=2;     //suena durante 200 ms
            BUZZER.tb=2;
            BUZZER.tc=4;
            BUZZER.td=0;
            BUZZER.te=0;
            break;
        case singleLong:
            BUZZER.flg=1;
            BUZZER.counter=0;
            BUZZER.ta=7;     //suena durante 700 ms
            BUZZER.tb=0;
            BUZZER.tc=0;
            BUZZER.td=0;
            BUZZER.te=0;
            break;
        case turnoff:
            BUZZER.flg=1;
            BUZZER.counter=1;
            BUZZER.ta=7;     //suena durante 700 ms
            BUZZER.tb=0;
            BUZZER.tc=0;
            BUZZER.td=0;
            BUZZER.te=0;
            break;
    }
    BUZZER.Beeptype=0;

    if(BUZZER.flg){         //Emite pitido

        if(BUZZER.ta) {          //Etapa A - ON
            BUZZER_ON;
            BUZZER.ta--;
        } else if(BUZZER.tb) {   //Etapa B - OFF
            BUZZER_OFF;
            BUZZER.tb--;
        } else if(BUZZER.tc) {   //Etapa C - ON
            BUZZER_ON;
            BUZZER.tc--;
        } else if(BUZZER.td) {   //Etapa D - OFF
            BUZZER_OFF;
            BUZZER.td--;
        } else if(BUZZER.te) {   //Etapa E - ON
            BUZZER_OFF;
            BUZZER.te--;
        } else {
            BUZZER_OFF;
            BUZZER.flg=0;
        }
    }

    //*************************************************************************

    //******************************* Light Timeout
    if(luz.ui_timeout) {    //Presionï¿½ pulsador de luz, enciende durante 10 seg.
        luz.ui_timeout--;
    } else {
        LightOutput=0;
    }
    //*************************************************************************

    //***********************Debug messages. USB
    if(flg_debug){
        uc_debug++;
        if(uc_debug>9){
            uc_debug=0;
            sprintf(st_UART2,"T:%.2fA:%dB:%dC:%dD:%dE:%d\n\r",
                f_temp,calentar.tempSet,calentar.timeSet,calentar.timeLeft,calentar.ul_processCount, uc_estado);
            flg_send2=1;
        }
#ifdef POWER_OUTPUT_C
        if(uc_debug==5) {
            sprintf(st_UART2,"***DD:%d_A:%d____B:0x%X/0x%X____C:0x%X_D:%X___I:%d\n\r",
                CoolDown_EN,HeatUp_EN,CoolingPWM,HeatingPWM,Peltier_HowMuch,(int) PID_calc.result,(int) f_lm334);
            flg_send2=1;
        }
#endif 
    }
    //**************************************************************************
    
    

}

//******************************************************************************
//                     Timer 1 CallBack routine
//
//Timing for 1 ms mark. Mostly to refresh digits and output
//******************************************************************************
void TMR2_CallBack(void){        //1ms - refresco de nï¿½meros

    operationFlgs.readADC=1;        //Leo MCP

    //Se enciende la luz de cabina mientras el timeout este contando
    if(luz.ui_timeout) LightOutput=1;
    else LightOutput=0;
    //*************************************************************************

    //************** Toggle TRIAC if output is Set
    if(calentar.flg_ONPID || PIDAT.flg_ONPID) MainOutput=~MainOutput;
    else MainOutput=0;
        
    // Si estï¿½ calibrando, enciende la resistencia segï¿½n el valor de flg_CalHeat.
    if(calibration.flg_calibrate) {
        if(calibration.flg_CalHeat) {
            MainOutput=~MainOutput;
            LEDs_Rt_SetHigh();
            LEDs_Rt2_SetHigh();
            LEDs_Rt3_SetHigh();
            LEDs_Rt4_SetHigh();
        }
        else {
            MainOutput=0;
            LEDs_Rt_SetLow();
            LEDs_Rt2_SetLow();
            LEDs_Rt3_SetLow();
            LEDs_Rt4_SetLow();
        }
    }
    //**************************************************************************

    //*************************************************************************
    //Rotary encoder check
    if(uc_estado==state_Config){//!flg_StdBy && !flg_go) {        //Si NO estï¿½ en Stand By y que NO estï¿½ calentando
        if(!Sw_A_GetValue()){    //Veo el estado del encoder
            if(tecla.flg_encoder) {       //control antirebote
                uc_encodertime=0;     //reseteo el timeout del velocity control
                if(!Sw_B_GetValue()) {
                    if(uc_encoder>49) {
                        if(flg_muestro==Temp_Set) calentar.tempSet+=100;
                        else if(flg_muestro==Tiempo_Seteado) calentar.timeSet+=100;
                    }else if(uc_encoder>9) {
                        if(flg_muestro==Temp_Set) calentar.tempSet+=10;
                        else if(flg_muestro==Tiempo_Seteado) calentar.timeSet+=10;
                        uc_encoder++;
                    } else {
                        if(flg_muestro==Temp_Set) calentar.tempSet++;
                        else if(flg_muestro==Tiempo_Seteado) calentar.timeSet++;
                        uc_encoder++;
                    }
                    if(calentar.tempSet>Convert.TEMP_MAX) calentar.tempSet=Convert.TEMP_MAX;
                    if(calentar.timeSet>9999) calentar.timeSet=9999;
                    Rotate_LEDs(CW);
                }
                else{
                    if(uc_encoder>49){
                        if(flg_muestro==Temp_Set) calentar.tempSet-=100;
                        else if(flg_muestro==Tiempo_Seteado) calentar.timeSet-=100;
                    }else if(uc_encoder>9){
                        if(flg_muestro==Temp_Set) calentar.tempSet-=10;
                        else if(flg_muestro==Tiempo_Seteado) calentar.timeSet-=10;
                        uc_encoder++;
                    } else {
                        uc_encoder++;
                        if(flg_muestro==Temp_Set) calentar.tempSet--;
                        else if(flg_muestro==Tiempo_Seteado) calentar.timeSet--;
                    }
                    if(calentar.tempSet<0) calentar.tempSet=0;
                    if(calentar.timeSet<0) calentar.timeSet=0;
                    Rotate_LEDs(CCW);
                }
                tecla.flg_encoder=0;
            }
        } else {
            tecla.flg_encoder=1;
        }
    }
    //*************************************************************************

    //********************What to show on screen
    TEMP_SET_Led=0;
    TIME_Led=0;
    //Auxiliary value to show
    if(flg_muestro==Temp_Set){              //Shows Set Temperature
        TEMP_SET_Led=1;
        if(calentar.tempSet<0) i_aux=-1*calentar.tempSet;
        else i_aux=calentar.tempSet;
    } else if(flg_muestro==Tiempo_Restante) {       //Shows remaining time
        TIME_Led=1;
        i_aux=calentar.timeLeft;
    } else if(flg_muestro==Tiempo_Seteado) {        //Shows Set time
        TIME_Led=1;
        i_aux=calentar.timeSet;
    }

    if(!flg_abort) {        //If not in "abort" display
        if(flg_muestro==nada) {     //if set to show nothing
            uc_uni2=nada;
            uc_dec2=nada;
            uc_cen2=nada;
            uc_mil2=nada;
        } else {                //else, shows auxiliary value 
            Process_number(i_aux);
            uc_uni2=uc_uni;
            uc_dec2=uc_dec;
            uc_cen2=uc_cen;
            uc_mil2=uc_mil;
        }

        if(calibration.flg_calibrate || (uc_estado==state_PIDAT)) {     //If in some calibration mode
            uc_mil2=letra_c;                                            //Show "Cal."
            uc_cen2=letra_a;
            uc_dec2=letra_l;
            uc_uni2=nada;
        }
        
        if(f_temp<0) {                  //If temperature is negative
            i_aux=-1*f_temp;            //Computes absolute value
            flg_tempNeg=1;              //And sets negative sign to display
        }
        else {
            i_aux=f_temp;
            flg_tempNeg=0;
        }

        Process_number(i_aux);          //Process number to display in main set
        
    } else{             //If abort, then display "Cancelar"
        uc_mil2=letra_c;
        uc_cen2=letra_a;
        uc_dec2=letra_n;
        uc_uni2=letra_c;
        uc_mil=letra_e;
        uc_cen=letra_l;
        uc_dec=letra_a;
        uc_uni=letra_r;
    }

    clear_digits();           //Turns of digits
    //*************** Digit sweep
    switch(uc_digito) {
        case 0:
            dig_1;
            BUS595_data.D1=uc_uni;
            BUS595_data.D2=uc_uni2;
            if(Error_Code==ERROR_MCP) BUS595_data.D1=dos;
            break;
        case 1:
            dig_2;
            BUS595_data.D1=uc_dec;
            BUS595_data.D2=uc_dec2;
            if(!flg_abort) {
                if(flg_muestro==Temp_Set) BUS595_data.D2_DP=1;
                if(!flg_tempNeg) BUS595_data.D1_DP=1;
            }
            if(Error_Code==ERROR_MCP) BUS595_data.D1=letra_r;
            break;
        case 2:
            dig_3;
            BUS595_data.D1=uc_cen;
            BUS595_data.D2=uc_cen2;
            if(Error_Code==ERROR_MCP) BUS595_data.D1=letra_r;
            break;
        case 3:
            dig_4;

            if(flg_tempNeg&&!flg_abort) BUS595_data.D1=signo_menos;
            else BUS595_data.D1=uc_mil;

            if(Error_Code==ERROR_MCP) BUS595_data.D1=letra_e;

            BUS595_data.D2=uc_mil2;
            break;
    }
    uc_digito++;
    if(uc_digito>3) uc_digito=0;
    //*********************

    //*******************if dimming digits
    if(dim>dimer) {                     //Dimerizo numeros en encendido o apagado
        if(flg_dimin||flg_dimout) {
            BUS595_data.D1=0;                //porciï¿½n de tiempo con nï¿½meros apagados
            BUS595_data.D2=0;
        }
    }
    dim++;
    if(dim>10) dim=0;

    uc_dimuptop=(50-(dimer*5));
    if(uc_dimuptop<10) uc_dimuptop=10;
    uc_dimup++;
    if(uc_dimup>uc_dimuptop){//1500/(dimer+1))) {
        if(flg_dimin) {
            dimer++;
            if(dimer>10) flg_dimin=0;//flg_dimup=0;
        }
        if(flg_dimout){
            dimer--;
            if(dimer<=0) {
                flg_dimout=0;
                TMR2_Stop();
                TMR3_Start();
                i_dimupRT_top=0;
                uc_dimupRT=0;
            }
        }
        uc_dimup=0;
    }
    //**************************************************************************

    //If Output is set, turn Rotary LEDs ON
    if(MainOutput){
        LEDs_Rt_SetHigh();
        LEDs_Rt2_SetHigh();
        LEDs_Rt3_SetHigh();
        LEDs_Rt4_SetHigh();
    }

    //*****************************Test timeout is set -> light up everything
    if(ui_test) {
        ui_test--;

        LEDs_Rt_SetHigh();
        LEDs_Rt2_SetHigh();
        LEDs_Rt3_SetHigh();
        LEDs_Rt4_SetHigh();
        LED_Cancel_SetHigh();
        LED_Luz_SetHigh();
        LED_USB_SetHigh();
        LED_Motor_SetHigh();
        SPI1BUF=0xFF;
        while(SPI1STATbits.SPIBUSY){};
        SPI1BUF=0xFF;
        flg_tecla=1;
        while(SPI1STATbits.SPIBUSY){};
        SPI1BUF=0xFF;
        while(SPI1STATbits.SPIBUSY){};
        SPI1BUF=0xFF;
        while(SPI1STATbits.SPIBUSY){};
        Refresh_SetHigh();
        Delayms(100);
        Refresh_SetLow();
    } else if((!flg_StdBy)||flg_dimout) {           //Not in Stand By, display what is supposed to
        Refresh_digits();
    } else {                            //Else, is in stand By, or dimming, displays nothing
        SPI1BUF=nada;
        while(SPI1STATbits.SPIBUSY){};
        SPI1BUF=nada;
        flg_tecla=1;
        while(SPI1STATbits.SPIBUSY){};
        SPI1BUF=nada;
        while(SPI1STATbits.SPIBUSY){};
        SPI1BUF=nada;
        while(SPI1STATbits.SPIBUSY){};
        Refresh_SetHigh();
        Delayms(100);
        Refresh_SetLow();
    }

    
    //*******************************Internal datalog download data
    if(datalog.download&&datalog.next) {              //Si recibí comando para descargar datos
        if(datalog.pointer2<datalog.pointer){
            if(!datalog.flg_send) {         //Y ya envió un dato
                datalog.convert=(float) datalog.data[datalog.pointer2] / 10.0;
                sprintf(st_download,"D:;%d;%.1f\n\r",datalog.pointer2*datalog.tiempo,datalog.convert);
                datalog.pointer2++;
                datalog.flg_send=1;             //envía siguiente dato
                datalog.next=0;
            }
        } else {
            if(!datalog.flg_send) {         // No more data to send
                sprintf(st_download,"ENDofFILE\n\r");           //ENDofFile
                datalog.flg_send=1;
                datalog.download=0;
                datalog.next=0;
            }
        }
    }
    //**************************************************************************
}

//******************************************************************************
//                     Timer 3 CallBack routine
//
//Timing for 50 us mark for dimming
//******************************************************************************
void TMR3_CallBack(void) {              //50us para dimerizado de RT

    if(BUZZER.counter<30){
        BUZZER.counter++;
    } else {
        BUZZER.counter=1;
        CCP2PR+=10;
        CCP2RB=CCP2PR/2;
    }


    if(flg_StdBy) {         //if in standBy -> pulse rotary LEDs
        if(uc_dimRT<uc_dimtopRT) {
            LEDs_Rt_SetHigh();
            LEDs_Rt2_SetHigh();
            LEDs_Rt3_SetHigh();
            LEDs_Rt4_SetHigh();
        } else {
            LEDs_Rt_SetLow();
            LEDs_Rt2_SetLow();
            LEDs_Rt3_SetLow();
            LEDs_Rt4_SetLow();
        }
        uc_dimRT++;
        if(uc_dimRT>100) uc_dimRT=0;

        i_dimupRT_top=1000-(uc_dimtopRT*15);
        if(i_dimupRT_top<0) i_dimupRT_top=0;
        if(i_dimupRT_top<10) i_dimupRT_top=10;


        uc_dimupRT++;
        if(uc_dimupRT>i_dimupRT_top) {
            uc_dimupRT=0;
            if(flg_dimupRT){
                uc_dimtopRT++;
                if(uc_dimtopRT>100) flg_dimupRT=0;
            } else {
                if(uc_dimtopRT) uc_dimtopRT--;
                if(uc_dimtopRT<1) {
                    //uc_dimtopRT=0;
                    uc_delayOFFRT++;
                    if(uc_delayOFFRT>100) {
                        flg_dimupRT=1;
                        uc_delayOFFRT=0;
                    }
                }

            }
        }

    }
}

//******************************************************************************
//                     Short Delay routine
//
//
//******************************************************************************
void Delayms(int tiempo){           //Funciï¿½n de delay
    int counter;
    for(counter=0;counter<tiempo;counter++){
    }
}

//******************************************************************************
//                     ADC CallBack routine
//
//
//******************************************************************************

void ADC_CallBack(void){            //Lee ADC para ver estado de pulsadores
    /*ADC1_ConversionResultBufferGet(i_adcBuf);
    Bt_Rt=0;
    Bt_Other=0;
    for(uc_adc_counter=0;uc_adc_counter<16;uc_adc_counter+=2) {
        Bt_Rt+=i_adcBuf[uc_adc_counter];
        printf("BT_Rt %d\n\r",i_adcBuf[uc_adc_counter]);
    }
    for(uc_adc_counter=1;uc_adc_counter<16;uc_adc_counter+=2){
        Bt_Other+=i_adcBuf[uc_adc_counter];
        printf("BT_Other %d\n\r",i_adcBuf[uc_adc_counter]);
    }*/
    //Bt_Rt=ADC1BUF1;
    //Bt_Other=ADC1BUF0;

    //Bt_Rt=ADC1BUF10;
    //Bt_Other=ADC1BUF9;

    //ADC1_Start();
}

//******************************************************************************
//                     Process number routine
//
//Processes number and decodes it to display on the 7segment digits
//******************************************************************************
void Process_number(int num) {          //Separa los dï¿½gitos y los procesa a 7 segmentos
	uc_mil=num/1000;
	uc_cen=num/100;
	uc_dec=num/10;
	uc_uni=num;
	uc_uni-=uc_dec*10;
	uc_dec-=uc_cen*10;
	uc_cen-=uc_mil*10;

	uc_mil=Decode_7seg(uc_mil);
	uc_cen=Decode_7seg(uc_cen);
	uc_dec=Decode_7seg(uc_dec);
	uc_uni=Decode_7seg(uc_uni);
}

//******************************************************************************
//                     Decoding from number to 7 segment routine
//
//
//******************************************************************************
char Decode_7seg(char num) { //Decodifica un dï¿½gito de decimal a 7segmentos

	char c_return=0;

	switch (num) {
	case 0:
		c_return=cero;
		break;
	case 1:
		c_return=uno;
		break;
	case 2:
		c_return=dos;
		break;
	case 3:
		c_return=tres;
		break;
	case 4:
		c_return=cuatro;
		break;
	case 5:
		c_return=cinco;
		break;
	case 6:
		c_return=seis;
		break;
	case 7:
		c_return=siete;
		break;
	case 8:
		c_return=ocho;
		break;
	case 9:
		c_return=nueve;
		break;
	default:
		c_return=0;
		break;
	}

	return c_return;
}


//******************************************************************************
//                     Refresh digits routine
//
//
//******************************************************************************

void Refresh_digits(void) {         //Refresco estado del bus 595

	SPI1BUF=BUS595_data.uc_595[0];
    while(SPI1STATbits.SPIBUSY);
    SPI1BUF=BUS595_data.uc_595[1];
    while(SPI1STATbits.SPIBUSY);
    SPI1BUF=BUS595_data.uc_595[2];
    while(SPI1STATbits.SPIBUSY);
    SPI1BUF=BUS595_data.uc_595[3];
    while(SPI1STATbits.SPIBUSY);

    Refresh_SetHigh();
    Delayms(100);
    Refresh_SetLow();

}

//******************************************************************************
//                     Rotate LEDs routine
//
//Rotates clockwise or counter-clockwise the LEDs following the rotation of
//the rotary encoder.
//******************************************************************************
void Rotate_LEDs (unsigned char sentido) {      //Hace girar los leds del encoder
    switch(uc_sequence){
        case 0:
            LEDs_Rt_SetHigh();
            LEDs_Rt2_SetLow();
            LEDs_Rt3_SetLow();
            LEDs_Rt4_SetLow();
            break;
        case 1:
            LEDs_Rt_SetLow();
            LEDs_Rt2_SetHigh();
            LEDs_Rt3_SetLow();
            LEDs_Rt4_SetLow();
            break;
        case 2:
            LEDs_Rt_SetLow();
            LEDs_Rt2_SetLow();
            LEDs_Rt3_SetHigh();
            LEDs_Rt4_SetLow();
            break;
        case 3:
            LEDs_Rt_SetLow();
            LEDs_Rt2_SetLow();
            LEDs_Rt3_SetLow();
            LEDs_Rt4_SetHigh();
            break;
    }

    if(sentido) {
        uc_sequence--;
        if(uc_sequence>3) uc_sequence=3;
    }
    else{
        uc_sequence++;
        if(uc_sequence>3) uc_sequence=0;
    }
}


//******************************************************************************
//                     PID routine
//
//Computes how much power to deliver to the heating (or peltier) element
//******************************************************************************
void PID(void) {
    //Del PDF de autotuning: Output (%) = (100/Kp) * (e + SUM(e)/Ki + delta(e)*Kd)

    //PID_calc.error=(float) calentar.tempSet - f_temp;     //calculo el error entre mediciï¿½n y seteo
    //PID_calc.error*=10;

    /*if(flg_borrar) {
        if(PID_calc.error>490 && PID_calc.error<510) {
            PID_calc.I_term/=2;
            flg_borrar=0;
        }
    }*/
    PID_calc.P_term=PID_calc.error*K_PID.Kp;          //Calculo parte Proporcional

    //calculo parte Integrativa, y limito el valor mï¿½ximo
    PID_calc.I_term+=PID_calc.error;              //Acumulo el error
    if(PID_calc.I_term>PID_calc.I_termMAX) PID_calc.I_term=PID_calc.I_termMAX;    
    else if(PID_calc.I_term<(-1*PID_calc.I_termMAX)) PID_calc.I_term=(-1)*PID_calc.I_termMAX;
    else if(PID_calc.I_term<(-1*PID_calc.I_termMin)) PID_calc.I_term=(-1)*PID_calc.I_termMin;
    else if(PID_calc.I_term<PID_calc.I_termMin) PID_calc.I_term=PID_calc.I_termMin;
    //if(PID_calc.error>1000) {
    PID_calc.aux=PID_calc.I_term*K_PID.Ki;

        //Calculo parte Derivativa, y guardo el error como error anterior
    PID_calc.D_term=PID_calc.D_lastError-PID_calc.error;
    PID_calc.D_term*=K_PID.Kd;
    PID_calc.D_lastError=PID_calc.error;

        //sumo las tres partes para obtener el resultado final
    PID_calc.result=PID_calc.P_term+PID_calc.aux;
    if(VARIOS_flgs.Dterm) PID_calc.result+=PID_calc.D_term;
    //PID_calc.result/=K_PID.Kp;                    //Resultado PID en porcentaje
    /*} else {
        PID_calc.aux=PID_calc.I_term/K_PID.Ki2;

        //Calculo parte Derivativa, y guardo el error como error anterior
        PID_calc.D_term=PID_calc.D_lastError-PID_calc.error;
        PID_calc.D_term*=K_PID.Kd2;
        PID_calc.D_lastError=PID_calc.error;

        //sumo las tres partes para obtener el resultado final
        PID_calc.result=PID_calc.P_term+PID_calc.aux;
        if(include_D) PID_calc.result+=PID_calc.D_term;
        PID_calc.result/=K_PID.Kp2;                    //Resultado PID en porcentaje
    }*/
    PID_calc.result*=PID_TIME;
    //PID_calc.result/=100;    //resultado del PID en 0,001s de tiempo de encendido

#ifndef POWER_OUTPUT_C
    if(PID_calc.result<0) PID_calc.result=0;
#endif     
}

//*****************************************************************************
// Rutinas para grabado en EEPROM (flash) interna de parámetros y constantes
// de conversión.
//*****************************************************************************
//**************Grabo en memoria nueva Configuracion
//*****************************************************************************

    // Tiempo y temperatura default
void Save_ConfigEEPROM(void){

    INTERRUPT_GlobalDisable();

    Write_EEPROM(EEPROM_Time,TimeDefault.L);

    Write_EEPROM(EEPROM_Temp,TempDefault.L);
    
    INTERRUPT_GlobalEnable();

}

// Parámetros de calibración PID
void Save_KPID_EEPROM(void){
    INTERRUPT_GlobalDisable();

    flg_EE_fault=DEE_Write(EEPROM_PID_Kp,K_PID.KPL);

    flg_EE_fault=DEE_Write(EEPROM_PID_Ki,K_PID.KIL);

    flg_EE_fault=DEE_Write(EEPROM_PID_Kd,K_PID.KDL);
    
    INTERRUPT_GlobalEnable();

}

// Parámetros de conversión de temperatura
void Save_TempCorrection_EEPROM(void) {
    INTERRUPT_GlobalDisable();
    flg_EE_fault=DEE_Write(EEPROM_KTEMP,Convert.KtempL);
    flg_EE_fault=DEE_Write(EEPROM_CTEMP,Convert.CtempL);
    INTERRUPT_GlobalEnable();    
}

// Parámetros de conversión de temperatura de fábrica
void Save_TempFactCorrection_EEPROM(void) {
    INTERRUPT_GlobalDisable();
    flg_EE_fault=DEE_Write(EEPROM_KTEMPFact,Convert.KtempFL);
    flg_EE_fault=DEE_Write(EEPROM_CTEMPFact,Convert.CtempFL);
    INTERRUPT_GlobalEnable();    
}



//*****************************************************************************
// Chequeo de EEPROM inicial
void Check_EEPROM(void) {
    
    INTERRUPT_GlobalDisable();

    flg_EE_fault=0;
    EEPROM.ul_data=DEE_Read(EEPROM_Data);
    if(0!=EEPROM.data[0]) flg_EE_fault=1;          //EE_pointer=0 ï¿½ 4
    if(1!=EEPROM.data[1]) flg_EE_fault=1;          //EE_pointer=1 ï¿½ 5
    if(2!=EEPROM.data[2]) flg_EE_fault=1;          //EE_pointer=2 ï¿½ 6
    if(3!=EEPROM.data[3]) flg_EE_fault=1;          //EE_pointer=3 ï¿½ 7
    EEPROM.ul_data=DEE_Read(EEPROM_Data+4);
    if(4!=EEPROM.data[0]) flg_EE_fault=1;          //EE_pointer=0 ï¿½ 4
    if(5!=EEPROM.data[1]) flg_EE_fault=1;          //EE_pointer=1 ï¿½ 5
    if(6!=EEPROM.data[2]) flg_EE_fault=1;          //EE_pointer=2 ï¿½ 6
    if(7!=EEPROM.data[3]) flg_EE_fault=1;          //EE_pointer=3 ï¿½ 7

    //*************************************************************************
    //                          Error de EEPROM
    // Puede deberse a que es la primera vez que se programa el dispositivo
    //*************************************************************************
    if(flg_EE_fault) {                      //EEPROM Error
        EEPROM.data[0]=0;                     //EE_pointer=0 ï¿½ 4
        EEPROM.data[1]=1;                     //EE_pointer=1 ï¿½ 5
        EEPROM.data[2]=2;                     //EE_pointer=2 ï¿½ 6
        EEPROM.data[3]=3;                     //EE_pointer=3 ï¿½ 7
        flg_EE_fault=DEE_Write(EEPROM_Data,EEPROM.ul_data);

        EEPROM.data[0]=4;                     //EE_pointer=0 ï¿½ 4
        EEPROM.data[1]=5;                     //EE_pointer=1 ï¿½ 5
        EEPROM.data[2]=6;                     //EE_pointer=2 ï¿½ 6
        EEPROM.data[3]=7;                     //EE_pointer=3 ï¿½ 7
        flg_EE_fault=DEE_Write(EEPROM_Data+4,EEPROM.ul_data);

        TempDefault.i=500;
        TimeDefault.i=0;
        Save_ConfigEEPROM();

        Convert.K_temp=0.001206;//0.001098;//0.000916662;//0.000730519;
        flg_EE_fault=DEE_Write(EEPROM_KTEMP,Convert.KtempL);
        
        Convert.K_tempFactory=Convert.K_temp;
        flg_EE_fault=DEE_Write(EEPROM_KTEMPFact,Convert.KtempFL);

        Convert.C_temp=2900.055176;//2549.830078;//2597.403;//2664.1806089812;//
        flg_EE_fault=DEE_Write(EEPROM_CTEMP,Convert.CtempL);
        
        Convert.C_tempFactory=Convert.C_temp;
        flg_EE_fault=DEE_Write(EEPROM_CTEMPFact,Convert.CtempFL);
        
        Convert.K_LM334=1;
        flg_EE_fault=DEE_Write(EEPROM_KLM334,Convert.KLM334L);

        Convert.TEMP_MAX=3000;
        flg_EE_fault=DEE_Write(EEPROM_TMAX,Convert.TempMaxL);

        K_PID.Kp=0.06959;//0.005487;//0.019358;//151.7;//25.9;
        K_PID.Ki=0.01350;//0.000123;//0.00007;//1333.432;//833.58;//189.5;
        K_PID.Kd=1.255911;//1.097152;//170.5;//38.76;
        Save_KPID_EEPROM();
        
        VARIOS_flgs.Dterm=0;
        
#ifdef POWER_OUTPUT_A                           //If is a 100W oven, then disable Safeguard as default
        VARIOS_flgs.SafeGuard=0;
        VARIOS_flgs.SmartPID=1;
        VARIOS_flgs.WrmpUP=1;
#endif
    
#ifdef POWER_OUTPUT_B                           //If it's 1kW oven, enable Safeguard as default
        VARIOS_flgs.SafeGuard=1;
        VARIOS_flgs.SmartPID=1;
        VARIOS_flgs.WrmpUP=1;
#endif
                
#ifdef POWER_OUTPUT_C                            //If it's Peltier oven
        VARIOS_flgs.SafeGuard=0;
        VARIOS_flgs.SmartPID=0;
        VARIOS_flgs.WrmpUP=0;
#endif

        flg_EE_fault=DEE_Write(EEPROM_VariosFlgs,VARIOS_flgs.L);
        
        
        calentar.i_SetPointMargin=50;            //0.5 degree difference marks the start of regulated regim.
        calentar.i_safelimit=30;
        flg_EE_fault=DEE_Write(EEPROM_Margin,calentar.ul_LimitAndSPMargin);
        
    }

    //*****************************EEPROM OK - Restauro configuraciï¿½n
    else {                    //EEPROM OK

        //********************************** Leo Config
        TimeDefault.L=DEE_Read(EEPROM_Time);
        
        TempDefault.L=DEE_Read(EEPROM_Temp);

        Convert.KtempL=DEE_Read(EEPROM_KTEMP);
                
        Convert.KtempFL=DEE_Read(EEPROM_KTEMPFact);
        
        Convert.CtempL=DEE_Read(EEPROM_CTEMP);
        
        Convert.CtempFL=DEE_Read(EEPROM_CTEMPFact);

        Convert.KLM334L=DEE_Read(EEPROM_KLM334);

        Convert.TempMaxL=DEE_Read(EEPROM_TMAX);
                
        K_PID.KPL=DEE_Read(EEPROM_PID_Kp);

        K_PID.KIL=DEE_Read(EEPROM_PID_Ki);

        K_PID.KDL=DEE_Read(EEPROM_PID_Kd);
        
        VARIOS_flgs.L=DEE_Read(EEPROM_VariosFlgs);

        calentar.ul_LimitAndSPMargin=DEE_Read(EEPROM_Margin);
        
        //*******************************************************************
    }

    //************************************************************************
    SNPointer=SerialNumber+0x80000000;
    SN.L=*SNPointer;    
    /*SN.c[0]=SNPointer;
    SNPointer++;
    SN.c[1]=SNPointer;
    SNPointer++;
    SN.c[2]=SNPointer;
    SNPointer++;
    SN.c[3]=SNPointer;*/
    
    INTERRUPT_GlobalEnable();
}


//*****************************************************************************
void Write_EEPROM(unsigned long Address, unsigned long Value){
    EE_error=0;
    do{
        flg_EE_fault=DEE_Write(Address,Value);
        EE_error++;
        if(EE_error>4) break;
    } while(flg_EE_fault);
};


//******************************************************************************
//                     UART commands processing
//
//
//******************************************************************************
void ProcessUART(void) {

    //sprintf(st_UART,"\n\rA: %d -",uc_RX_Pointer);

    uc_RX_Pointer=strlen(st_UART_Rx);
    if(st_UART_Rx[0]=='a' && st_UART_Rx[uc_RX_Pointer-1]=='+') {
        strncpy(st_aux,&st_UART_Rx[1],uc_RX_Pointer-2);
        ui_Pointer=strchr(st_aux,',');
        if(ui_Pointer){              //Hay una coma en el mensaje, hay parï¿½metros
            flg_parameter=1;
        } else {                        //no hay coma, no hay parï¿½metros, simplemente pregunto por algï¿½n valor
            flg_parameter=0;
        }

        //****************************************Valor del parï¿½metro K de temperatura
        if(strstr(st_aux,"ktemp")){
            if(flg_parameter){        //Hay parï¿½metros, modifico el valor
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                Convert.K_temp=strtof(st_aux2,&ui_Pointer);
                Save_TempCorrection_EEPROM();
            }
            sprintf(st_UART,"KT:%f\n\r\n",Convert.K_temp);
            flg_send=1;
        }
        //****************************************Valor del parï¿½metro K de temperatura de fábrica
        else if(strstr(st_aux,"fktmp")){                            //modifies or reads Ktemp Factory value
            if(flg_parameter){        //Hay parï¿½metros, modifico el valor
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                Convert.K_tempFactory=strtof(st_aux2,&ui_Pointer);          //updates KtempFactory calibration
                Save_TempFactCorrection_EEPROM();                           //Saves KtempFactory to emulated eeprom
            }
            sprintf(st_UART,"KTf:%f\n\r\n",Convert.K_tempFactory);
            flg_send=1;
        }
        //****************************************Valor del parï¿½metro C de temperatura
        else if (strstr(st_aux,"ctemp")){                   //modifies or reads Ctemp value
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                Convert.C_temp=strtof(st_aux2,&ui_Pointer);     //updates Ctemp calibration
                Save_TempCorrection_EEPROM();                   //saves Ctemp calibration to emulated eeprom
            }
            sprintf(st_UART,"CT:%f\n\r\n",Convert.C_temp);
            flg_send=1;
        }
        //****************************************Valor del parï¿½metro C de temperatura de fábrica
        else if (strstr(st_aux,"fctmp")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                Convert.C_tempFactory=strtof(st_aux2,&ui_Pointer);          //updates factory calibration of Ctemp
                Save_TempFactCorrection_EEPROM();                   //saves factory calibration to emulated eeprom
            }
            sprintf(st_UART,"CTf:%f\n\r\n",Convert.C_tempFactory);
            flg_send=1;
        }
        //****************************************Restaura calibración de fábrica
        else if(strstr(st_aux,"restore")) {
            Convert.K_temp=Convert.K_tempFactory;           //restores Ktemp to factory calibration
            Convert.C_temp=Convert.C_tempFactory;           //restores Ctemp to factory calibration
            Save_TempCorrection_EEPROM();                   //Saves restored calibration to emulated eeprom
            sprintf(st_UART,"RestoreOK\n\r");
            flg_send=1;
        }
        //****************************************Valor del parï¿½metro K del LM334
        else if (strstr(st_aux,"klm")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                Convert.K_LM334=strtof(st_aux2,&ui_Pointer);
            }
            sprintf(st_UART,"KLM:%f\n\r\n",Convert.K_LM334);
            flg_send=1;
        }
        //****************************************PID AutoTuning
        else if(strstr(st_aux,"PIDAT")){
            PIDAT.flg_NOAT=0;
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                PIDAT.uc_secAT=atoi(st_aux2);
                PIDAT.uc_secAT--;
            } else {
                PIDAT.ui_secCount=0;
            }
            if(uc_estado==state_Config) {
                ChangeState(state_PIDAT);
            }
        }
        //****************************************PIDAT_2 (PIDAT sin cï¿½lculo)
        else if(strstr(st_aux,"PID2AT")){
            if(uc_estado==state_PIDAT) ChangeState(state_Config);
            else {
                PIDAT.flg_NOAT=1;
                if(flg_parameter){
                    strcpy(st_aux2,++ui_Pointer);
                    ui_Pointer=strlen(st_aux2);
                    PIDAT.uc_secAT=atoi(st_aux2);
                    PIDAT.uc_secAT--;
                } else {
                    PIDAT.ui_secCount=0;
                }
                if(uc_estado==state_Config) ChangeState(state_PIDAT);
            }
        }
        //****************************************Activate Debug Mode
        else if(strstr(st_aux,"debug")){
            flg_debug=~flg_debug;
        }
        //****************************************Valor del tï¿½rmino K de PID
        else if(strstr(st_aux,"KpPID")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                K_PID.Kp=strtof(st_aux2,&ui_Pointer);
                Save_KPID_EEPROM();
            }
            sprintf(st_UART,"Kp_PID:%f\n\r\n",K_PID.Kp);
            flg_send=1;
        }
        //****************************************Valor del tï¿½rmino I de PID
        else if(strstr(st_aux,"KiPID")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                K_PID.Ki=strtof(st_aux2,&ui_Pointer);
                Save_KPID_EEPROM();
            }
            sprintf(st_UART,"Ki_PID:%f\n\r\n",K_PID.Ki);
            flg_send=1;
        }
        //****************************************Valor del tï¿½rmino D de PID
        else if(strstr(st_aux,"KdPID")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                K_PID.Kd=strtof(st_aux2,&ui_Pointer);
                Save_KPID_EEPROM();
            }
            sprintf(st_UART,"Kd_PID:%f\n\r\n",K_PID.Kd);
            flg_send=1;
        }
        //****************************************Enable/Disable D de PID
        else if(strstr(st_aux,"Dterm")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                VARIOS_flgs.Dterm=atoi(st_aux2);
                INTERRUPT_GlobalDisable();
                flg_EE_fault=DEE_Write(EEPROM_VariosFlgs,VARIOS_flgs.L);                        
                INTERRUPT_GlobalEnable();
            }
            sprintf(st_UART,"Dterm:%d\n\r\n",VARIOS_flgs.Dterm);
            flg_send=1;
        }
        //****************************************Activo/Desactivo SmartPID
        else if(strstr(st_aux,"smartPID")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                VARIOS_flgs.SmartPID=atoi(st_aux2);
                INTERRUPT_GlobalDisable();
                flg_EE_fault=DEE_Write(EEPROM_VariosFlgs,VARIOS_flgs.L);
                INTERRUPT_GlobalEnable();
            }
            sprintf(st_UART,"smartPID:%d\n\r\n",VARIOS_flgs.SmartPID);
            flg_send=1;
        }
        //****************************************Activo/Desactivo SafeGuard
        else if(strstr(st_aux,"safeguard")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                VARIOS_flgs.SafeGuard=atoi(st_aux2);
                INTERRUPT_GlobalDisable();
                flg_EE_fault=DEE_Write(EEPROM_VariosFlgs,VARIOS_flgs.L);
                INTERRUPT_GlobalEnable();
            }
            sprintf(st_UART,"Safeguard:%d\n\r\n",VARIOS_flgs.SafeGuard);
            flg_send=1;
        }
        //****************************************Valor de SafeGuard Limit
        else if(strstr(st_aux,"sfglimit")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                //ui_Pointer=strlen(st_aux2);
                calentar.i_safelimit=atoi(st_aux2)*10;
                INTERRUPT_GlobalDisable();
                flg_EE_fault=DEE_Write(EEPROM_Margin,calentar.ul_LimitAndSPMargin);
                INTERRUPT_GlobalEnable();
            }
            sprintf(st_UART,"Safeguard Limit:%d\n\r\n",calentar.i_safelimit/10);
            flg_send=1;
        }
        //****************************************Valor de Margen de SetPoint
        else if(strstr(st_aux,"SPmargin")){
            flg_EE_fault=0;
            if(flg_parameter) {
                strcpy(st_aux2,++ui_Pointer);
                calentar.i_SetPointMargin=atoi(st_aux2);   //0.5 degree difference marks the start of regulated regim.
                INTERRUPT_GlobalDisable();            
                flg_EE_fault=DEE_Write(EEPROM_Margin,calentar.ul_LimitAndSPMargin);
                INTERRUPT_GlobalEnable();
            }
            sprintf(st_UART,"SetPointMargin:%d\n\r",calentar.i_SetPointMargin/10);
            flg_send=1;
        }
        //****************************************Multiplicador de WarmUp
        else if(strstr(st_aux,"wrmupK")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                f_warmupK=strtof(st_aux2,&ui_Pointer);
            }
            sprintf(st_UART,"WarmUp_K:%f\n\r\n",f_warmupK);
            flg_send=1;
        }
        //****************************************Enable/disable Warmup
        else if(strstr(st_aux,"wrmup")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                VARIOS_flgs.WrmpUP=atoi(st_aux2);
                INTERRUPT_GlobalDisable();
                flg_EE_fault=DEE_Write(EEPROM_VariosFlgs,VARIOS_flgs.L);
                INTERRUPT_GlobalEnable();
            }
            sprintf(st_UART,"WarmUp:%d\n\r\n",VARIOS_flgs.WrmpUP);
            flg_send=1;
        }
        //****************************************Start Process
        else if(strstr(st_aux,"startPr")){
            if(uc_estado==state_Config) {
                ChangeState(state_Process);
                sprintf(st_UART,"Start_Heating\n\r");
                flg_send=1;
            } else {
                sprintf(st_UART,"NotConfig\n\r");
                flg_send=1;
            }
        }
        //****************************************StopProcess
        else if(strstr(st_aux,"stopPr")){
            if(uc_estado==state_Process) ChangeState(state_Config);
            sprintf(st_UART,"Stop_Heating\n\r");
            flg_send=1;
        }
        //****************************************TempSet
        else if(strstr(st_aux,"tempSet")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                calentar.tempSet=atoi(st_aux2);
            }
            sprintf(st_UART,"TempSet:%d\n\r",calentar.tempSet);
            flg_send=1;
        }
        //****************************************Timeset
        else if(strstr(st_aux,"timeSet")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                calentar.timeSet=atoi(st_aux2);
            }
            calentar.timeLeft=calentar.timeSet;
            sprintf(st_UART,"TimeSet:%d\n\r",calentar.timeSet);
            flg_send=1;
        }
        //****************************************Exit
        else if(strstr(st_aux,"exit")){
            ChangeState(state_Config);
        }
        //****************************************Estado actual del equipo
        else if(strstr(st_aux,"state")){
            sprintf(st_UART,"State:%d\n\r",uc_estado);
                flg_send=1;
        }
        //****************************************Calibraciï¿½n 1 punto
        else if(strstr(st_aux,"cal1")){
            if(uc_estado==state_Config){
                ChangeState(state_TwoPointCal);
                sprintf(st_UART,"Cal2pts\n\r");
                flg_send=1;
            } else {
                sprintf(st_UART,"NotConfig\n\r");
                flg_send=1;
            }
        }
        //****************************************Calibraciï¿½n Multi punto
        else if(strstr(st_aux,"cal2")){
            if(uc_estado==state_Config){
                ChangeState(state_MultiPointCal);
                
            /*} else if (uc_estado==state_MultiPointCal) {
                sprintf(st_UART,"CalMPnt\n\r");
                flg_send=1;*/
            } else {
                sprintf(st_UART,"NotConfig\n\r");
                flg_send=1;
            }
        }
        //****************************************Temperatura de calibraciï¿½n
        else if(strstr(st_aux,"tcal")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                ui_Pointer=strlen(st_aux2);
                calibration.f_newData=strtof(st_aux2,&ui_Pointer);
                calibration.flg_newData=1;
            }
            sprintf(st_UART2,"Cal:%.0f\n\r", calibration.f_newData);
            flg_send2=1;
        }
        //****************************************Fin_CalMultiPunto
        else if(strstr(st_aux,"CalEnd")){
            if(uc_estado==state_MultiPointCal) calibration.flg_CalEnd=1;
            else {
                sprintf(st_UART2,"No estaba calibrando");
                flg_send2=1;
            }
        }
        
        //****************************************Datalog set interval
        //if time interval is different than 0, then it autostarts.
        else if(strstr(st_aux,"dtlgint")) {
            if(flg_parameter) {
                strcpy(st_aux2,++ui_Pointer);
                datalog.tiempo=atoi(st_aux2);    //tiempo asignado de intervalo.
                datalog.timeout=datalog.tiempo;     //timeout - Cuenta regresiva para cada toma de datos.
                datalog.pointer=0;          //starts at the beggining of the data array
                datalog.download=0;
                datalog.record=1;
            }
            sprintf(st_UART,"dtlgint:%d",datalog.tiempo);
            flg_send=1;
        }
        
        //****************************************Datalog download values
        else if(strstr(st_aux,"dtlgdwnld")) {
            if(datalog.pointer) {               //there are some data stored, begins download
                datalog.download=1;
                datalog.pointer2=0;
                datalog.record=0;  
                sprintf(st_download,"I:%d;Data:%d\n\r",datalog.tiempo,datalog.pointer);
                datalog.flg_send=1;
            } else {
                sprintf(st_UART,"NoData");
                flg_send=1;
            }
        }
        
        else if(strstr(st_aux,"dtlgnext")) {
            datalog.next=1;
        }
        //****************************************Test device   
        else if(strstr(st_aux,"test")){
            ui_test=10000;
            sprintf(st_UART,"TEST\n\r");
            flg_send=1;
        }
        //****************************************Versiï¿½n de Firmware
        else if(strstr(st_aux,"version")){
            sprintf(st_UART,"Firmware Version: %d.%d\n\r",versionA,versionB);
            flg_send=1;
        }
        //****************************************Nï¿½mero de Serie
        else if(strstr(st_aux,"serial")){
            sprintf(st_UART,"Serial Number: %04X%08X\n\r",SModel,SN.L);//SNumber);
            strcat(st_UART,SN.string);
            flg_send=1;
        }
        //****************************************Device type
        else if(strstr(st_aux,"device")){
            sprintf(st_UART,"TDuC_v1\n\r");
            flg_send=1;
        }
#ifdef POWER_OUTPUT_C
        //****************************************EXCLUSIVE FOR PELTIER OVENS
        //**************************************CoolDown
        else if(strstr(st_aux,"cooldown")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                l_PeltierAux=atoi(st_aux2);
                if(l_PeltierAux>0xFFFF) Peltier_HowMuch=0xFFFF;
                else Peltier_HowMuch=l_PeltierAux;
                CoolDown(Peltier_HowMuch);
            }
            sprintf(st_UART,"CoolDown:%d\n\r",CoolingPWM);
            flg_send=1;
        }
        //**************************************HeatUp
        else if(strstr(st_aux,"heatup")){
            if(flg_parameter){
                strcpy(st_aux2,++ui_Pointer);
                l_PeltierAux=atoi(st_aux2);
                if(l_PeltierAux>0xFFFF) Peltier_HowMuch=0xFFFF;
                else Peltier_HowMuch=l_PeltierAux;
                HeatUp(Peltier_HowMuch);
            }
            sprintf(st_UART,"HeatUP:%d\n\r",HeatingPWM);
            flg_send=1;
        }
        //****************************************StopHeating
        else if(strstr(st_aux,"peltieroff")){
            PeltierOff();
            sprintf(st_UART,"StopPeltier");
            flg_send=1;
        }
#endif
        //****************************************Error de comando
        else {
            sprintf(st_UART,"Comando erroneo\n\r");
            flg_send=1;
        }
    } else {
        if(USBUSARTIsTxTrfReady()) {
            putrsUSBUSART("Error\n\r");
        }
    }


    //uc_RX_bytes=strlen(st_UART_Rx);
    for(uc_aux=0;uc_aux<25;uc_aux++){
        //if(uc_aux<5) st_RX_Buffer[uc_aux]=0;
        st_UART_Rx[uc_aux]=0;
        if(!flg_send) st_UART[uc_aux]=0;
        st_aux[uc_aux]=0;
        st_aux2[uc_aux]=0;
    }
    uc_RX_Pointer=0;

};

//***********************************************************************************************
// Autotune de PID - utiliza cálculo de Zieger-Nichols?
//***********************************************************************************************
void PID_Autotune(void) {
    
    if(tecla.tiempo&&!tecla.estado) {        //Si suelto el pulsador
        if((tecla.cual==BT_Encoder)&&(tecla.tiempo<20)) {      //Habï¿½a apreatado el encoder
            tecla.procesado=1;
            tecla.cual=nada;
            if(flg_abort){                      //Si había sostenido el cancel para salir
                calentar.enableOutput=0;        //termina el proceso
                flg_abort=0;
                ChangeState(state_Config);      //Y vuelve a config
            } 
        }
        else if((tecla.cual==BT_Cancel)&&(tecla.tiempo<20)) {      //Habï¿½a apreatado cancel
            flg_abort=0;            //cancelo el deseo de abortar proceso
            tecla.procesado=1;
            tecla.cual=nada;
        }

    } else if((tecla.cual==BT_Cancel)&&(tecla.tiempo>30)) {      //Mantengo apretado Cancel
        flg_abort=1;                //Quiero cancelar el proceso?
        tecla.procesado=1;
        tecla.cual=nada;
    }
    
    
    if(PIDAT.flg_1sec) {
        PIDAT.flg_1sec=0;         //Ya procesï¿½ la mediciï¿½n actual
        PIDAT.f_slope=PIDAT.f_tempPIDautotune-PIDAT.f_tempPIDlast;
        
        if(!PIDAT.flg_NOAT) {               //if flg_NOAT = 0 -> it has to do AutoTuning
            if(PIDAT.f_slope>=PIDAT.f_PIDATslope) {                  //Se entrï¿½ una pendiente mayor
                PIDAT.f_PIDATslope=PIDAT.f_slope;                   //Se registra
                PIDAT.f_tempInflexion=PIDAT.f_tempPIDautotune;      //y se guarda la temperatura
                PIDAT.ui_t_inflexion=PIDAT.ui_tiempo_actual;        //el tiempo en que ocurriï¿½
                PIDAT.ui_t_inflexion_1=PIDAT.ui_tiempoAnterior;     //ui_tiempo_actual-PID_TIME;     //el tiempo anterior
                //ui_t_inflexion_1--;
                PIDAT.f_tempInflexion_1=PIDAT.f_tempPIDlast;            //Y la temperatura anterior
                PIDAT.ui_AT_end=0;                //Se reinicia el contador de pendientes
            } else {
                PIDAT.ui_AT_end++;
            }

            if(PIDAT.ui_AT_end>PIDAT.ui_t_END) {
                PIDAT.f_PIDATaux=PIDAT.ui_t_inflexion-PIDAT.ui_t_inflexion_1;
                if(PIDAT.f_PIDATaux) PIDAT.f_PIDATslope/=PIDAT.f_PIDATaux;
                PIDAT.f_PIDATslope/=10;
                PIDAT.f_Cslope=PIDAT.f_PIDATslope*PIDAT.ui_t_inflexion;
                PIDAT.f_tempInflexion/=10;
                PIDAT.f_Cslope-=PIDAT.f_tempInflexion;
                PIDAT.f_Td=PIDAT.f_PIDATslope/PIDAT.f_Cslope;
                PIDAT.f_PIDATaux=10*PIDAT_Output;
                PIDAT.f_Rr=PIDAT.f_PIDATaux/PIDAT.f_PIDATslope;

                
                K_PID.Kp=0.000519481*PIDAT.f_Td;      //0.002597403*f_Td;  //0.012987013*f_Td;
                K_PID.Kp*=PIDAT.f_Rr;
                if(K_PID.Kp<0) K_PID.Kp*=-1;

                PIDAT.f_PIDATaux=PIDAT.f_Td*PIDAT.f_Td;
                K_PID.Ki=PIDAT.f_PIDATaux/169.4;//1694;          //169.4;
                K_PID.Ki*=PIDAT.f_Rr;

                K_PID.Kd=0.005844156*PIDAT.f_Rr;

                Save_KPID_EEPROM();
                
                //sprintf(st_UART,"FIN -t:%ds - T:%.2f - Sl:%.2f\n\r",ui_t_inflexion,f_tempInflexion,f_PIDATslope);
                //flg_send=1;
                //sprintf(st_UART2,"PIDAT_Kp:%f:Ki:%f:Kd:%f\n\rTd:%f:Rr:%f\n\r",K_PID.Kp,K_PID.Ki,K_PID.Kd,PIDAT.f_Td,PIDAT.f_Rr);
                //flg_send2=1;
                ChangeState(state_Config);
            }
        }       //otherwhise, it just heats up at a constant rate, and as to be stoped manually.

        PIDAT.f_tempPIDlast=PIDAT.f_tempPIDautotune;
        PIDAT.f_slopeLast=PIDAT.f_slope;
        PIDAT.ui_tiempoAnterior=PIDAT.ui_tiempo_actual;
        //Pasa los valores actuales a los registros de "anterior" para la proxima medicion


        //sprintf(st_UART,"t:%d:T:%.2f:Sl:%.2f\n\r",PIDAT.ui_tiempo_actual,PIDAT.f_tempPIDautotune,PIDAT.f_slope);
        //flg_send=1;

    }
}

//*****************************************************************************
//          SmartPID();
//
//  Controla si hay oscilaciï¿½n en la seï¿½al, y registra los picos de error
// y tiempos en los que estos ocurren. Despuï¿½s de 2 perï¿½odos de oscilaciï¿½n,
// calcula la reducciï¿½n del termino Ki, y continï¿½a controlando
//
//*****************************************************************************
void SmartPID(void) {

    //Parte A - Chequea si llegó a régimen dentro de un límite de tiempo
    //considerable, o si la respuesta es demasiado lenta
    
    //Error menor a medio grado, se considera que llegï¿½ a temperatura
    
    if(f_temp>f_tempSet) {      //if it's above set temperature.        
        //increases reachTIMEOUT, to "reset" the entrance to this portion of SmartPID       
        smartPID.ul_reachTIMEOUT=calentar.ul_processCount+smartPID.ul_reachTIMEOUT_default;           //updates reachout timeout
        if(calentar.flg_llego) smartPID.ul_reachTIMEOUT+=smartPID.ul_reachTIMEOUT_default;          //if it has already reached temp. range, then it waits twice as long to check reachout
        smartPID.flg_checkReach=0;              //disables checkReach because it's above set temperature.              
    } else smartPID.flg_checkReach=1;           //is below set temperature, enables reachout check

    
    if(smartPID.flg_checkReach && (calentar.ul_processCount>smartPID.ul_reachTIMEOUT)) {            //todavï¿½a no llegï¿½ a rï¿½gimen
        //Has exceeded reachTIMEOUT (processCount > reachTIMEOUT) 
        
        //calculates average error in the period elapsed
        smartPID.ul_aux=smartPID.ul_reachTIMEOUT-smartPID.ul_lastReachCheck;     // takes into account the period of time elapsed
        if(smartPID.ul_aux) {                       //checks to prevent division by 0
            smartPID.ul_errorAvg/=(smartPID.ul_aux);                 //divides by a quarter of the time.
        }   else {
            smartPID.ul_errorAvg=0;
        }
                        
        //and modifies Kp or Ki to make it heat up a bit faster.
        if(smartPID.ul_errorAvg>200) {           //AvgError es mayor a 20 grados, incremento ambos Kp y Ki
            smartPID.f_aux=PID_calc.error/f_tempSet;
            smartPID.f_aux/=10;
            smartPID.f_aux+=1;
            K_PID.Kp*=smartPID.f_aux;
            K_PID.Ki*=smartPID.f_aux;
            Save_KPID_EEPROM();
        }
        else if(smartPID.ul_errorAvg>100) {       //faltan mas de 10 grados, incremento Kp
            smartPID.f_aux=PID_calc.error/f_tempSet;
            smartPID.f_aux/=10;
            smartPID.f_aux+=1;
            K_PID.Kp*=smartPID.f_aux;
            //sprintf(st_UART,"SMART PID -> Kp:%f\n\r\n",K_PID.Kp);
            //flg_send=1;
            Save_KPID_EEPROM();
        } else if (smartPID.ul_errorAvg>50) {//Falta mas de 0.3 grados, pero menos de 10, incremento Ki
            smartPID.f_aux=PID_calc.error/f_tempSet;
            smartPID.f_aux/=10;
            smartPID.f_aux+=1;
            K_PID.Ki*=smartPID.f_aux;
            //sprintf(st_UART,"SMART PID -> Ki:%f\n\r\n",K_PID.Ki);
            //flg_send=1;
            Save_KPID_EEPROM();
        }
        
        //increases reachTIMEOUT, to "reset" the entrance to this portion of SmartPID       
        smartPID.ul_reachTIMEOUT=calentar.ul_processCount+smartPID.ul_reachTIMEOUT_default;
        if(calentar.flg_llego) smartPID.ul_reachTIMEOUT+=smartPID.ul_reachTIMEOUT_default;          //if it has already reached temp. range, then it waits twice as long to check reachout
        smartPID.ul_lastReachCheck=calentar.ul_processCount;            //stores position of last calculation
        smartPID.ul_errorAvg=0;
        
    } //else {                    //Llegï¿½ a rï¿½gimen (T-0.5ï¿½C)


    
    //Parte B, chequea oscilación en la salida chequeando las pedientes instantáneas
    
    smartPID.f_currentSlope=f_temp-smartPID.f_tempAnt;      //calculates current slope. f_temp 15 = 1.5º
    smartPID.f_tempAnt=f_temp;                              //stores current temperature as previous temperature for future slope calculation
    //if(smartPID.f_currentSlope<0) smartPID.f_currentSlope*=-1;          //if it's negative, compute absolute value.
    
    if(smartPID.f_currentSlope>=1) {                //current slope is bigger than 0.1ºC (is rising)
        if(!smartPID.flg_rising) {               //if it's NOT rising
            smartPID.uc_signCount++;            //counts positive slope, to register change in slope
        } else smartPID.uc_signCount=0;
    } else if (smartPID.f_currentSlope<=-1) {       //current slope is below -0.1ºC (is falling)
        if(smartPID.flg_rising) {               //if it's rising
            smartPID.uc_signCount++;            //counts negative slope, to  register change in slope
        } else smartPID.uc_signCount=0;
    } else {                                        //current slope is near top or bottom peak, slope absolute value is belo 0.1º
        if(smartPID.flg_rising) {                   //if it was rising, and slope is stuck, searches for peak value
            if(f_temp>smartPID.f_peakMaxRising) {           //current temperature is above max registered temp.
                smartPID.f_peakMaxRising=f_temp;                //Register new value for peak temp.
                smartPID.ul_tMaxRising=calentar.ul_processCount;        //register time at peak temp.                
            }
        } else {                                    //if it was falling, and slope is stuck, searches for min  peak vallue
            if(f_temp<smartPID.f_peakMaxFalling) {              //current temp. is below min peak
                smartPID.f_peakMaxFalling=f_temp;               //registers new value for min peak.
                smartPID.ul_tMaxFalling=calentar.ul_processCount;       //registeres time at min peak temp.
            }
        }
    }
    
    if(smartPID.uc_signCount>signChange) {          //enough sign counts to determine a lobe change
        smartPID.uc_lobeCount++;                    //go to next lobe
        smartPID.uc_signCount=0;                    //reset sign count for next lobe
        smartPID.flg_rising=~smartPID.flg_rising;           //change in direction of slope
        //if(smartPID.flg_rising) smartPID.flg_rising=0;      //if it was rising, now it's falling
        //else smartPID.flg_rising=1;                         //else, it was falling, so now it's rising
    }

    if(smartPID.uc_lobeCount==4) {                  //went over 2 periods of oscillation
        
        smartPID.f_aux=smartPID.f_peakMaxRising-smartPID.f_peakMaxFalling;          //checks how much amplitude it got from max peak compared to min peak
        
        if(smartPID.f_aux>e_permitido){                 //If max peak amplitude is above allowed A value             
            //smartPID.f_aux=smartPID.f_peakMaxRising-smartPID.f_peakMaxFalling;      //calculates total oscillation span
            smartPID.f_aux/=f_tempSet;                  //divides value by set temperature
            smartPID.f_aux+=1;                          //Adds 1, so it's 1.XXX value, to multiply
            K_PID.Kp/=smartPID.f_aux;               //modifies Kp accordingly (decreases value)
            //sprintf(st_UART2,"SMART PID Ovsh-> Kp:%f\n\r\n",K_PID.Kp);          //prints new value to USB 
            //flg_send2=1;
            Save_KPID_EEPROM();                 //Stores new value in EEPROM (FLASH)
        } else if (smartPID.f_aux>e_permitido2) {       //Overshoot is above 0.5º
            //smartPID.f_aux=smartPID.f_peakMaxRising-smartPID.f_peakMaxFalling;      //calculates total oscillation span
            f_aux=PID_calc.I_termMAX/4.0;
            f_aux*=3;               //computes 3/4 of Iterm Max allowed value.
            if(PID_calc.I_term>f_aux) {               //if current i_term is above 3/4 of max, and it oscillates,
                PID_calc.I_term/=4.0;          //reduces I_term to 3/4 of current value
                PID_calc.I_term*=3.0;
            } else {            //I term is not that big, yet system oscillates
                smartPID.f_aux/=f_tempSet;    //divides value by set temperature
                smartPID.f_aux+=1;
                K_PID.Ki/=smartPID.f_aux;                   //modifies Ki accordingly (slightly decreases value)
                //sprintf(st_UART2,"SMART PID Ovsh-> Ki:%f\n\r\n",K_PID.Ki);          //prints new value to USB 
                //flg_send2=1;
                Save_KPID_EEPROM();                 //Stores new value in EEPROM (FLASH)
            }
        }      
        //finishes calculation, resets values to default for next period calculation
        smartPID.uc_lobeCount=0;                    //resets to 0
        smartPID.f_peakMaxRising=0;                 //Max temperature for peak at rising edge set to 0º
        smartPID.f_peakMaxFalling=5000;             //Min peak for falling edge set to 500º (above any reading, so it's ovewriten)
    }
    
    
    //*******Parte C - Check for overshoot at first lobe
    //Pasï¿½ un tiempo determinado, deberï¿½a haber encontrado un overshoot
    //o controla si llegï¿½ a temperatura, para evaluar modificar el Kp
    /*if(ui_SPIDcounter>processOvershoot){

        if(uc_lobeCount>0) {      //Llegï¿½ a pasar la temperatura seteada
            //flg_inicio=0;
            e_overshoot*=-1;
            if(e_overshoot>e_permitido){     //superï¿½ el error permitido para overshoot
                e_overshoot/=100;
                e_overshoot+=1;
                K_PID.Kp/=e_overshoot;
                e_overshoot=0;
                sprintf(st_UART,"SMART PID Ovsh-> Kp:%f\n\r\n",K_PID.Kp);
                flg_send=1;
            }
        } else if(uc_lobeCount==0) {        //Nunca llegï¿½ a temperatura

        }

        //if(uc_lobeCount>5);

        ui_SPIDcounter=0;
    }*/


    //Controla los errores de oscilaciï¿½n, y registra los valores de pico, y los
    //tiempos en los que ocurren, para calcular amplitud y frecuencia de la
    //oscilaciï¿½n de temperatura, y modificar Ki

}


void CheckOvershoot(void){
    if(PID_calc.error<0) smartPID.f_aux=-1*PID_calc.error;
    else smartPID.f_aux=PID_calc.error;

    if(smartPID.f_aux>smartPID.e_overshoot) {
        smartPID.e_overshoot=smartPID.f_aux;
        smartPID.ul_tOvershoot=calentar.ul_processCount;
        smartPID.ul_overshoot_timeout=0;
        smartPID.flg_overshoot=1;
    } else {
        smartPID.ul_overshoot_timeout++;
    }

    if(smartPID.ul_overshoot_timeout>smartPID.ul_overshoot_MAX) { //Ya encontrï¿½ el mï¿½ximo, ajusta Kp
        if(!smartPID.flg_overshootCorrection){
            /*if(e_overshoot>e_permitido){     //superï¿½ el error permitido para overshoot
                e_overshoot/=10000;
                e_overshoot+=1;
                K_PID.Kp/=e_overshoot;
                K_PID.Ki*=e_overshoot;
                e_overshoot=0;
                ul_overshoot_timeout=0;
                flg_overshoot=0;
                sprintf(st_UART2,"SMART PID Ovsh-> Kp:%f\n\rSMART PID Ovsh-> Ki:%f\r\n",K_PID.Kp,K_PID.Ki);
                flg_send2=1;
            }*/
            if(smartPID.e_overshoot>e_permitido2){     //superï¿½ el error permitido para overshoot
                smartPID.e_overshoot/=10000;
                smartPID.e_overshoot+=1;
                K_PID.Kp/=smartPID.e_overshoot;
                smartPID.e_overshoot=0;
                smartPID.ul_overshoot_timeout=0;
                sprintf(st_UART2,"SMART PID Ovsh-> Kp:%f\n\r\n",K_PID.Kp);
                flg_send2=1;
                Save_KPID_EEPROM();
            } else if(smartPID.e_overshoot>e_permitido){     //superï¿½ el error permitido para overshoot
                smartPID.e_overshoot/=10000;
                smartPID.e_overshoot+=1;
                K_PID.Ki*=smartPID.e_overshoot;
                smartPID.e_overshoot=0;
                smartPID.ul_overshoot_timeout=0;
                sprintf(st_UART2,"SMART PID Ovsh-> Ki:%f\n\r\n",K_PID.Ki);
                flg_send2=1;
                Save_KPID_EEPROM();
            }
            smartPID.flg_overshootCorrection=1;
        }
    }
}





// transmit the ws2812 led
void ws2812_send(ws2812_ptr* led) {
    int j;
    long int val;
    // the WS2812 wants bits in the order of:
    // GGGGGGGGRRRRRRRRBBBBBBBB
    // but I want to work in the opposite order. so i'm going to flip
    // the bits around and do some shifting so my order is
    // BBBBBBBBRRRRRRRRGGGGGGGG
    // with the most significant bit on the far right. so the RGB value
    // of 128 64 32, which normally would be:
    // R : 0b10000000
    // G : 0b01000000
    // B : 0b00100000
    // will become:
    // BBBBBBBBRRRRRRRRGGGGGGGG
    // 000001000000000100000010
    val = (bitflip(led->b) << 16) + (bitflip(led->r) << 8) + (bitflip(led->g));

    // now begin shifting them over one at a time
    for(j = 0; j < 24; j++) {
        // depending on if the currently viewed bit is 1 or 0
        // the pin will stay high for different times

        if (val & 1 == 1) {
            // if it is a 1, let it stay higher a bit longer
            W2812 = 1;
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();

            W2812 = 0;
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();


        } else {
            // but a 0 should go high and then low as fast as possible
            W2812 = 1;
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();

            W2812 = 0;

            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();
            Nop();

        }

        // and then right shift to get the next bit
        val = val >> (unsigned char)1;
    }
}

void ws2812_Rst (void) {
    W2812 = 1;
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();

    W2812 = 0;

    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();

    W2812 = 1;
}

// reverse the bits in a char
unsigned long int bitflip(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return (unsigned long int)b;
}

void PeltierOff(void) {             //Shuts down Cooling and Heating PWM
    CoolingPWM=0;                   //Cooling PWM Value set to 0
    HeatingPWM=0;                   //Heating PWM Value set to 0
    HeatLAT=0;                      //Heating LAT output pin set to 0
    CoolLAT=0;                      //Cooling LAT Output pin set to 0
    HeatUp_EN=0;                    //Heating PWM Enable set to 0
    CoolDown_EN=0;                  //Cooling PWM Enable set to 0
    FAN_SetLow();
}

void HeatUp(unsigned int HowMuch) {         //Set Heating PWM
    PeltierOff();
    FAN_SetHigh();
    HeatingPWM=HowMuch;                 //PWM Value
    HeatUp_EN=1;                        //Output Enable
}

void CoolDown(unsigned int HowMuch){        //Set Cooling PWM
    PeltierOff();
    FAN_SetHigh();
    CoolingPWM=HowMuch;                 //PWM Value
    CoolDown_EN=1;                      //Output Enable
};