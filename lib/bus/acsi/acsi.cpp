#ifdef BUILD_ATARI16BIT

/**
 * ACSI Functions
 */
#include "acsi.h"

#include "../../include/debug.h"
#include "driver/spi_slave.h"


#include "fnConfig.h"
#include "fnSystem.h"

#include "led.h"
#include "modem.h" 

// Calculate 8-bit checksum
uint8_t acsi_checksum(uint8_t *buf, unsigned short len)
{
    unsigned int chk = 0;

    for (int i = 0; i < len; i++)
        chk = ((chk + buf[i]) >> 8) + ((chk + buf[i]) & 0xff);

    return chk;
}
/**
 * @brief Returns top 3 bits which is ACSI id.
*/

uint8_t acsi_get_id(uint8_t b) { return (b >> 5);}

uint8_t acsi_inquiry() 
{
    Debug_println("ACSI Inquiry");
    //Debug_printf("%s",_acsi_inquiry);
    return 0;
}
/**
 * @brief 5 least significant bit form a byte. ACSI command.
 * @return ACSI command
*/

uint8_t acsi_get_cmd(uint8_t b) { return (b && 0x1f); }

void virtualDevice::acsi_complete()
{
    fnSystem.delay_microseconds(DELAY_T5);
#ifdef ESP_PLATFORM
   // acsi_get_bus().get_modem()->get_uart()->write('C');
#else
    fnSioCom.write('C');
#endif
    Debug_println("COMPLETE!");
}

void virtualDevice::bus_to_computer(uint8_t *buf, uint16_t len, bool err)
{
    // Write data frame to computer
    Debug_printf("->ACSI write %hu bytes\n", len);

#ifdef VERBOSE_ACSI
    Debug_printf("SEND <%u> BYTES\n\t", len);
    for (int i = 0; i < len; i++)
        Debug_printf("%02x ", buf[i]);
    Debug_print("\n");
#endif

    // Write ERROR or COMPLETE status
    if (err == true)
        acsi_error();
    else
        acsi_complete();

    // Write data frame
#ifdef ESP_PLATFORM
  //  UARTManager *uart = acsi_get_bus().get_modem()->get_uart();
//    uart->write(buf, len);
    // Write checksum
  //  uart->write(acsi_checksum(buf, len));

 //   uart->flush();
#else
    fnSioCom.write(buf, len);
    // Write checksum
    fnSioCom.write(sio_checksum(buf, len));

    fnSioCom.flush();
#endif
}

// TODO apc: change return type to indicate valid/invalid checksum
/*
   SIO READ from ATARI by DEVICE
   buf = buffer from atari to fujinet
   len = length
   Returns checksum
*/
uint8_t virtualDevice::bus_to_peripheral(uint8_t *buf, unsigned short len)
{
    return 0;
}

void virtualDevice::process(uint32_t commanddata, uint8_t checksum)
{
    cmdFrame.commanddata = commanddata;
    cmdFrame.checksum = checksum;


    fnUartDebug.printf("process() not implemented yet for this device. Cmd received: %02x\n", cmdFrame.comnd);
}
systemBus virtualDevice::acsi_get_bus() { return ACSI; }

void systemBus::service()
{
    uint8_t buf[32];
    // Listen to the bus, and react here.
    if (fnUartBUS.available())
    {
        int c=fnUartBUS.read();
        if (c==0) return;
        else if (c == 'C') // acsi command follows
       {
        fnUartBUS.readBytes(buf,6);
        Debug_println("ACSI command recieved.");
 
        }
    }
}

void virtualDevice::acsi_error()
{
    fnSystem.delay_microseconds(DELAY_T5);
#ifdef ESP_PLATFORM
//    acsi_get_bus().get_modem()->get_uart()->write('E');
#else
    fnSioCom.write('E');
#endif
    Debug_println("ERROR!");
}

void systemBus::setup()
{
    Debug_println("ACSI SETUP");

    // // Setup PICO UART
    Debug_println("Setup UART for PICO...");
 
    
 
    fnSystem.set_pin_mode(PIN_UART1_RX, gpio_mode_t::GPIO_MODE_INPUT); // There's no PULLUP/PULLDOWN on pins 34-39
    fnSystem.set_pin_mode(PIN_UART1_TX, gpio_mode_t::GPIO_MODE_OUTPUT); // There's no PULLUP/PULLDOWN on pins 34-39


    fnUartBUS.begin(ACSI_BAUDRATE);
 

    fnUartBUS.flush();
    
   
 }

void systemBus::shutdown()
{
    shuttingDown = true;

    for (auto devicep : _daisyChain)
    {
        Debug_printf("Shutting down device %02x\n", devicep.second->id());
        devicep.second->shutdown();
    }
    Debug_printf("All devices shut down.\n");
}

void systemBus::addDevice(virtualDevice *pDevice, uint8_t device_id)
{
    Debug_printf("Adding device: %02X\n", device_id);
    pDevice->_devnum = device_id;
    _daisyChain[device_id] = pDevice;
}

bool systemBus::deviceExists(uint8_t device_id)
{
    return _daisyChain.find(device_id) != _daisyChain.end();
}

void systemBus::remDevice(virtualDevice *pDevice)
{

}

void systemBus::remDevice(uint8_t device_id)
{
    if (deviceExists(device_id))
    {
        _daisyChain.erase(device_id);
    }
}

int systemBus::numDevices()
{
    return _daisyChain.size();
}

void systemBus::changeDeviceId(virtualDevice *p, uint8_t device_id)
{
    for (auto devicep : _daisyChain)
    {
        if (devicep.second == p)
            devicep.second->_devnum = device_id;
    }
}

virtualDevice *systemBus::deviceById(uint8_t device_id)
{
    for (auto devicep : _daisyChain)
    {
        if (devicep.second->_devnum == device_id)
            return devicep.second;
    }
    return nullptr;
}

void systemBus::reset()
{
    for (auto devicep : _daisyChain)
        devicep.second->reset();
}

void systemBus::enableDevice(uint8_t device_id)
{
    if (_daisyChain.find(device_id) != _daisyChain.end())
        _daisyChain[device_id]->device_active = true;
}

void systemBus::disableDevice(uint8_t device_id)
{
    if (_daisyChain.find(device_id) != _daisyChain.end())
        _daisyChain[device_id]->device_active = false;
}

bool systemBus::enabledDeviceStatus(uint8_t device_id)
{
    if (_daisyChain.find(device_id) != _daisyChain.end())
        return _daisyChain[device_id]->device_active;

    return false;
}

systemBus ACSI;
#endif /* ACSI */
