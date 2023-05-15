/*
 * This sample toggles LED1 and logs over serial console whenever you press button1!
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(button_click_led, LOG_LEVEL_DBG);

#define SLEEP_TIME_MS   1000  // ms

// Create reference variable for devicetree nodes with identifiers "led0", "button0", and "uart0" 
#define LED0_NODE DT_NODELABEL(led0)
#define LED1_NODE DT_NODELABEL(led1)
#define BUTTON0_NODE DT_NODELABEL(button0)
#define UART_NODE DT_NODELABEL(uart0)


/*
* A build error on this line means your board is unsupported.
* See the sample documentation for information on how to fix this.
*/
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);
static const struct device *uart = DEVICE_DT_GET(UART_NODE);

// Create button ISR callbacks
static struct gpio_callback button_cb_data;

// Create UART receive and transmit buffers
static uint8_t rx_buf[10] = {0};
static uint8_t tx_buf[] = {"Press 1-2 on your keyboard in a Serial Emulator to toggle LEDS.\n\r"};

void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	/*
	 * This ISR toggles both LED1 and LED2 when Button1 is pressed
	 */
	LOG_WRN("button pressed!");
	LOG_DBG("%d + %d = %d!!!", 1, 2, 3);
    gpio_pin_toggle_dt(&led);
	gpio_pin_toggle_dt(&led1);
}

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {
		case UART_RX_RDY:
			if((evt->data.rx.len) == 1){

			if (evt->data.rx.buf[evt->data.rx.offset] == '1')
				gpio_pin_toggle_dt(&led);
			else if (evt->data.rx.buf[evt->data.rx.offset] == '2')
				gpio_pin_toggle_dt(&led1);	
			}
			break;
		case UART_RX_DISABLED:
			uart_rx_enable(dev, rx_buf, sizeof rx_buf, 100);
			break;
		default:
			break;
	}
}

void main(void)
{
	int ret;

	// All LED pins are on the same port, so only need to check one of the LED's ports
	if (!device_is_ready(led.port)) {
		printk("LED not ready\r\n");
		return;
	}

	if (!device_is_ready(button.port)) {
		printk("Button not ready\r\n");
		return;
	}

	if (!device_is_ready(uart)){
		printk("UART device not ready\r\n");
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return;
	}

	ret = uart_callback_set(uart, uart_cb, NULL);
	if (ret) {
		return;
	}

	ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
	if (ret) {
		return;
	}

	gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	ret = uart_rx_enable(uart, rx_buf, sizeof rx_buf, 100);
	if (ret) {
		return;
	}

	while (1) {
		// Let interrupt do its thing
		// k_msleep(SLEEP_TIME_MS);
		k_yield();
	}
}
