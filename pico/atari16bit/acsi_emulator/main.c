/**
 * ACSI-emulator tests communication with acsiFuji. Sends and recieves acsi commands and data over UART.
*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "../pinmap.h"
#include "hardware/uart.h"

/// \tag::hello_uart[]

#define UART_ID uart0
#define BAUD_RATE 2000000

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define VERSION 0.01
#define LED_PIN 25

static char buf[32*1024]; // Room for 64 sectors in buffer

int main() {
    stdio_init_all();
    sleep_ms(8000); //Wait a bit
    printf("Pico ACSI Emulator VERSION\n");
    printf("------------------\n");

    printf("Init UART to esp32...\n\n");

    uart_init(UART_ID, BAUD_RATE);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);   
    
    printf("Init LED..\n\n");   
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN,GPIO_OUT);
    gpio_put(LED_PIN,0);
 
 /* Send Test unit ready (WIP)*/
    while(1) {
    printf("Send test unit ready.\n");
    uart_putc_raw(UART_ID,'C');
    uart_putc_raw(UART_ID,0);
    /* Wait for answer */
    
    }
    /**
     * @brief main loop test acsiFujinet over serial.
    */
    while(1) {
     
    }
    return 0;
}