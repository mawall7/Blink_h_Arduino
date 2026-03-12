#include "adc.h"
#include <avr/io.h>

static uint8_t prescaler_bits(uint8_t prescaler) {
    switch (prescaler) {
        case 2:   return (1 << ADPS0);
        case 4:   return (1 << ADPS1);
        case 8:   return (1 << ADPS1) | (1 << ADPS0);
        case 16:  return (1 << ADPS2);
        case 32:  return (1 << ADPS2) | (1 << ADPS0);
        case 64:  return (1 << ADPS2) | (1 << ADPS1);
        case 128: return (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
        default:  return (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // fallback 128
    }
}

void adc_init(uint8_t ref, uint8_t prescaler) {
    // Referens
    // REFS1:0 i ADMUX
    ADMUX = (ref << REFS0);

    // Högerjusterat resultat (standard)
    ADMUX &= ~(1 << ADLAR);

    // Prescaler
    ADCSRA = prescaler_bits(prescaler);

    // Enable ADC
    ADCSRA |= (1 << ADEN);
}

void adc_start(uint8_t channel) {
    channel &= 0x07; // 0–7

    // Rensa gamla kanalbitar
    ADMUX = (ADMUX & 0xF8) | channel;

    // Start conversion
    ADCSRA |= (1 << ADSC);
}

bool adc_ready(void) {
    // ADSC = 0 när klar
    return !(ADCSRA & (1 << ADSC));
}

uint16_t adc_get(void) {
    // Läs ADCL först, sedan ADCH
    uint8_t low  = ADCL;
    uint8_t high = ADCH;
    return (high << 8) | low;
}

uint16_t adc_read(uint8_t channel) {
    adc_start(channel);
    while (!adc_ready()) { }
    return adc_get();
}