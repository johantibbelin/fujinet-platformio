#ifndef ACSI_H
#define ACSI_H

/**
 * Atari16bit Routines
 */

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <vector>

#include <map>

/**
 * ACSI Commands (AHDI)
*/

#define ACSI_CMD_TEST_UNIT_READY 0x00
#define ACSI_CMD_VERIFY_TRACK 0x05
#define ACSI_CMD_FORMAT_TRACK 0x06
#define ACSI_CMD_READ 0x08                  /* Read sectors with implied seek */
#define ACSI_CMD_WRITE 0x0a                 /* Write sectors with implied seek */
#define ACSI_CMD_SEEK 0x0b                  /* Seek sector */
#define ACSI_CMD_CORRECTION_PATTERN 0x0d    /* Correction pattern */
#define ACSI_CMD_MODE_SELECT 0x15           /* Mode select */
#define ACSI_MODE_SENSE 0x1a                /* Mode Sense */

#define ACSI_CMD_INQUIRY 0x12               /* Inquiry command */

/**
 * ICD extension. SCSI command follows, up to 12 bytes
*/

#define ACSI_CMD_ICD_EXT 0x1f               /* ICD extension following (SCSI-2 commands) up to 12 more bytes */

/**
 * Printer command
*/

#define ACSI_CMD_PRINT 0x0a                 /* Same as WRITE (used by Atari Laser printers (and Fujinet)) */

/**
 * @brief ACSI command number to sed SIO command.
 * @details Fujinet SIO command over ACSI
 * Byte 0: bit 7-5:ID bit 4-0: 0x01
 * Byte 1: Device
 * Byte 2: Command
 * Byte 3: Aux1
 * byte 4: Aux2
 * byte 5: Checksum
*/

#define ACSI_CMD_FUJINET 0x02              /* Fujinet command (tunneling SIO commands) */

/**
 * @brief Fujinet ACSI command for network traffic.
*/

#define ACSI_CMD_NETWORK 0x09

/**
 * @brief SCSI commands for use with ICD extensions.
 * @details Used to overcome the 1 GB limit built into ACSI (21 bit block adress.)
*/

#define SCSI_CMD_READ_CAPACITY 0x25
#define SCSI_CMD_READ10 0x28
#define SCSI_CMD_WRITE10 0x2a
#define SCSI_CMD_VERIFY 0x2f
#define SCSI_CMD_READ_LONG 0x3e
#define SCSI_CMD_WRITE_LONG 0x3f
#define SCSI_CMD_REPORT_LUNS 0xa0

/**
 * @brief ACSI inquiry string for fujinet
*/

static char _acsi_inquiry[32] ="FujiNet \0";

/* Using SIO device IDs for ACSI (internally) */
#define ACSI_BAUDRATE 2000000

#define ACSI_DEVICEID_DISK 0x31
#define ACSI_DEVICEID_DISK_LAST 0x3F

#define ACSI_DEVICEID_PRINTER 0x41
#define ACSI_DEVICEID_PRINTER_LAST 0x44


#define ACSI_DEVICEID_FUJINET 0x70
#define ACSI_DEVICEID_FN_NETWORK 0x71
#define ACSI_DEVICEID_FN_NETWORK_LAST 0x78

#define ACSI_DEVICEID_MODEM 0x50

#define ACSI_DEVICEID_CPM 0x5A

#define DELAY_T4 800
#define DELAY_T5 800

// This is used for the network protocol adapters.
union cmdFrame_t
{
    struct
    {
        uint8_t device;
        uint8_t comnd;
        uint8_t aux1;
        uint8_t aux2;
        uint8_t cksum;
    };
    struct
    {
        uint32_t commanddata;
        uint8_t checksum;
    } __attribute__((packed));
};

/**
 * @brief Type for tunneling SIO cmdFrames through ACSI. Command number 0x01.
*/

struct acsiCmdFrame_t
{
    uint8_t deviceId_Cmd;
    cmdFrame_t SioCmdFrame;
};

/**
 * @brief ACSI command. 6 bytes.
*/

struct acsiCmd_t
{
    uint8_t deviceId_Cmd;           /* Device Id and command */
    uint8_t logicId_blockHi;
    uint8_t blockMid;
    uint8_t blockLow;
    uint8_t blockCount;
    uint8_t controlByte;
};

/**
 * @brief ACSI command with ICD extension. Sends SCSI-1 and SCSI-2 commands over ACSI.
*/

struct acsiCmd_icdExt_t
{
    uint8_t deviceId_Cmd; /* always ID + 0x1f */
    uint8_t scsiCmd[12];  /* SCSI command */
};

class systemBus;
class ACSIFuji;     // declare here so can reference it, but define in fuji.h
class ACSICPM;
class ACSIModem;
class ACSIPrinter;
//class modem;

/**
 * @brief An ACSI Device
 */
class virtualDevice
{
protected:
    friend systemBus; // We exist on the ACSI Bus, and need its methods.

    void bus_to_computer(uint8_t *buf, uint16_t len, bool err);

    uint8_t bus_to_peripheral(uint8_t *buf, unsigned short len);
    
    /**
     * @brief Device Number: 0-255
     */
    uint8_t _devnum;

    virtual void shutdown() {}

    /**
     * @brief Perform reset of device
     */
    virtual void reset() {};

    /**
     * @brief All ACSI devices repeatedly call this routine to fan out to other methods for each command. 
     * This is typcially implemented as a switch() statement.
     */
    virtual void process(uint32_t commanddata, uint8_t checksum) = 0;
    
    /**
     * @brief send current status of device
     */
    virtual void status() {};



    /**
     * @brief command frame, used by network protocol, ultimately
     */
    cmdFrame_t cmdFrame;
    
    systemBus acsi_get_bus();

    void acsi_error();

    void acsi_complete();

public:

    /**
     * @brief is device active (turned on?)
     */
    bool device_active = true;

    /**
     * @brief return the device number (0-15) of this device
     * @return the device # (0-15) of this device
     */
    uint8_t id() { return _devnum; }
};

/**
 * @brief The ACSI Bus
 */
class systemBus
{
private:
    std::map<uint8_t, virtualDevice *> _daisyChain;
    ACSIModem *_modemDev = nullptr;
public:
    void setup(); // one time setup
    void service(); // this runs in a loop 
    void shutdown(); // shutdown
    void reset(); // reset

    int numDevices();
    void addDevice(virtualDevice *pDevice, uint8_t device_id);
    void remDevice(virtualDevice *pDevice);
    void remDevice(uint8_t device_id);
    bool deviceExists(uint8_t device_id);
    void enableDevice(uint8_t device_id);
    void disableDevice(uint8_t device_id);
    bool enabledDeviceStatus(uint8_t device_id);
    virtualDevice *deviceById(uint8_t device_id);
    void changeDeviceId(virtualDevice *pDevice, uint8_t device_id);
    QueueHandle_t qH89Messages = nullptr;

    bool shuttingDown = false;                                  // TRUE if we are in shutdown process
    bool getShuttingDown() { return shuttingDown; };


   ACSIModem *get_modem() { return _modemDev; }
};

extern systemBus ACSI;

#endif /* ACSI_H */
