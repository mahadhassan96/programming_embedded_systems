#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/pwm.h"
/* !!! PART 2 & 3 !!! */

/* !!! MIGHT WANT TO CHANGE THIS !!! */
#define BUTTON_DEBOUNCE_DELAY   100

const static uint led_0 = 0;
static uint led = 0; 
static uint16_t pwm_led = 0;
static bool incr = false;
const static uint b1 = 20; 
const static uint b2 = 21; 
const static uint b3 = 22;
uint pwm_slice;
; 
/* Last button ISR time */
unsigned long button_time = 0;

/* Event queue */
queue_t evt_queue; 

/* Function pointer primitive */ 
typedef void (*state_func_t)( void );

typedef struct _state_t
{
    uint8_t id;
    state_func_t Enter;
    state_func_t Do;
    state_func_t Exit;
    uint32_t delay_ms;
} state_t;

typedef enum _event_t 
{
    b1_evt,
    b2_evt,
    b3_evt,
    no_evt
} event_t;

/* !!! PART 2 & 3 !!! */

void button_isr(uint gpio, uint32_t events) 
{
    if ((to_ms_since_boot(get_absolute_time())-button_time) > BUTTON_DEBOUNCE_DELAY) 
    {
        button_time = to_ms_since_boot(get_absolute_time());
        
        event_t evt;
        switch(gpio)
        {
            case b1: 
                evt = b1_evt; 
                queue_try_add(&evt_queue, &evt); 
            break; 

            case b2: 
                evt = b2_evt; 
                queue_try_add(&evt_queue, &evt); 
            break;

            case b3: 
                evt = b3_evt; 
                queue_try_add(&evt_queue, &evt); 
            break;
        }
    }
}

void private_init() 
{
    /* !!! PART 2 !!! */
    /* Event queue setup */ 
    queue_init(&evt_queue, sizeof(event_t), 10); 

    /* !!! PART 2 !!! */
    /* Button setup */
    /* Setup buttons */
    gpio_init(b1);
    gpio_init(b2);
    gpio_init(b3);
    /* Enable interrupt line shared by all GPIO pins */
    irq_set_enabled(IO_IRQ_BANK0, true);
    /* Set callback button_isr, triggered on any IRQs on that line*/
    gpio_set_irq_callback(button_isr);
    /* Trigger IRQs on falling edges*/
    gpio_set_irq_enabled(b1, GPIO_IRQ_EDGE_FALL, true); 
    gpio_set_irq_enabled(b2, GPIO_IRQ_EDGE_FALL, true); 
    gpio_set_irq_enabled(b3, GPIO_IRQ_EDGE_FALL, true); 

    /* !!! PART 1 !!! */
    /* LED setup */
    for(int i = 0; i < 4; i++){
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
    }

    pwm_slice = pwm_gpio_to_slice_num(led_0);
}

/* The next three methods are for convenience, you might want to use them. */
event_t get_event(void)
{
    /* !!!! PART 2 !!!! */
    event_t evt = no_evt; 
    if (queue_try_remove(&evt_queue, &evt)){ 
        return evt; 
    }
    return no_evt; 
}

void leds_off () 
{
    /* !!! PART 1 !!! */
    for(int i = 0; i < 4; i++){
        gpio_put(i, 0);
    }
}

void leds_on () 
{
    /* !!! PART 2 !!! */
    for(int i = 0; i < 4; i++){
        gpio_put(i, 1);
    }
}

void enter_state_0(void){
    led = led_0;
    leds_off();
}

void exit_state_0(void){
    if(led > 0){    gpio_put(led-1, 0); }
    else{   gpio_put(led, 0);   }
}

void enter_state_1(void){
    leds_off();
}

void exit_state_1(void){
    leds_off();
}

void enter_state_2(void){
    led = 3;
    leds_off();
}

void exit_state_2(void){
    if(led < 3){    gpio_put(led+1, 0); }
    else{   gpio_put(led, 3);   }
}

void enter_state_3(void){
    leds_off();
    gpio_set_function(led_0, GPIO_FUNC_PWM);
    pwm_set_enabled(pwm_slice, true);
    pwm_set_wrap(pwm_slice, 5000);

    incr = true;
}

void exit_state_3(void){
    pwm_set_chan_level(pwm_slice, PWM_CHAN_A, 0);
    pwm_set_enabled(pwm_slice, false);
    gpio_set_function(0, GPIO_FUNC_SIO);
}

void do_state_0(void)
{
    /* !!! PART 1 !!! */
    if(led > 0){    gpio_put(led-1, 0); }
    else    {   gpio_put(3, 0); }
    gpio_put(led, 1);
    led = (led + 1) % 4;
}

void do_state_1(void)
{
    /* !!! PART 1 !!! */
    bool leds_check = gpio_get(0) | gpio_get(1) | gpio_get(2) | gpio_get(3);
    if(leds_check){
        leds_off();
    }
    else{
        leds_on();
    }
}

void do_state_2(void)
{
    if(led < 3){    gpio_put(led+1, 0); }
    else    {   gpio_put(0, 0); }

    gpio_put(led, 1);
    led = ((led-1) + 4) % 4;
}

void do_state_3(void)
{
    pwm_set_chan_level(pwm_slice, PWM_CHAN_A, pwm_led);
    if(incr){
        pwm_led=pwm_led+50;
        if(pwm_led >= 5000)  {   incr = false;   }
    }
    else{
        pwm_led=pwm_led-50;
        if(pwm_led <= 0) {   incr = true;    }
    }
}

/* !!! PART 1 !!! */
const state_t state0 = {
    0, 
    enter_state_0,
    do_state_0,
    exit_state_0, 
    300
};

const state_t state1 = {
    1, 
    enter_state_1,
    do_state_1,
    exit_state_1, 
    300
};

const state_t state2 = {
    2, 
    enter_state_2,
    do_state_2,
    exit_state_2, 
    200
};

const state_t state3 = {
    3, 
    enter_state_3,
    do_state_3,
    exit_state_3, 
    10
};

/* !!! PART 2 !!! */
const state_t state_table[4][4] =   {/*{b1_event, b2_event, b3_event, no_event}*/
                                    {state2, state1, state3, state0},
                                    {state0, state2, state3, state1},
                                    {state1, state0, state3, state2},
                                    {state0, state0, state0, state3}
                                };

/* !!! ALL PARTS !!! */
int main() 
{
    private_init(); 

    state_t current_state = state0;
    event_t evt = no_evt;

    for(;;) 
    {
        current_state.Enter();
        evt = get_event();

        while(1)
        {
            current_state.Do();
            sleep_ms(current_state.delay_ms);
            evt = get_event();
            if(current_state.id != state_table[current_state.id][evt].id){
                break;
            }
        }
        current_state.Exit();
        current_state = state_table[current_state.id][evt];
    }
}
