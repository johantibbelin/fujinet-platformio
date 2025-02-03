/**
 * @brief A simpler version that uses PIO and bitbanging for ACSI.
 * @details Uses PIO for waiting on first command frame byte. Handling and recieving remaining bytes is handled in C code. 
 * PIO is used for DMA transfers.
 * @author Johan Tibbelin (sjfroos) 
*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

#include "pinmap.h"

/**
 * PIO include files
 */
#include "acsi_cmd.pio.h"
#include "dma_in.pio.h"
#include "dma_out.pio.h"
#include "wait_cmd.pio.h"
#include "send_status.pio.h"

/**
 * Bootsector (for testing)
 */
#include "bootsector.h"

#define LED_PIN 25

/* UART pins connected to ESP32 */
#define ESP32_UART_TX 4
#define ESP32_UART_RX 5

/**
 * ACSI stuff
 */

//ACSI Bus Direction (for use with ACSI_D_DIR pin)
#define ACSI_BUS_INPUT 1
#define ACSI_BUS_OUTPUT 0

/* ACSI Commands*/
#define CMD_TEST_UNIT_READY 0x00
#define CMD_REQUEST_SENSE 0x03
#define CMD_READ 0x08
#define CMD_WRITE 0x0a
#define CMD_SEEK 0x0b
#define CMD_INQUERY 0x12
#define CMD_MODE_SENSE 0x1a

/* ICD extension */
#define CMD_ICD_EXT 0x1f

/* Atari Page Interface */

#define CMD_PRINT 0x0a //Same as Write (Printer need own ID)
#define CMD_MODE_SELECT 0x15
#define CMD_STOP_PRINT 0x1b

// ACSI IDs- ids to repond to

#define ACSI_ID 2
#define PRINTER_ID 5 /* Standard id for printer */

// Bootsector enabled
bool dummy_bootsector = true;

// ACSI Error Codes

#define ERROR_OK 0x00
#define ERROR_INVALID_COMMAND 0x20
#define ERROR_INVALID_ADDRESS 0x21
#define ERROR_VOLUME_OVERFLOW 0x23
#define ERROR_INVALID_ARGUMENT 0x24
#define ERROR_INVALID_DEVICE_NUMBER 0x25

// PIO instances global

PIO pio = pio0;
PIO pio_dma = pio1;
PIO pio_snd_status =pio1;
PIO pio_snd_irq = pio0;

uint sm_snd_status = 1;
uint sm_dma = 0;
uint sm_cmd=0;

/**
 * Serial is handled by core1
 */
void core1_entry() {
    //Core one code goes here
    while (1) {


    }
}

void dpins_high() { 
    gpio_put(ACSI_D_DIR,0); /* Output */
    for (int i=ACSI_D0;i<=ACSI_D7;i++) {
        gpio_set_dir(i,GPIO_OUT);
        gpio_put(i, 1);
    }

}

void dpins_low() {
    gpio_put(ACSI_D_DIR,0); /* Output */
    for (int i=ACSI_D0;i<=ACSI_D7;i++) {
        gpio_set_dir(i,GPIO_OUT);
        gpio_put(i, 0);
    }
}

int setup_acsi_gpio() {

/**
 * Setup gpios and dissable pulls as we have them on the board
*/
    gpio_init(ACSI_IRQ);
    gpio_set_dir(ACSI_IRQ, GPIO_OUT);
    gpio_disable_pulls(ACSI_IRQ);
    gpio_put(ACSI_IRQ,1);

    for (int i=ACSI_D0;i<=ACSI_D7;i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_disable_pulls(i);
        gpio_put(i,0);
    }
    /* ACSI RW (from ST) direction of data pins 
        We need to change ACSI_D_DIR as well */
    gpio_init(ACSI_RW);
    gpio_set_dir(ACSI_RW, GPIO_IN);
    gpio_disable_pulls(ACSI_RW);

    /* DIR on 74LS641-1 (Data pins)*/
    gpio_init(ACSI_D_DIR);
    gpio_set_dir(ACSI_D_DIR, GPIO_OUT);
    gpio_disable_pulls(ACSI_D_DIR);
    gpio_put(ACSI_D_DIR,1);  /* HIGH = INPUT */

    /* Reset from ST (active low)*/
    gpio_init(ACSI_RESET);
    gpio_set_dir(ACSI_RESET,GPIO_IN);
    gpio_disable_pulls(ACSI_RESET);

    /* A1 New command packet on bus*/
    gpio_init(ACSI_A1);
    gpio_set_dir(ACSI_A1, GPIO_IN);
    gpio_disable_pulls(ACSI_A1);

    /* DMA Signals*/
    gpio_init(ACSI_ACK);
    gpio_set_dir(ACSI_ACK, GPIO_IN);
    gpio_disable_pulls(ACSI_ACK);

    gpio_init(ACSI_CS);
    gpio_set_dir(ACSI_CS, GPIO_IN);
    gpio_disable_pulls(ACSI_CS);

    gpio_init(ACSI_DRQ);
    gpio_set_dir(ACSI_DRQ,GPIO_OUT);
    gpio_disable_pulls(ACSI_DRQ);
    gpio_put(ACSI_DRQ,1);

} /* setup_acsi_gpio() */

/* Functions for switching PIO instance */

