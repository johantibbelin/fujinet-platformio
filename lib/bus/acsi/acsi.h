#ifndef ACSI_H
#define ACSI_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <forward_list>


/* TODO: change to acsi command frame */
struct cmdFrame_t
{
    uint8_t devcmd;
    uint8_t bladrhi;
    uint8_t bladrmid;
    uint8_t bladrlow;
    uint8_t blcount;
    uint8_t ctrlbyte;
};

// helper functions


// class def'ns
class modem;          // declare here so can reference it, but define in modem.h
class sioFuji;        // declare here so can reference it, but define in fuji.h
class systemBus;      // declare early so can be friend
class sioNetwork;     // declare here so can reference it, but define in network.h
class sioUDPStream;   // declare here so can reference it, but define in udpstream.h
class sioCassette;    // Cassette forward-declaration.
class sioCPM;         // CPM device.
class sioPrinter;     // Printer device

class virtualDevice
{
protected:
    friend systemBus;

    int _devnum;

    cmdFrame_t cmdFrame;
    bool listen_to_type3_polls = false;

    /**
     * @brief Send the desired buffer to the Atari.
     * @param buff The byte buffer to send to the Atari
     * @param len The length of the buffer to send to the Atari.
     * @return TRUE if the Atari processed the data in error, FALSE if the Atari successfully processed
     * the data.
     */
    void bus_to_computer(uint8_t *buff, uint16_t len, bool err);

    /**
     * @brief Receive data from the Atari.
     * @param buff The byte buffer provided for data from the Atari.
     * @param len The length of the amount of data to receive from the Atari.
     * @return An 8-bit wrap-around checksum calculated by the Atari, which should be checked with sio_checksum()
     */
    uint8_t bus_to_peripheral(uint8_t *buff, uint16_t len);

    /**
     * @brief Send an acknowledgement byte to the Atari 'A'
     * This should be used if the command received by the SIO device is valid, and is used to signal to the
     * Atari that we are now processing the command.
     */
    void sio_ack();

    /**
     * @brief Send a non-acknowledgement (NAK) to the Atari 'N'
     * This should be used if the command received by the SIO device is invalid, in the first place. It is not
     * the same as sio_error().
     */
    void sio_nak();

    /**
     * @brief Send a COMPLETE to the Atari 'C'
     * This should be used after processing of the command to indicate that we've successfully finished. Failure to send
     * either a COMPLETE or ERROR will result in a SIO TIMEOUT (138) to be reported in DSTATS.
     */
    void sio_complete();

    /**
     * @brief Send an ERROR to the Atari 'E'
     * This should be used during or after processing of the command to indicate that an error resulted
     * from processing the command, and that the Atari should probably re-try the command. Failure to
     * send an ERROR or COMPLTE will result in a SIO TIMEOUT (138) to be reported in DSTATS.
     */
    void sio_error();

    /**
     * @brief Return the two aux bytes in cmdFrame as a single 16-bit value, commonly used, for example to retrieve
     * a sector number, for disk, or a number of bytes waiting for the sioNetwork device.
     * 
     * @return 16-bit value of DAUX1/DAUX2 in cmdFrame.
     */
    unsigned short sio_get_aux();

    /**
     * @brief All SIO commands by convention should return a status command, using bus_to_computer() to return
     * four bytes of status information to be put into DVSTAT ($02EA)
     */
    virtual void sio_status() = 0;

    /**
     * @brief All SIO devices repeatedly call this routine to fan out to other methods for each command. 
     * This is typcially implemented as a switch() statement.
     */
    virtual void sio_process(uint32_t commanddata, uint8_t checksum) = 0;

    // Optional shutdown/reboot cleanup routine
    virtual void shutdown(){};

public:
    /**
     * @brief get the SIO device Number (1-255)
     * @return The device number registered for this device
     */
    int id() { return _devnum; };

    /**
     * @brief Command 0x3F '?' intended to return a single byte to the atari via bus_to_computer(), which
     * signifies the high speed SIO divisor chosen by the user in their #FujiNet configuration.
     */
    virtual void sio_high_speed();

    /**
     * @brief Is this virtualDevice holding the virtual disk drive used to boot CONFIG?
     */
    bool is_config_device = false;

    /**
     * @brief is device active (turned on?)
     */
    bool device_active = true;

    /**
     * @brief status wait counter
     */
    uint8_t status_wait_count = 5;

    /**
     * @brief Get the systemBus object that this virtualDevice is attached to.
     */
    systemBus sio_get_bus();
};

enum sio_message : uint16_t
{
    SIOMSG_DISKSWAP,  // Rotate disk
    SIOMSG_DEBUG_TAPE // Tape debug msg
};

struct sio_message_t
{
    sio_message message_id;
    uint16_t message_arg;
};

// typedef sio_message_t sio_message_t;

class systemBus
{
private:
    std::forward_list<virtualDevice *> _daisyChain;

    int _command_frame_counter = 0;

    virtualDevice *_activeDev = nullptr;
    modem *_modemDev = nullptr;
    sioFuji *_fujiDev = nullptr;
    sioNetwork *_netDev[8] = {nullptr};
    sioUDPStream *_udpDev = nullptr;
    sioCassette *_cassetteDev = nullptr;
    sioCPM *_cpmDev = nullptr;
    sioPrinter *_printerdev = nullptr;

 
public:
    void setup();
    void service();
    void shutdown();

    int numDevices();
    void addDevice(virtualDevice *pDevice, int device_id);
    void remDevice(virtualDevice *pDevice);
    virtualDevice *deviceById(int device_id);
    void changeDeviceId(virtualDevice *pDevice, int device_id);

    int getBaudrate();                                          // Gets current SIO baud rate setting
    void setBaudrate(int baud);                                 // Sets SIO to specific baud rate
    void toggleBaudrate();                                      // Toggle between standard and high speed SIO baud rate

    int setHighSpeedIndex(int hsio_index);                      // Set HSIO index. Sets high speed SIO baud and also returns that value.
    int getHighSpeedIndex();                                    // Gets current HSIO index
    int getHighSpeedBaud();                                     // Gets current HSIO baud

    void setUDPHost(const char *newhost, int port);             // Set new host/ip & port for UDP Stream
    void setUltraHigh(bool _enable, int _ultraHighBaud = 0);    // enable ultrahigh/set baud rate
 //   bool getUltraHighEnabled() { return useUltraHigh; }
 //   int getUltraHighBaudRate() { return _sioBaudUltraHigh; }

    bool shuttingDown = false;                                  // TRUE if we are in shutdown process
    bool getShuttingDown() { return shuttingDown; };

    sioCassette *getCassette() { return _cassetteDev; }
    sioPrinter *getPrinter() { return _printerdev; }
    sioCPM *getCPM() { return _cpmDev; }

    // I wish this codebase would make up its mind to use camel or snake casing.
    modem *get_modem() { return _modemDev; }

    QueueHandle_t qSioMessages = nullptr;
};

extern systemBus ACSI;

#endif // guard