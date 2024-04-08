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


void virtualDevice::process(uint32_t commanddata, uint8_t checksum)
{
    cmdFrame.commanddata = commanddata;
    cmdFrame.checksum = checksum;


    fnUartDebug.printf("process() not implemented yet for this device. Cmd received: %02x\n", cmdFrame.comnd);
}

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
    
void systemBus::setup()
{
    Debug_println("ACSI SETUP");

    // // Setup PICO UART
    Debug_println("Setup UART for PICO...");
 
    
 
    /*fnSystem.set_pin_mode(PIN_UART1_RX, gpio_mode_t::GPIO_MODE_INPUT); // There's no PULLUP/PULLDOWN on pins 34-39
    fnSystem.set_pin_mode(PIN_UART1_TX, gpio_mode_t::GPIO_MODE_OUTPUT); // There's no PULLUP/PULLDOWN on pins 34-39
*/

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