void PIO_IRQ_handler() {

    if (pio_interrupt_get(pio,0)) {
        uint data = pio_sm_get(pio,0);
        uint cmd = data & 0x1f;
        uint id = data >> 5;
        printf("id: %x cmd: %x",id,cmd);
        uint ret = 0;
        if (id = ACSI_ID) {
            ret = 5;
            
        }
    pio_sm_put(pio,0,ret);
    pio_interrupt_clear(pio,0);
    }
}
uint get_device_number(uint d) {
    return d >> 5;
}
int get_block_address(uind, dh,dm,dl) {
    int a=0;
    a = ((dh & 0x1f) << 16 ) | (dm << 8) | dl;
    return a; 
}
int acsi_write_status(uint status) {
    //TODO: 1-3 second timeout
    pio_sm_put_blocking(pio_snd_status,sm_snd_status,status); 
    return 0;    
}
int send_dummy_bootsector() {
    
    //Set bus to output
    gpio_put(ACSI_D_DIR,ACSI_BUS_OUTPUT);  /* HIGH = INPUT */
    pio_sm_set_consecutive_pindirs(pio_dma,0,ACSI_D0,8,true);
    //Send bootsector over bus
    for (int i=0;i<512;i++) {
        pio_sm_put_blocking(pio_dma, 0,_fuji2bootsector[i]);
    }
    sleep_us(12); // wait for dma to finnish

    //Send status byte
    pio_sm_put_blocking(pio_snd_status,sm_snd_status,ERROR_OK);
    sleep_us(15); //Wait a bit
    // Return bus to input
    gpio_put(ACSI_D_DIR,ACSI_BUS_INPUT);
    pio_sm_set_consecutive_pindirs(pio_dma, 0, ACSI_D0, 8,false);

    return 0;
}

int main() {
    int i;
    uint8_t d,id,cmd;
    uint8_t cdb[6];
    bool unit_ready = true; // Unit is always ready (for now) 
    stdio_init_all();
    //sleep_ms(8000);
    printf("\n\nPico ACSI controller\n\n");
    printf("Init GPIOs...\n\n");
    
    setup_acsi_gpio();
    
    printf("Init LED..\n\n");   
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN,GPIO_OUT);
    gpio_put(LED_PIN,1);
    sleep_ms(1000);
    gpio_put(LED_PIN,0);
    sleep_ms(4000);
    
    /* Core1 to be used for communication (not implementet yet)*/
    /*printf("Starting Core1 (ACSI handling.)\n \n");
    multicore_launch_core1(core1_entry);*/
    printf("Setting up PIO.\n");
    
    uint offset_snd_status = pio_add_program(pio_snd_status, &send_status_program);
    send_status_program_init(pio_snd_status, sm_snd_status, offset_snd_status, ACSI_IRQ);
    //Setup ACSI cmd program on pio0
     uint offset_dma = pio_add_program(pio_dma,&acsi_dma_out_program);
    acsi_dma_out_program_init(pio_dma,sm_dma, offset_dma,ACSI_DRQ);

     uint offset_cmd = pio_add_program(pio, &wait_cmd_program);
    wait_cmd_program_init(pio, sm_cmd, offset_cmd, ACSI_IRQ);

    printf("PIO setup done.\n");
    uint8_t data[6*8];
    uint8_t ad,aid,acmd,bytes_to_fetch;
    while(1) {
    wait_cmd:
    ad = (uint8_t)pio_sm_get_blocking(pio,0);
    acmd = ad & 0x1f;
    aid = ad >> 5;
    bytes_to_fetch = 5;
    if (aid == ACSI_ID) {
        if (acmd == CMD_ICD_EXT) {
            // ICD extentions
            bytes_to_fetch = 10;
        } 
        pio_sm_put(pio,0,bytes_to_fetch-1); // Send bytes to read to PIO (-1 as 0 counts)
        data[0]=ad;
    }
    else {
        pio_sm_put(pio,0,0); // Ignore command (fetch 0 bytes)
        goto wait_cmd;
    }
    // Read aditional bytes
    // TODO: add 3s timeout
    for (int i=1;i<6;i++) { 
        data[i]= (uint8_t)pio_sm_get_blocking(pio,0);
    }
    switch (acmd) {
        case CMD_TEST_UNIT_READY:
            if (unit_ready) {
                acsi_write_status(ERROR_OK);
            }
            else {
                acsi_write_status(-1);
            }
            
            break;
        
        case CMD_READ:
            // Get device number (6 Floppy A 7 Floppy B)
            if (get_device_number(data[1]) == 0) {
                int block = get_block_address(data[1],data[2],data[3]);
                if (block == 0 && dummy_bootsector && data[4] == 1) {
                    send_dummy_bootsector();
                }
            }
            break;
        case CMD_WRITE:
            break;
        case CMD_SEEK:
            acsi_write_status(ERROR_OK); //No need to seek (in modern times)
            break;
        case CMD_REQUEST_SENSE:
            break;
        case CMD_MODE_SENSE:
            break;
        case CMD_MODE_SELECT:
            break;
        case CMD_INQUERY:
            break;
        default:
            /* Command not implemented */
            acsi_write_status(ERROR_INVALID_COMMAND);
    }

    for (int i=0;i<6;i++) {
        printf("0x%02x, ",data[i]);
    }
    }
    return 0;
}