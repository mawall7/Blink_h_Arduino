#include "uart.h"
#include <stdlib.h>

#define F_CPU 16000000UL

void uart_init(unsigned long baud)
{
    unsigned int ubrr = (F_CPU / 16 / baud) - 1;

    // Sätt baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Aktivera sändning och mottagning
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Sätt ramformat: 8 databitar, 1 stoppbit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(char data)
{
    // Vänta tills sändbufferten är tom
    while (!(UCSR0A & (1 << UDRE0)));

    // Lägg data i bufferten
    UDR0 = data;
}

char uart_receive(void)
{
    // Vänta tills data har mottagits
    while (!(UCSR0A & (1 << RXC0)));

    return UDR0;
}

void uart_print(const char *str)
{
    while (*str)
    {
        uart_transmit(*str++);
    }
}

void uart_print_uint16(uint16_t value)
{
    char buffer[6]; // max 65535 + null
    itoa(value, buffer, 10);
    uart_print(buffer);
}