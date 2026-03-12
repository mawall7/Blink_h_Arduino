#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitops.h"
#include "millis.h"
#include "adc.h"
#include "uart.h"

#define HEARTBEAT_MS 250
#define LED_WHITE PB2 
#define LED_BLUE PB3 
#define LED_GREEN PB4 
#define LED_RED PB5 

#define LEDTOGGLE_BUTTON PB1
#define LEDTOGGLE_RESETBUTTON PD5
//#define LED_INTERRUPT PB0
//#define LED_DIMMER PD6
//#define BUTTON_LED PD2
//#define BUTTON_INTERRUPT PD4

#define LED_RGB_RED PB0
#define LED_RGB_GREEN PD7
#define LED_RGB_BLUE PD6

#define ROTOR_CLK PD2
#define ROTOR_DT PD3
#define ROTOR_SW PD4

typedef enum 
{
    ROTOR_ST_OFF,
    ROTOR_ST_RED,
    ROTOR_ST_GREEN,
    ROTOR_ST_BLUE,
    ROTOR_ST_ALL
} ROTOR_STATE;

typedef enum 
{
    ROTATION_LEFT,
    ROTATION_RIGHT
} ROTOR_DIRECTION;


static volatile uint8_t  last_pd4_state = 1;    // för enkel flankdetektering (pull-up => 1 = ej tryckt)
static volatile uint16_t led8_pulse_ms = 0;     // hur länge PIN 8 ska vara tänd

static volatile uint8_t last_pd7_state = 1;
static volatile uint8_t rotor_enable = 1;

static volatile millis_t last_button_ms = 0;
static volatile millis_t last_button_pb1_ms = 0;

static volatile millis_t last_resetbutton_ms = 0;
static volatile millis_t last_resetbutton_pb5_ms = 0;


//static volatile uint8_t last_button_ledselect = 0;

// #####################################
// --- Pin Change Interrupt för D4 (PD4 = PCINT20) ---
// static void pcint_init_pd4(void)
// {
//     // Aktivera PCINT för port D-gruppen (PCIE2)
//     PCICR |= (1 << PCIE2);

//     // Aktivera mask för PCINT20 (PD4)
//     PCMSK2 |= (1 << PCINT20);

//     // Initiera last state från aktuell pin (pull-up => normalt 1)
//     //last_pd4_state = (PIND & (1 << BUTTON_INTERRUPT)) ? 1 : 0;
// }

static void pcint_init_pd4(void)
{
    // Aktivera PCINT för port D-gruppen (PCIE2)
    PCICR |= (1 << PCIE2);

    // Aktivera mask för PCINT23 (PD7)
    //aktivera mask för PCINT20 (PD4)
    PCMSK2 |= (1 << PCINT20);

    // Initiera last state från aktuell pin (pull-up => normalt 1)
    last_pd4_state = (PIND & (1 << ROTOR_SW)) ? 1 : 0;
}

// static void pcint_init_pb1(void){

//     PCICR |= (1 << PCIE0);    // Port B
//     PCMSK0 |= (1 << PCINT1);  // PB1
// }

// ISR(PCINT0_vect)
// {
//     static millis_t last_press = 0;
//     millis_t now = millis_get();

//     if ((now - last_press) > 50) { // debounce
//         if (!(PINB & (1 << LEDTOGGLE_BUTTON))) { // fallande flank
//             last_button_ledselect ^= 1;
//             uart_print("LED toggled via interrupt");
//         }
//         last_press = now;
//     }
// }
//interruptet startar bara en timer när knappen trycks ner ,men LEDen konditions för på och av kollas i main mha timern medför kort puls
//eller då rotationsencodern vrids = räcknas som ett knapptryck!
ISR(PCINT2_vect)
{
    // Läs nuvarande nivå på PD4 dvs. läs alla bitar för PORTD
    uint8_t now = PIND;
    
        
    //kollar om rotenc knappen 1 eller 0 
    if (last_pd4_state == 1 && ((now & (1 << ROTOR_SW)) ? 1 : 0) == 0) 
    {
        millis_t now_ms = millis_get();
        if ((millis_t)(now_ms - last_button_ms) > 50)
        {
            rotor_enable = (rotor_enable ? 0: 1);
            last_button_ms = now_ms;
        }
    }

    last_pd4_state = ((now & (1 << ROTOR_SW)) ? 1 : 0);
}

