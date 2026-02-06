#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 500
#define BTN_NODE DT_ALIAS(btn0)

/* Creates semaphore with count set to 0 and limit to 1 */
K_SEM_DEFINE(sem, 0, 1)
K_MUTEX_DEFINE(my_mutex);

/*Btn & LED variables*/
const struct gpio_dt_spec btn0 = GPIO_DT_SPEC_GET(BTN_NODE, gpios);
static struct gpio_callback btn_cb_data;

#define LED2_NODE   DT_ALIAS(led2) 
#define LED3_NODE   DT_ALIAS(led3) 
#define LED4_NODE   DT_ALIAS(led4) 
#define LED5_NODE   DT_ALIAS(led5) 

struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios); 
struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios); 
struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED4_NODE, gpios); 
struct gpio_dt_spec led5 = GPIO_DT_SPEC_GET(LED5_NODE, gpios);
struct gpio_dt_spec* led[4] = {&led2, &led3, &led4, &led5};
int i = 0;

void led_init()
{
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE); 
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE); 
    gpio_pin_configure_dt(&led4, GPIO_OUTPUT_ACTIVE); 
    gpio_pin_configure_dt(&led5, GPIO_OUTPUT_ACTIVE); 

    gpio_pin_set_dt(&led2, 0);
    gpio_pin_set_dt(&led3, 0);
    gpio_pin_set_dt(&led4, 0);
    gpio_pin_set_dt(&led5, 0);
}

void blinky_task()
{   led_init();
    for(;;)
    {
        gpio_pin_toggle_dt(led[i]); 
        k_msleep(250); 
    }
}

void button_isr() {
    k_sem_give(&sem);
}

void btn_init()
{
    gpio_pin_configure_dt(&btn0, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&btn0, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&btn_cb_data, button_isr, BIT(btn0.pin));
    gpio_add_callback_dt(&btn0, &btn_cb_data);
}

void button_task()
{
    btn_init();
    for (;;) {
        k_sem_take(&sem, K_FOREVER);
        k_mutex_lock(&my_mutex, K_FOREVER); 
        gpio_pin_set_dt(led[i], 0);
        i = (i + 1) % 4;
        k_mutex_unlock(&my_mutex); 
    }
}

K_THREAD_DEFINE(t1, STACK_SIZE, button_task, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(t2, STACK_SIZE, blinky_task, NULL, NULL, NULL, 5, 0, 0);
