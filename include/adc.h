#pragma once
#include <stdint.h>
#include <stdbool.h>

/*
 * Initiera ADC.
 *  - ref: referensspänning
 *      0 = AREF extern
 *      1 = AVcc (standard på Uno)
 *      3 = Intern 1.1V
 *
 *  - prescaler: ADC-klockdelare (2,4,8,16,32,64,128)
 *    För 16 MHz: 128 ger ~125 kHz ADC-klocka (rekommenderat).
 */
void adc_init(uint8_t ref, uint8_t prescaler);

/*
 * Läs en kanal (0–7) blockerat.
 * Returnerar 10-bitarsvärde (0–1023).
 */
uint16_t adc_read(uint8_t channel);

/*
 * Starta en konvertering på kanal (0–7).
 */
void adc_start(uint8_t channel);

/*
 * Kolla om konvertering klar.
 */
bool adc_ready(void);

/*
 * Hämta resultat (utan att starta ny).
 */
uint16_t adc_get(void);