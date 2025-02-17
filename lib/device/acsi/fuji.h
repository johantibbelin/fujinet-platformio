#ifndef FUJI_H
#define FUJI_H

#include <cstdint>

#include "network.h"
#include "disk.h"

#include "fujiHost.h"
#include "fujiDisk.h"
#include "fujiCmd.h"

#define MAX_HOSTS 8
#define MAX_DISK_DEVICES 8
#define MAX_NETWORK_DEVICES 4

#define MAX_SSID_LEN 32
#define MAX_WIFI_PASS_LEN 64

#define MAX_APPKEY_LEN 64

#define READ_DEVICE_SLOTS_DISKS1 0x00
#define READ_DEVICE_SLOTS_TAPE 0x10

typedef struct
{
    char ssid[32];
    char hostname[64];
    unsigned char localIP[4];
    unsigned char gateway[4];
    unsigned char netmask[4];
    unsigned char dnsIP[4];
    unsigned char macAddress[6];
    unsigned char bssid[6];
    char fn_version[15];
} AdapterConfig;

enum appkey_mode : uint8_t
{
    APPKEYMODE_READ = 0,
    APPKEYMODE_WRITE,
    APPKEYMODE_INVALID
};

struct appkey
{
    uint16_t creator = 0;
    uint8_t app = 0;
    uint8_t key = 0;
    appkey_mode mode = APPKEYMODE_INVALID;
    uint8_t reserved = 0;
} __attribute__((packed));

class ACSIFuji : public virtualDevice
{
private:
    bool isReady = false;
    bool alreadyRunning = false; // Replace isReady and scanStarted with THIS.
    bool scanStarted = false;
    bool hostMounted[MAX_HOSTS];
    bool setSSIDStarted = false;

    uint8_t response[1024];
    uint16_t response_len;

    systemBus *_ACSI_bus;

    fujiHost _fnHosts[MAX_HOSTS];

    fujiDisk _fnDisks[MAX_DISK_DEVICES];

    int _current_open_directory_slot = -1;

    ACSIDisk *_bootDisk; // special disk drive just for configuration

    uint8_t bootMode = 0; // Boot mode 0 = CONFIG, 1 = MINI-BOOT

    uint8_t _countScannedSSIDs = 0;

    appkey _current_appkey;

protected:
    void ACSI_reset_fujinet();          // 0xFF
    void ACSI_net_get_ssid();           // 0xFE
    void ACSI_net_scan_networks();      // 0xFD
    void ACSI_net_scan_result();        // 0xFC
    void ACSI_net_set_ssid();           // 0xFB
    void ACSI_net_get_wifi_status();    // 0xFA
    void ACSI_mount_host();             // 0xF9
    void ACSI_disk_image_mount();       // 0xF8
    void ACSI_open_directory();         // 0xF7
    void ACSI_read_directory_entry();   // 0xF6
    void ACSI_close_directory();        // 0xF5
    void ACSI_read_host_slots();        // 0xF4
    void ACSI_write_host_slots();       // 0xF3
    void ACSI_read_device_slots();      // 0xF2
    void ACSI_write_device_slots();     // 0xF1
    void ACSI_disk_image_umount();      // 0xE9
    void ACSI_get_adapter_config();     // 0xE8
    void ACSI_new_disk();               // 0xE7
    void ACSI_unmount_host();           // 0xE6
    void ACSI_get_directory_position(); // 0xE5
    void ACSI_set_directory_position(); // 0xE4
    void ACSI_set_hH89_index();      // 0xE3
    void ACSI_set_device_filename();    // 0xE2
    void ACSI_set_host_prefix();        // 0xE1
    void ACSI_get_host_prefix();        // 0xE0
    void ACSI_set_ACSI_external_clock(); // 0xDF
    void ACSI_write_app_key();          // 0xDE
    void ACSI_read_app_key();           // 0xDD
    void ACSI_open_app_key();           // 0xDC
    void ACSI_close_app_key();          // 0xDB
    void ACSI_get_device_filename();    // 0xDA
    void ACSI_set_boot_config();        // 0xD9
    void ACSI_copy_file();              // 0xD8
    void ACSI_set_boot_mode();          // 0xD6
    void ACSI_enable_device();          // 0xD5
    void ACSI_disable_device();         // 0xD4
    void ACSI_device_enabled_status();  // 0xD1

    void ACSI_test_command();

    void process(uint32_t commanddata, uint8_t checksum) override;

    void shutdown() override;

public:
    bool boot_config = true;
    
    bool status_wait_enabled = true;
    
    ACSIDisk *bootdisk();

    ACSINetwork *network();

    void debug_tape();

    void insert_boot_device(uint8_t d);

    void setup(systemBus *H89bus);

    void image_rotate();
    int get_disk_id(int drive_slot);
    std::string get_host_prefix(int host_slot);

    fujiHost *get_hosts(int i) { return &_fnHosts[i]; }
    fujiDisk *get_disks(int i) { return &_fnDisks[i]; }
    fujiHost *set_slot_hostname(int host_slot, char *hostname);
    void _populate_slots_from_config();
    void _populate_config_from_slots();

    void mount_all();              // 0xD7

    ACSIFuji();
};

extern ACSIFuji theFuji;

#endif // FUJI_H
