/****************************************************************************
 * @file     main.c
 * @version  V3.00
 * @brief    Demonstrate how to update chip flash data through UART interface
 *           between chip UART and PC UART.
 *           Nuvoton NuMicro ISP Programming Tool is also required in this
 *           sample code to connect with chip UART and assign update file
 *           of Flash.
 * 
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "main.h"


volatile uint8_t g_u8UartIdleCntr = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

/**
  * @brief      This function check selected clock source status
  * @param[in]  u32ClkMask is selected clock source. Including :
  *             - \ref CLK_CLKSTATUS_XTL12M_STB_Msk
  *             - \ref CLK_CLKSTATUS_OSC22M_STB_Msk
  *             - \ref CLK_CLKSTATUS_OSC10K_STB_Msk
  *             - \ref CLK_CLKSTATUS_PLL_STB_Msk
  * @retval     0  clock is not stable
  * @retval     1  clock is stable
  * @details    To wait for clock ready by specified CLKSTATUS bit or timeout (~300ms)
  */
uint32_t CLK_WaitClockReady(uint32_t u32ClkMask)
{
    int32_t i32TimeOutCnt = 1200000;

    while((CLK->CLKSTATUS & u32ClkMask) != u32ClkMask)
    {
        if(i32TimeOutCnt-- <= 0)
            return 0;
    }

    return 1;
}


void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    
    /* Enable Internal RC 22.1184MHz clock */
    // CLK->PWRCON |= (CLK_PWRCON_OSC22M_EN_Msk | CLK_PWRCON_XTL12M_EN_Msk);
    CLK->PWRCON |= CLK_PWRCON_OSC22M_EN_Msk;

    CLK->PLLCON = PLLCON_SETTING;

    /* Waiting for Internal RC clock ready */
    // while (!(CLK->CLKSTATUS & CLK_CLKSTATUS_OSC22M_STB_Msk));
    CLK_WaitClockReady(CLK_CLKSTATUS_PLL_STB_Msk | CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    // CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLK_S_Msk)) | CLK_CLKSEL0_HCLK_S_HIRC;
    // CLK->CLKDIV = (CLK->CLKDIV & (~CLK_CLKDIV_HCLK_N_Msk)) | CLK_CLKDIV_HCLK(1);

    /* Switch HCLK clock source to PLL */
    CLK->CLKSEL0 = CLK_CLKSEL0_HCLK_S_PLL;
    
    /* Set core clock as PLL_CLOCK from PLL */
    // CLK->PLLCON = PLLCON_SETTING;
    // while (!(CLK->CLKSTATUS & CLK_CLKSTATUS_PLL_STB_Msk));
    // CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLK_S_Msk)) | CLK_CLKSEL0_HCLK_S_PLL;
    
    /* Enable IP clock */
    CLK->APBCLK = CLK_APBCLK_UART0_EN_Msk | CLK_APBCLK_WDT_EN_Msk;

    /* Select IP clock source */
    CLK->CLKSEL1 = CLK_CLKSEL1_UART_S_HIRC | CLK_CLKSEL1_WDT_S_HCLK_DIV2048;
    
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    //SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            // PLL
    SystemCoreClock = PLL_CLOCK / 1;        // HCLK
    CyclesPerUs     = PLL_CLOCK / 1000000;  // For SYS_SysTickDelay()
    
    /* Enable UART module clock */
    // CLK->APBCLK |= (CLK_APBCLK_UART0_EN_Msk);
    
    /* Select UART module clock source */
    // CLK->CLKSEL1 = (CLK->CLKSEL1 & (~CLK_CLKSEL1_UART_S_Msk)) | CLK_CLKSEL1_UART_S_HIRC;
    
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    
    /* Set P3 multi-function pins for UART0 RXD and TXD */
    SYS->P3_MFP &= ~(SYS_MFP_P30_Msk | SYS_MFP_P31_Msk);
    SYS->P3_MFP |= (SYS_MFP_P30_RXD0 | SYS_MFP_P31_TXD0);
}





/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/

