#ifndef UART_H
#define UART_H

#include <avr/io.h>

// Initiera UART
void uart_init(unsigned long baud);

// Skicka ett tecken
void uart_transmit(char data);

// Ta emot ett tecken
char uart_receive(void);

// Skicka en sträng
void uart_print(const char *str);

// skickar uint16 som sträng
void uart_print_uint16(uint16_t value);

#endif