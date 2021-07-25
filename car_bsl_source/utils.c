#include "main.h"
#include "utils.h"


/*
 * @returns     Send value from UART debug port
 * @details     Send a target char to UART debug port .
 */
void SendChar_ToUART(int ch)
{
    while (UART_T->FSR & UART_FSR_TX_FULL_Msk);

    UART_T->THR = ch;
    if ( ch == '\n')
    {
        while (UART_T->FSR & UART_FSR_TX_FULL_Msk);
        UART_T->THR = '\r';
    }
}



void SendString(char *str)
{
    while (*str != '\0')
    {
        SendChar_ToUART(*str++);
    }
}




//*****************************************************************************
//
//! Convert a byte of data to ascii and send to UART Tx.
//!
//! \return None.
//
//*****************************************************************************
void UART_Print_Data(uint8_t u8Data)
{
    uint8_t u8Byte=0, i=0;

    for(i=2; i>0; i--)
    {
        u8Byte = (uint8_t)((u8Data >> (i-1)*4) & 0x0F);
        /* if( u8Byte <= 9 )
        {
            u8Byte += 48;
        }
        else
        {
            // u8Byte = (u8Byte - 10) + 65;
            u8Byte += 55;
        }*/
        SendChar_ToUART(HEX2ASCII(u8Byte));
    }
}



uint8_t SetBootFromAPROM(void)
{
    uint32_t au32Config[2];

    SYS_UnlockReg();
    
    FMC_ReadConfig(au32Config, 2);
    if ( (au32Config[0] & 0x80) == 0x00 )
    {
        FMC_EnableConfigUpdate();
        au32Config[0] |= 0x80;
        FMC_Erase(FMC_CONFIG_BASE);
        if ( FMC_WriteConfig(au32Config, 2) < 0 )
            return 0;

        // Perform chip reset to make new User Config take effect
        SYS_ResetChip();
    }
    
    return 1;
}