int main(void)
{
    static uint32_t u32TestCntr = 0;
    // uint8_t u8RxData = 0;
    
    /* Unlock protected registers */
    SYS_UnlockReg();
    
    /* Configure WDT */
    // WDT->WTCR &= ~(WDT_WTCR_WTE_Msk | WDT_WTCR_DBGACK_WDT_Msk);
    // WDT->WTCR |= (WDT_TIMEOUT_2POW18 | WDT_WTCR_WTR_Msk);
    
    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();
    
    /* Init UART to 115200-8n1 */  
    UART_T_Init();
    
    /* Enable FMC ISP */        
    CLK->AHBCLK |= CLK_AHBCLK_ISP_EN_Msk;
    FMC->ISPCON |= FMC_ISPCON_ISPEN_Msk;
    
    /* Get APROM size, data flash size and address */        
    g_apromSize = GetApromSize();
    GetDataFlashInfo(&g_dataFlashAddr, &g_dataFlashSize);
 
    /* Set Systick time-out for 300ms */     
    // SysTick->LOAD = 300000 * CyclesPerUs;
    // SysTick->VAL  = (0x00);
    // SysTick->CTRL = SysTick->CTRL | SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;   /* Use CPU clock */

    GPIO_SetMode(P2, BIT4, GPIO_PMD_OUTPUT);
    GPIO_SetMode(P2, BIT6, GPIO_PMD_OUTPUT);
    P24 = 0;
    P26 = 1;
    
    /* Wait for CMD_CONNECT command until Systick time-out */
    while (1)
    {
        if ( ++u32TestCntr >= 500000 )
        {
            WDT_RESET_COUNTER();
            u32TestCntr = 0;
            P24 ^= 1;
            if ( ++g_u8UartIdleCntr >= 120 )
            {
                g_u8UartIdleCntr = 0;
                SetBootFromAPROM();
                WDT_Open(WDT_TIMEOUT_2POW4, WDT_RESET_DELAY_3CLK, TRUE, FALSE);
                while(1);
            }
            // UART_Print_Data(bufhead);
            // SendChar_ToUART('\n');
        }

        // if ( (UART_T->FSR & UART_FSR_RX_EMPTY_Msk) == 0 )
        // {
            // u8RxData = UART_T->RBR;
            // SendChar_ToUART(u8RxData);
        // }
        
        /* Wait for CMD_CONNECT command */         
        if ( (bufhead >= 4) || (bUartDataReady == TRUE) )
        {
            uint32_t lcmd;
            lcmd = inpw(uart_rcvbuf);

            if ( lcmd == CMD_CONNECT )
            {
                g_u8UartIdleCntr = 0;
                goto _ISP;
            }
            else
            {
                bUartDataReady = FALSE;
                bufhead = 0;
            }
        }
            
        /* Systick time-out, then go to APROM */   
        // if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
            // goto _APROM;
        // }
        // if ( ++u32TestCntr >= 10000 )
        // {
            // u32TestCntr = 0;
            // goto _APROM;
        // }
    }

_ISP:
    
    /* Prase command from master and send response back */   
    while (1)
    {
        if (bUartDataReady == TRUE)
        {
            // WDT->WTCR &= ~(WDT_WTCR_WTE_Msk | WDT_WTCR_DBGACK_WDT_Msk);
            // WDT->WTCR |= (WDT_TIMEOUT_2POW18 | WDT_WTCR_WTR_Msk);
            bUartDataReady = FALSE;
            ParseCmd(uart_rcvbuf, 64);
            PutString();
        }
        
        if ( ++u32TestCntr >= 1000000 )
        {
            WDT_RESET_COUNTER();
            u32TestCntr = 0;
            P24 ^= 1;
            if ( ++g_u8UartIdleCntr >= 10 )
            {
                g_u8UartIdleCntr = 0;
                SetBootFromAPROM();
                WDT_Open(WDT_TIMEOUT_2POW4, WDT_RESET_DELAY_3CLK, TRUE, FALSE);
                while(1);
            }
            // UART_Print_Data(bufhead);
            // SendChar_ToUART('\n');
        }
    }

_APROM:
    
    /* Reset system and boot from APROM */
    SYS->RSTSRC = (SYS_RSTSRC_RSTS_POR_Msk | SYS_RSTSRC_RSTS_RESET_Msk); /* Clear reset status flag */
    FMC->ISPCON &= ~(FMC_ISPCON_BS_Msk|FMC_ISPCON_ISPEN_Msk);
    SCB->AIRCR = (V6M_AIRCR_VECTKEY_DATA | V6M_AIRCR_SYSRESETREQ);

    /* Trap the CPU */
    while (1);
}

/*** (C) COPYRIGHT 2019 Nuvoton Technology Corp. ***/