void pwm_init_pd6(void)
{
    // Fast PWM mode (WGM00=1, WGM01=1)
    TCCR0A |= (1 << WGM00) | (1 << WGM01);

    // Non-inverting mode på OC0A (PD6)
    TCCR0A |= (1 << COM0A1);

    // Prescaler = 64
    TCCR0B |= (1 << CS01) | (1 << CS00);

    // Start duty cycle (0%)
    OCR0A = 0;
}

// #####################################
// #####################################
void rotor_update(ROTOR_STATE rt_st)
{
    if (rotor_enable == 1) //0
    {
        return;
    }
    CLR_BIT(PORTB, LED_RGB_RED);
    CLR_BIT(PORTD, LED_RGB_GREEN);
    CLR_BIT(PORTD, LED_RGB_BLUE);

    switch (rt_st)
    {
    case ROTOR_ST_OFF:
        break;

    case ROTOR_ST_RED:
        uart_print("redupdateled");
        SET_BIT(PORTB, LED_RGB_RED);
        break;

    case ROTOR_ST_GREEN:
        uart_print("greenupdateled");
        SET_BIT(PORTD, LED_RGB_GREEN);
        break;

    case ROTOR_ST_BLUE:
        SET_BIT(PORTD, LED_RGB_BLUE);
        break;

    case ROTOR_ST_ALL:
        SET_BIT(PORTB, LED_RGB_RED);
        SET_BIT(PORTD, LED_RGB_GREEN);
        SET_BIT(PORTD, LED_RGB_BLUE);
        break;
        
    default:
        break;
    }

}

ROTOR_STATE rotor_event(ROTOR_STATE rt_st, ROTOR_DIRECTION rt_dir)
{
    ROTOR_STATE new_rotor_state;

    switch (rt_st)
    {
    case ROTOR_ST_OFF:
        if(rt_dir == ROTATION_LEFT)
        {
            new_rotor_state = ROTOR_ST_ALL;
        }
        else
        {
            new_rotor_state = ROTOR_ST_RED;
        }
        break;

    case ROTOR_ST_RED:
        if(rt_dir == ROTATION_LEFT)
        {
            new_rotor_state = ROTOR_ST_OFF;
        }
        else
        {
            new_rotor_state = ROTOR_ST_GREEN;
        }
        break;

    case ROTOR_ST_GREEN:
        if(rt_dir == ROTATION_LEFT)
        {
            new_rotor_state = ROTOR_ST_RED;
        }
        else
        {
            new_rotor_state = ROTOR_ST_BLUE;
        }
        break;

    case ROTOR_ST_BLUE:
        if(rt_dir == ROTATION_LEFT)
        {
            new_rotor_state = ROTOR_ST_GREEN;
        }
        else
        {
            new_rotor_state = ROTOR_ST_ALL;
        }
        break;

    case ROTOR_ST_ALL:
        if(rt_dir == ROTATION_LEFT)
        {
            new_rotor_state = ROTOR_ST_BLUE;
        }
        else
        {
            new_rotor_state = ROTOR_ST_OFF;
        }
        break;

    default:
            new_rotor_state = rt_st;
        break;

    }
    rotor_update(new_rotor_state);
    return new_rotor_state;
}

// #####################################

