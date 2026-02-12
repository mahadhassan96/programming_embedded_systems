#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

/*Btn variable*/
#define BTN_NODE DT_ALIAS(btn0)
#define LED2_NODE   DT_ALIAS(led2)
#define LED3_NODE   DT_ALIAS(led3) 
struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios); 
const struct gpio_dt_spec btn0 = GPIO_DT_SPEC_GET(BTN_NODE, gpios);
static struct gpio_callback btn_cb_data;
static int btn_state = 0;
float time_diff = 0.0;
uint32_t remaining_time = 0;

struct k_timer my_timer;

static void time_over(struct k_timer *dummy){
    gpio_pin_toggle_dt(&led3);
    printk("Time expired. Try again\n");
    btn_state = 0;
}

void button_isr() {
    if(btn_state == 0){
        btn_state = 1;
        k_timer_start(&my_timer, K_SECONDS(6), K_NO_WAIT);
    }
    else{
        remaining_time = k_timer_remaining_get(&my_timer);
        k_timer_stop(&my_timer);
        btn_state = 0;
    }
}

void timer_stop(){
    uint32_t running_time = 6000 - remaining_time;
    printk("Time was: %d ms\n", running_time);
    if(running_time - 3000 <= 200){
        printk("Congratulations, You won!\n");
    }

    gpio_pin_toggle_dt(&led2);
    btn_state = 0;
}

void init_button(){
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set_dt(&led2, 0);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set_dt(&led3, 0);
    gpio_pin_configure_dt(&btn0, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&btn0, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&btn_cb_data, button_isr, BIT(btn0.pin));
    gpio_add_callback_dt(&btn0, &btn_cb_data);
}

K_TIMER_DEFINE(my_timer, time_over, timer_stop);

int main(void)
{
    /*while(1){
        printf("Hello World!\n");
        k_msleep(500); 
    }*/

    printk("\nInitializing button\n");

    init_button();

	return 0;
}
