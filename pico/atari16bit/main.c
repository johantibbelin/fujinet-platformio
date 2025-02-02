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

// ACSI ID- id to repond to

#define ACSI_ID 2

// PIO instances global

PIO pio = pio0;
PIO pio_dma = pio1;
PIO pio_snd_status =pio1;
/**
 * ACSI is handled by core1
 */
void core1_entry() {
    //Core one code goes here
    sleep_ms(5000);
    printf("\nCore 1 Started.\n");
    // Setup PIO
    printf("Setting up PIO.\n");
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &wait_cmd_program);
    wait_cmd_program_init(pio, sm, offset, 6);
    printf("PIO setup done.\n");
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

void acsi_dma_out_enable(PIO pio,uint sm) {
  pio_gpio_init(pio,8);
  pio_gpio_init(pio,9);
  pio_gpio_init(pio,10);
  pio_gpio_init(pio,11);
  pio_gpio_init(pio,12);
  pio_gpio_init(pio,13);
  pio_gpio_init(pio,14);
  pio_gpio_init(pio,15);
  pio_sm_set_consecutive_pindirs(pio,sm,8,8,true);
  pio_sm_set_enabled(pio, sm, true);
}
void acsi_dma_out_disable(PIO pio,uint sm) {
  pio_sm_set_consecutive_pindirs(pio,sm,8,8,false);
  pio_sm_set_enabled(pio, sm, false);
  for(int i=ACSI_D0;i<=ACSI_D7;i++) {
    gpio_init(i);
    gpio_set_dir(i,GPIO_IN);
    gpio_disable_pulls(i);
    }
}

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


int main() {
    int i;
    uint8_t d,id,cmd;
    uint8_t cdb[6];
    stdio_init_all();
    //sleep_ms(8000);
    printf("Pico ACSI controller\n\n");
    printf("Init GPIOs...\n\n");
    
    setup_acsi_gpio();
    
    printf("Init LED..\n\n");   
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN,GPIO_OUT);
    gpio_put(LED_PIN,1);
    sleep_ms(1000);
    gpio_put(LED_PIN,0);
    sleep_ms(4000);
    
    /*printf("Starting Core1 (ACSI handling.)\n \n");
    multicore_launch_core1(core1_entry);*/
    printf("Setting up PIO.\n");
    
    uint sm_snd_status = 1;
    uint offset_snd_status = pio_add_program(pio_snd_status, &send_status_program);
    send_status_program_init(pio_snd_status, sm_snd_status, offset_snd_status, ACSI_IRQ);
    //Setup ACSI cmd program on pio0
    uint sm_dma = 0;
    uint offset_dma = pio_add_program(pio_dma,&acsi_dma_out_program);
    acsi_dma_out_program_init(pio_dma,sm_dma, offset_dma,ACSI_DRQ);

    uint sm_cmd=0;
    uint offset_cmd = pio_add_program(pio, &wait_cmd_program);
    wait_cmd_program_init(pio, sm_cmd, offset_cmd, ACSI_IRQ);

    /* Setup interrupt and handler */
    //pio_set_irq0_source_enabled(pio, pis_interrupt0, true); // sets IRQ0
    
    //irq_set_exclusive_handler(PIO0_IRQ_0, PIO_IRQ_handler);  //Set the handler in the NVIC
    //irq_set_enabled(0, true);
    
    // Setup dma and Status_byte PIO porgrams on pio1
    //offset = pio_add_program(pio_dma,&acsi_dma_out_program);
    //acsi_dma_out_program_init(pio_dma,sm,offset,ACSI_DRQ);

    printf("PIO setup done.\n");
    uint8_t data[6*8];
    uint8_t ad,aid,acmd;
    while(1) {
    wait_cmd:
    ad = (uint8_t)pio_sm_get_blocking(pio,0);
    acmd = ad & 0x1f;
    aid = ad >> 5;
    if (aid == ACSI_ID) {
        pio_sm_put(pio,0,4); // Five more bytes to read
        data[0]=ad;
    }
    else {
        pio_sm_put(pio,0,0); // Ignore command
        goto wait_cmd;
    }
    // Read aditional bytes

    for (int i=1;i<6;i++) { 
        data[i]= (uint8_t)pio_sm_get_blocking(pio,0);
    }
    //Setup dma and send 16 bytes
    gpio_put(ACSI_D_DIR,0);  /* HIGH = INPUT */
    pio_sm_set_consecutive_pindirs(pio_dma,0,8,8,true);
    //acsi_dma_out_enable(pio_dma,0);
    for (int i=0;i<512;i++) {
        pio_sm_put_blocking(pio_dma, 0,_fuji2bootsector[i]);
    }
    sleep_us(12); // wait for dma to finnish

    //pio_gpio_init(pio_snd_status,ACSI_IRQ);
    pio_sm_put_blocking(pio_snd_status,sm_snd_status,0); //status ok
    //sleep_us(10);
    pio_sm_put_blocking(pio,0,1);
    sleep_us(15);
    gpio_put(ACSI_D_DIR,1);
    pio_sm_set_consecutive_pindirs(pio_dma, 0, 8, 8,false);
    //acsi_dma_out_disable(pio_dma,0);
    for (int i=0;i<6;i++) {
        printf("0x%02x, ",data[i]);
    }
        /*if(!gpio_get(ACSI_A1)) {
           if (!gpio_get(ACSI_CS)) {
              d = 0;  
              for (i=0;i<7;i++) {
                d = d + (gpio_get(i+8) << i);
              }  
              id = d >> 5;
              cmd = d && 0x1f;
              cdb[0]=d;
              if (id == 0) {
                gpio_put(ACSI_IRQ,0);
                sleep_us(2);
                gpio_put(ACSI_IRQ,1);

                // Get rest of the 5 cdb bytes 
              }
           }
            
        }*/
    }
    return 0;
}