int main(void) 
{
    
    //uart
    uart_init(9600);

    // Röd på PB0
    SET_BIT(DDRB, LED_RGB_RED);
    CLR_BIT(PORTB, LED_RGB_RED);

    // Grön på PD7
    SET_BIT(DDRD, LED_RGB_GREEN);
    CLR_BIT(PORTD, LED_RGB_GREEN);

    // Blå på PD6
    SET_BIT(DDRD, LED_RGB_BLUE);
    CLR_BIT(PORTD, LED_RGB_BLUE);
    ////

    //Knapp LED toogle 
    CLR_BIT(DDRB, LEDTOGGLE_BUTTON);  // sätt pin som input
    SET_BIT(PORTB, LEDTOGGLE_BUTTON);  // aktivera intern pull-up

    CLR_BIT(DDRD, LEDTOGGLE_RESETBUTTON);  // sätt pin som input
    SET_BIT(PORTD, LEDTOGGLE_RESETBUTTON);
    
    //Pin 13 (PB5) Output
    SET_BIT(DDRB, LED_RED);
    CLR_BIT(PORTB, LED_RED);
    
    // Pin 12 (PB4) Output
    SET_BIT(DDRB, LED_GREEN);
    CLR_BIT(PORTB, LED_GREEN);
    
    
    // Pin 10 (PB2) Output
    SET_BIT(DDRB, LED_WHITE);
    CLR_BIT(PORTB, LED_WHITE);
    
    // Pin 11 (PB3) Output
    SET_BIT(DDRB, LED_BLUE);
    CLR_BIT(PORTB, LED_BLUE);
    
    CLR_BIT(DDRD, ROTOR_SW);
    SET_BIT(PORTD, ROTOR_SW);

    // D5 (PD5) input
    CLR_BIT(DDRD, ROTOR_DT);
    
    // D3 (PD3) input
    CLR_BIT(DDRD, ROTOR_CLK);
  
    pcint_init_pd4();
   
    millis_init();
    sei();
   
    adc_init(1, 128);
    
    rotor_enable = 0;
    //millis
    millis_t start = millis_get();
    millis_t last_ms = start;


    // Debounce
    millis_t debounce_time = 50;
    //Green button
    millis_t prev_debounce = 0;
    uint8_t prev_button_state = 1; 
    uint8_t true_button_state = 0;
    static uint8_t pb1_pressed_flag = 0;
    //Red button
    millis_t prev_reset_debounce = 0;
    uint8_t prev_resetbutton_state = 1; 
    uint8_t true_resetbutton_state = 0;
    static uint8_t pb5_pressed_flag = 0;
    
    //Rotor
    uint8_t last_rotor_clk_state = 1;
    ROTOR_STATE rotor_state = ROTOR_ST_OFF;
    uint16_t timed_millis = 250;
    uint8_t buttonLEDOn = 0;
   
    uint8_t blink_red = 1;
    uint8_t blink_green = 1;
    uint8_t blink_blue = 1;
    uint8_t blink_white = 1;

    while (1)
    {   uart_print("rotor enable? ");
        uart_print_uint16(rotor_enable);
        millis_t now = millis_get();

    
        if(rotor_enable == 1){ //SW  
            //grön knapp
            uint8_t current_button_state = READ_BIT(PINB, LEDTOGGLE_BUTTON);
            
            
            if (current_button_state != prev_button_state) {
                prev_debounce = now;
            }
            
            /* om signalen varit stabil länge nog */
            if ((now - prev_debounce) > 50) {

                if (current_button_state != true_button_state) {

                    true_button_state = current_button_state;
                    
                    /* knapptryck (pull-up → 0 när tryckt) */
                    if (true_button_state == 0) {
                        
                        uart_print("button toggled once");
                        uart_print_uint16(rotor_state);
                          switch (rotor_state)
                            {
                                case ROTOR_ST_RED:{
                                    uart_print("red toggled?");
                                    blink_red = 0;
                                    TOG_BIT(PORTB, LED_RED); 
                                    break;
                                }
                                case ROTOR_ST_GREEN:{
                                    blink_green = 0;
                                    TOG_BIT(PORTB, LED_GREEN);
                                    break;
                                }
                                case ROTOR_ST_BLUE:{
                                    blink_blue = 0;
                                    TOG_BIT(PORTB, LED_BLUE);
                                    break;
                                }
                                case ROTOR_ST_ALL:{
                                    blink_white = 0;
                                    TOG_BIT(PORTB, LED_WHITE);
                                    break;
                                }
                                default:
                                break;
                            }
                    }
                    
                }
            }
            
            prev_button_state = current_button_state;
            
        }
        
        if(rotor_enable == 1){ //SW 
        //röd knapp
            uint8_t current_resetbutton_state = READ_BIT(PIND, LEDTOGGLE_RESETBUTTON);
            
            
            
            if (current_resetbutton_state != prev_resetbutton_state) {
                prev_reset_debounce = now;
                
            }
            
            // om signalen varit stabil länge nog 
            if ((now - prev_reset_debounce) > 50) {

                if (current_resetbutton_state != true_resetbutton_state) {

                    true_resetbutton_state = current_resetbutton_state;
                    
                    // knapptryck (pull-up → 0 när tryckt) 
                    if (true_resetbutton_state == 0) {
                        uart_print("reset made");
                    
                        //synka blinkningarna igen
                        start = millis_get();
                        if(blink_red == 0 && rotor_state == ROTOR_ST_RED){
                            
                            blink_red = 1;
                        }
                        if(blink_green == 0 && rotor_state == ROTOR_ST_GREEN){
                            
                            blink_green = 1;    
                        }
                        if(blink_blue == 0 && rotor_state == ROTOR_ST_BLUE){
                            
                             blink_blue = 1;
                        }
                        if(blink_white == 0 && rotor_state == ROTOR_ST_ALL){
                            
                            blink_white = 1;
                        }
                    
                    }
                    
                }
             }
            
            prev_resetbutton_state = current_resetbutton_state;
        }
            // //röd knapp slut kod
                        
            //                 //   // if(prev_button_state!= true_button_state){
            //                 // switch (rotor_state)
            //                 // {
            //                 //     case ROTOR_ST_RED:
            //                 //     TOG_BIT(PINB, LED_RED);
            //                 //     break;
            //                 //     case ROTOR_ST_GREEN:
            //                 //     TOG_BIT(PIND, LED_RED);
            //                 //     break;
            //                 //     case ROTOR_ST_BLUE:
            //                 //     TOG_BIT(PIND, LED_RED);
            //                 //     break;
            //                 //     case ROTOR_ST_ALL:
            //                 //     TOG_BIT(PIND, LED_WHITE);
            //                 //     break;
                                
            //                 //     default:
            //                 //     break;
            //                 // }
                
      //rotor enable blink LEDS
            //uart_print("rotor enable");
            //uart_print_uint16(rotor_enable);
        
        
        // Heartbeat
        if ((millis_t)(now - start) >= timed_millis) 
        {
            start = now;
            //blink LEDS
            //if(rotor_enable == 0 || (rotor_enable == 1 && rotor_state != ROTOR_ST_RED))
                if(blink_red){

                    TOG_BIT(PORTB, LED_RED);
                }
                
                if(blink_green){

                    TOG_BIT(PORTB, LED_GREEN);
                }
                if(blink_blue){
                    
                    TOG_BIT(PORTB, LED_BLUE);
                }
                if(blink_white){
                    
                    TOG_BIT(PORTB, LED_WHITE);
                }
               

                //rotor enable blink RBG LEDS
                if(rotor_enable == 1){
                    switch(rotor_state){
                        case ROTOR_ST_RED:
                            TOG_BIT(PORTB, LED_RGB_RED);
                        break;
                        case ROTOR_ST_GREEN:
                            TOG_BIT(PORTD, LED_RGB_GREEN);
                        break;
                        case ROTOR_ST_BLUE:
                            TOG_BIT(PORTD, LED_RGB_BLUE);
                        break;
                        case ROTOR_ST_ALL:
                            TOG_BIT(PORTB, LED_RGB_RED);
                            TOG_BIT(PORTD, LED_RGB_GREEN);
                            TOG_BIT(PORTD, LED_RGB_BLUE);
                        break;
                            };
                        }
                        
                
        }
            
        
       
        uint8_t rotor_clk_state = READ_BIT(PIND, ROTOR_CLK);
        //uint8_t rotor_dt_state  = READ_BIT(PIND, ROTOR_DT);

        if (rotor_clk_state != last_rotor_clk_state && rotor_enable == 0 ) //1
        {   
            
            last_rotor_clk_state = rotor_clk_state;
            uint8_t rotor_dt_state  = READ_BIT(PIND, ROTOR_DT);

            if (rotor_clk_state==0 && rotor_dt_state!=0)
            {   
                uart_print("left_rotation");
                rotor_state = rotor_event(rotor_state, ROTATION_LEFT);
                uart_print_uint16(rotor_state);
            }
            if (rotor_clk_state==0 && rotor_dt_state==0)
            {
                rotor_state = rotor_event(rotor_state, ROTATION_RIGHT);
                uart_print("right_rotation");
                uart_print_uint16(rotor_state);
            }
            

            uart_print_uint16(rotor_clk_state);
            uart_print_uint16(rotor_dt_state);
        }

 
        //Uppdaterar pwm-led

            uint16_t adc_value = adc_read(0); //0-1023
            // konvertering till 0-255 * 10 -> 0 - 2500 ms
            timed_millis = (uint16_t)(adc_value >> 2) * 10 + HEARTBEAT_MS; 
 
    }
    
}
