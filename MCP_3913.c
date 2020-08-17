/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */
#include "MCP_3913.h"
#include "include.h"
#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/spi2.h"
#include "mcc_generated_files/uart2.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */
int global_data;


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */

/** 
  @Function
    int ExampleLocalFunctionName ( int param1, int param2 ) 

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
char Config_MCP(void) {         //Configura el ADC MCP 
    MCP_CS_SetLow();
    SPI2BUF=0x12;                   //write to reg 0x09
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    //Register 0x09 - Gain (1 register)
    SPI2BUF=0xAA;                   //Channel 1 G=32 - Ch0 G=4
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    //Status 0x0A (2 reg))
    SPI2BUF=0x17;                   
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0xB8;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    //Config 0x0C (2 reg)
    SPI2BUF=0xBF; //0xBE
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    //Offset Ch0 0x0E (3 reg)
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    //Gain Ch0 0x11 (3 reg)
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    //Offset Ch1 0x14 (3 reg)
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    //Gain Ch1 0x17 (3 reg)
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    SPI2BUF=0x00;
    while(SPI2STATbits.SPIBUSY);
    uc_sequence=SPI2BUF;
    MCP_CS_SetHigh();
    
    Delayms(100);
    
    //Reads register 0x09 (Gain) and compare it with written value
    while(SPI2STATbits.RXBUFELM) uc_aux=SPI2BUF;
    MCP_CS_SetLow();
    SPI2BUF=0x13;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    
    SPI2BUF=0xFF;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    MCP_CS_SetHigh();
    
    if(uc_aux!=0xAA) return -1;
    else return 0;
}


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

// *****************************************************************************

/** 
  @Function
    int ExampleInterfaceFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Remarks
    Refer to the example_file.h interface header for function usage details.
 */
void Read_MCP(void) {           //Lectura ADC MCP
        
    while(SPI2STATbits.RXBUFELM) uc_aux=SPI2BUF;
    
    INTERRUPT_GlobalDisable();
    
    MCP_CS_SetLow();
    
    SPI2BUF=0x01;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    
    SPI2BUF=0xFF;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    i_mcp=uc_aux<<8;
    
    SPI2BUF=0xFF;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;    
    i_mcp+=uc_aux;
    i_mcp<<=8;
    
    SPI2BUF=0xFF;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    i_mcp+=uc_aux;
    
    SPI2BUF=0xFF;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    i_mcp2=uc_aux<<8;
    
    SPI2BUF=0xFF;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    i_mcp2+=uc_aux;
    i_mcp2<<=8;
    
    SPI2BUF=0xFF;
    while(SPI2STATbits.SPIBUSY);
    uc_aux=SPI2BUF;
    i_mcp2+=uc_aux;
    
    MCP_CS_SetHigh();
    
    INTERRUPT_GlobalEnable();
    
    ui_mcpindex++;
    ul_mcp+=i_mcp;
    ul_mcp2+=i_mcp2;

    if(ui_mcpindex>1023) {
        f_mcp=ul_mcp2>>10;//500.0;
        f_mcp2=ul_mcp>>10;//500.0;
        ui_mcpindex=0;
        ul_mcp=0;
        ul_mcp2=0;

        f_temp=f_mcp*Convert.K_temp;
        f_temp-=Convert.C_temp;
        
        f_lm334=f_mcp2*Convert.K_LM334;
        
        /*printf("MCP: %.5f\r\n", f_mcp);
        printf("TEMP: %.5fºC\r\n", f_temp);
        printf("MCP_ch1: %.5f\r\n", f_mcp2);
        printf("V_LM334: %.5fmV\r\n", f_lm334);//*/

    }

    
}


/* *****************************************************************************
 End of File
 */
