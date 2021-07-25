#ifndef __UTILS_H__
#define __UTILS_H__

extern void UART_Print_Data(uint8_t u8Data);
extern void SendChar_ToUART(int ch);
extern void SendString(char *str);
extern uint8_t SetBootFromAPROM(void);




#endif
