#ifdef BUILD_ATARI16BIT

#include "fuji.h"

#include <cstring>

#include "../../include/debug.h"

#include "fnSystem.h"
#include "fnConfig.h"
#include "fnWiFi.h"
#include "fsFlash.h"

#include "utils.h"

#define ADDITIONAL_DETAILS_BYTES 12

ACSIFuji theFuji;        // global fuji device object
ACSINetwork *theNetwork; // global network device object (temporary)
ACSIPrinter *thePrinter; // global printer

// sioDisk sioDiskDevs[MAX_HOSTS];
// sioNetwork sioNetDevs[MAX_NETWORK_DEVICES];

bool _validate_host_slot(uint8_t slot, const char *dmsg = nullptr);
bool _validate_device_slot(uint8_t slot, const char *dmsg = nullptr);

bool _validate_host_slot(uint8_t slot, const char *dmsg)
{
    if (slot < MAX_HOSTS)
        return true;

    if (dmsg == NULL)
    {
        Debug_printf("!! Invalid host slot %hu\n", slot);
    }
    else
    {
        Debug_printf("!! Invalid host slot %hu @ %s\n", slot, dmsg);
    }

    return false;
}

bool _validate_device_slot(uint8_t slot, const char *dmsg)
{
    if (slot < MAX_DISK_DEVICES)
        return true;

    if (dmsg == NULL)
    {
        Debug_printf("!! Invalid device slot %hu\n", slot);
    }
    else
    {
        Debug_printf("!! Invalid device slot %hu @ %s\n", slot, dmsg);
    }

    return false;
}

// Constructor
ACSIFuji::ACSIFuji()
{
    // Helpful for debugging
    for (int i = 0; i < MAX_HOSTS; i++)
        _fnHosts[i].slotid = i;
}

// Reset FujiNet
void ACSIFuji::ACSI_reset_fujinet()
{
    Debug_println("Fuji cmd: REBOOT");
    //ACSI_complete();
    fnSystem.reboot();
}

// Scan for networks
void ACSIFuji::ACSI_net_scan_networks()
{
   Debug_println("Fuji cmd: SCAN NETWORKS");

    char ret[4] = {0};

    _countScannedSSIDs = fnWiFi.scan_networks();

    ret[0] = _countScannedSSIDs;

    //bus_to_computer((uint8_t *)ret, 4, false);
}

// Return scanned network entry
void ACSIFuji::ACSI_net_scan_result()
{
    Debug_println("Fuji cmd: GET SCAN RESULT");

    // Response to  FUJICMD_GET_SCAN_RESULT
    struct
    {
        char ssid[MAX_SSID_LEN+1];
        uint8_t rssi;
    } detail;

    bool err = false;
    if (cmdFrame.aux1 < _countScannedSSIDs)
        fnWiFi.get_scan_result(cmdFrame.aux1, detail.ssid, &detail.rssi);
    else
    {
        memset(&detail, 0, sizeof(detail));
        err = true;
    }

    //bus_to_computer((uint8_t *)&detail, sizeof(detail), err);
}

//  Get SSID
void ACSIFuji::ACSI_net_get_ssid()
{
}

// Set SSID
void ACSIFuji::ACSI_net_set_ssid()
{
/*    Debug_println("Fuji cmd: SET SSID");
    int i;

    // Data for  FUJICMD_SET_SSID
    struct
    {
        char ssid[MAX_SSID_LEN + 1];
        char password[MAX_WIFI_PASS_LEN];
    } cfg;

    uint8_t ck = bus_to_peripheral((uint8_t *)&cfg, sizeof(cfg));

    if (acsi_checksum((uint8_t *)&cfg, sizeof(cfg)) != ck) {
        sio_error();
        return;
    }

    bool save = cmdFrame.aux1 != 0;

    Debug_printf("Connecting to net: >%s< password: >%s<\r\n", cfg.ssid, cfg.password);

    int test_result = fnWiFi.test_connect(cfg.ssid, cfg.password);
    if (test_result != 0)
    {
        Debug_println("Could not connect to target SSID. Aborting save.");
        sio_error();
        return;
    }

    // Only save these if we're asked to, otherwise assume it was a test for connectivity
    if (save)
    {
        // 1. if this is a new SSID and not in the old stored, we should push the current one to the top of the stored configs, and everything else down.
        // 2. If this was already in the stored configs, push the stored one to the top, remove the new one from stored so it becomes current only.
        // 3. if this is same as current, then just save it again. User reconnected to current, nothing to change in stored. This is default if above don't happen

        int ssid_in_stored = -1;
        for (i = 0; i < MAX_WIFI_STORED; i++)
        {
            if (Config.get_wifi_stored_ssid(i) == cfg.ssid)
            {
                ssid_in_stored = i;
                break;
            }
        }

        // case 1
        if (ssid_in_stored == -1 && Config.have_wifi_info() && Config.get_wifi_ssid() != cfg.ssid)
        {
            Debug_println("Case 1: Didn't find new ssid in stored, and it's new. Pushing everything down 1 and old current to 0");
            // Move enabled stored down one, last one will drop off
            for (int j = MAX_WIFI_STORED - 1; j > 0; j--)
            {
                bool enabled = Config.get_wifi_stored_enabled(j - 1);
                if (!enabled)
                    continue;

                Config.store_wifi_stored_ssid(j, Config.get_wifi_stored_ssid(j - 1));
                Config.store_wifi_stored_passphrase(j, Config.get_wifi_stored_passphrase(j - 1));
                Config.store_wifi_stored_enabled(j, true); // already confirmed this is enabled
            }
            // push the current to the top of stored
            Config.store_wifi_stored_ssid(0, Config.get_wifi_ssid());
            Config.store_wifi_stored_passphrase(0, Config.get_wifi_passphrase());
            Config.store_wifi_stored_enabled(0, true);
        }

        // case 2
        if (ssid_in_stored != -1 && Config.have_wifi_info() && Config.get_wifi_ssid() != cfg.ssid)
        {
            Debug_printf("Case 2: Found new ssid in stored at %d, and it's not current (should never happen). Pushing everything down 1 and old current to 0\n", ssid_in_stored);
            // found the new SSID at ssid_in_stored, so move everything above it down one slot, and store the current at 0
            for (int j = ssid_in_stored; j > 0; j--)
            {
                Config.store_wifi_stored_ssid(j, Config.get_wifi_stored_ssid(j - 1));
                Config.store_wifi_stored_passphrase(j, Config.get_wifi_stored_passphrase(j - 1));
                Config.store_wifi_stored_enabled(j, true);
            }

            // push the current to the top of stored
            Config.store_wifi_stored_ssid(0, Config.get_wifi_ssid());
            Config.store_wifi_stored_passphrase(0, Config.get_wifi_passphrase());
            Config.store_wifi_stored_enabled(0, true);
        }

        // save the new SSID as current
        Config.store_wifi_ssid(cfg.ssid, sizeof(cfg.ssid));
        // Clear text here, it will be encrypted internally if enabled for encryption
        Config.store_wifi_passphrase(cfg.password, sizeof(cfg.password));

        Config.save();
    }
    Debug_println("Restarting WiFiManager");
    fnWiFi.start();

    // give it a few seconds to restart the WiFi before we return to the client, who will immediately start checking status
    // and get errors if we're not up yet
    fnSystem.delay(3000);

    //sio_complete();*/
}

// Get WiFi Status
void ACSIFuji::ACSI_net_get_wifi_status()
{
}

// Mount Server
void ACSIFuji::ACSI_mount_host()
{
}

// Disk Image Mount
void ACSIFuji::ACSI_disk_image_mount()
{
}

// Toggle boot config on/off, aux1=0 is disabled, aux1=1 is enabled
void ACSIFuji::ACSI_set_boot_config()
{
}

// Do SIO copy
void ACSIFuji::ACSI_copy_file()
{
}

// Set boot mode
void ACSIFuji::ACSI_set_boot_mode()
{
}

char *_generate_appkey_filename(appkey *info)
{
    static char filenamebuf[30];

    snprintf(filenamebuf, sizeof(filenamebuf), "/FujiNet/%04hx%02hhx%02hhx.key", info->creator, info->app, info->key);
    return filenamebuf;
}

/*
 Opens an "app key".  This just sets the needed app key parameters (creator, app, key, mode)
 for the subsequent expected read/write command. We could've added this information as part
 of the payload in a WRITE_APPKEY command, but there was no way to do this for READ_APPKEY.
 Requiring a separate OPEN command makes both the read and write commands behave similarly
 and leaves the possibity for a more robust/general file read/write function later.
*/
void ACSIFuji::ACSI_open_app_key()
{
}

/*
  The app key close operation is a placeholder in case we want to provide more robust file
  read/write operations. Currently, the file is closed immediately after the read or write operation.
*/
void ACSIFuji::ACSI_close_app_key()
{
}

/*
 Write an "app key" to SD (ONLY!) storage.
*/
void ACSIFuji::ACSI_write_app_key()
{
}

/*
 Read an "app key" from SD (ONLY!) storage
*/
void ACSIFuji::ACSI_read_app_key()
{
}

// DEBUG TAPE
void ACSIFuji::debug_tape()
{
}

// Disk Image Unmount
void ACSIFuji::ACSI_disk_image_umount()
{
}

// Disk Image Rotate
/*
  We rotate disks my changing their disk device ID's. That prevents
  us from having to unmount and re-mount devices.
*/
void ACSIFuji::image_rotate()
{
}

// This gets called when we're about to shutdown/reboot
void ACSIFuji::shutdown()
{
    for (int i = 0; i < MAX_DISK_DEVICES; i++)
        _fnDisks[i].disk_dev.unmount();
}

char dirpath[256];

void ACSIFuji::ACSI_open_directory()
{
}

void _set_additional_direntry_details(fsdir_entry_t *f, uint8_t *dest, uint8_t maxlen)
{
}

void ACSIFuji::ACSI_read_directory_entry()
{
}

void ACSIFuji::ACSI_get_directory_position()
{
}

void ACSIFuji::ACSI_set_directory_position()
{
}

void ACSIFuji::ACSI_close_directory()
{
}

// Get network adapter configuration
void ACSIFuji::ACSI_get_adapter_config()
{
}

//  Make new disk and shove into device slot
void ACSIFuji::ACSI_new_disk()
{
}

// Send host slot data to computer
void ACSIFuji::ACSI_read_host_slots()
{
}

// Read and save host slot data from computer
void ACSIFuji::ACSI_write_host_slots()
{
}

// Store host path prefix
void ACSIFuji::ACSI_set_host_prefix()
{
}

// Retrieve host path prefix
void ACSIFuji::ACSI_get_host_prefix()
{
}

// Send device slot data to computer
void ACSIFuji::ACSI_read_device_slots()
{
}

// Read and save disk slot data from computer
void ACSIFuji::ACSI_write_device_slots()
{
}

// Temporary(?) function while we move from old config storage to new
void ACSIFuji::_populate_slots_from_config()
{
}

// Temporary(?) function while we move from old config storage to new
void ACSIFuji::_populate_config_from_slots()
{
}

char f[MAX_FILENAME_LEN];

// Write a 256 byte filename to the device slot
void ACSIFuji::ACSI_set_device_filename()
{
}

// Get a 256 byte filename from device slot
void ACSIFuji::ACSI_get_device_filename()
{
}


void ACSIFuji::ACSI_enable_device()
{
}

void ACSIFuji::ACSI_disable_device()
{
}

void ACSIFuji::ACSI_device_enabled_status()
{
}

// Initializes base settings and adds our devices to the SIO bus
void ACSIFuji::setup(systemBus *_bus)
{
    _ACSI_bus = _bus;

    _populate_slots_from_config();

    // Disable booting from CONFIG if our settings say to turn it off
    boot_config = false;

    // Disable status_wait if our settings say to turn it off
    status_wait_enabled = false;
}

// Mount all
void ACSIFuji::mount_all()
{
}

ACSIDisk *ACSIFuji::bootdisk()
{
    return _bootDisk;
}


void ACSIFuji::process(uint32_t commanddata, uint8_t checksum)
{
    cmdFrame.commanddata = commanddata;
    cmdFrame.checksum = checksum;

    switch (cmdFrame.comnd)
    {
    // case FUJICMD_STATUS:
    //     H89_response_ack();
    //     break;
    case FUJICMD_RESET:
        ACSI_reset_fujinet();
        break;
    case FUJICMD_SCAN_NETWORKS:
        ACSI_net_scan_networks();
        break;
    case FUJICMD_GET_SCAN_RESULT:
        ACSI_net_scan_result();
        break;
    case FUJICMD_SET_SSID:
        ACSI_net_set_ssid();
        break;
    case FUJICMD_GET_SSID:
        ACSI_net_get_ssid();
        break;
    case FUJICMD_GET_WIFISTATUS:
        ACSI_net_get_wifi_status();
        break;
    case FUJICMD_MOUNT_HOST:
        ACSI_mount_host();
        break;
    case FUJICMD_MOUNT_IMAGE:
        ACSI_disk_image_mount();
        break;
    case FUJICMD_OPEN_DIRECTORY:
        ACSI_open_directory();
        break;
    case FUJICMD_READ_DIR_ENTRY:
        ACSI_read_directory_entry();
        break;
    case FUJICMD_CLOSE_DIRECTORY:
        ACSI_close_directory();
        break;
    case FUJICMD_GET_DIRECTORY_POSITION:
        ACSI_get_directory_position();
        break;
    case FUJICMD_SET_DIRECTORY_POSITION:
        ACSI_set_directory_position();
        break;
    case FUJICMD_READ_HOST_SLOTS:
        ACSI_read_host_slots();
        break;
    case FUJICMD_WRITE_HOST_SLOTS:
        ACSI_write_host_slots();
        break;
    case FUJICMD_READ_DEVICE_SLOTS:
        ACSI_read_device_slots();
        break;
    case FUJICMD_WRITE_DEVICE_SLOTS:
        ACSI_write_device_slots();
        break;
    //case FUJICMD_GET_WIFI_ENABLED:
    //    H89_net_get_wifi_enabled();
    //    break;
    case FUJICMD_UNMOUNT_IMAGE:
        ACSI_disk_image_umount();
        break;
    case FUJICMD_GET_ADAPTERCONFIG:
        ACSI_get_adapter_config();
        break;
    // case FUJICMD_NEW_DISK:
    //     rs232_ack();
    //     rs232_new_disk();
    //     break;
    case FUJICMD_SET_DEVICE_FULLPATH:
        ACSI_set_device_filename();
        break;
    // case FUJICMD_SET_HOST_PREFIX:
    //     H89_set_host_prefix();
    //     break;
    // case FUJICMD_GET_HOST_PREFIX:
    //     H89_get_host_prefix();
    //     break;
    // case FUJICMD_WRITE_APPKEY:
    //     H89_write_app_key();
    //     break;
    // case FUJICMD_READ_APPKEY:
    //     H89_read_app_key();
    //     break;
    // case FUJICMD_OPEN_APPKEY:
    //     H89_open_app_key();
    //     break;
    // case FUJICMD_CLOSE_APPKEY:
    //     H89_close_app_key();
    //     break;
    case FUJICMD_GET_DEVICE_FULLPATH:
        ACSI_get_device_filename();
        break;
    case FUJICMD_CONFIG_BOOT:
        ACSI_set_boot_config();
        break;
    // case FUJICMD_COPY_FILE:
    //     rs232_ack();
    //     rs232_copy_file();
    //     break;
    case FUJICMD_MOUNT_ALL:
        mount_all();
        break;
    // case FUJICMD_SET_BOOT_MODE:
    //     rs232_ack();
    //     rs232_set_boot_mode();
    //     break;
    // case FUJICMD_ENABLE_UDPSTREAM:
    //     rs232_ack();
    //     rs232_enable_udpstream();
    //     break;
    case FUJICMD_ENABLE_DEVICE:
        ACSI_enable_device();
        break;
    case FUJICMD_DISABLE_DEVICE:
        ACSI_disable_device();
        break;
    // case FUJICMD_RANDOM_NUMBER:
        // H89_random_number();
        // break;
    // case FUJICMD_GET_TIME:
        // H89_get_time();
        // break;
    case FUJICMD_DEVICE_ENABLE_STATUS:
        ACSI_device_enabled_status();
        break;
    default:
        fnUartDebug.printf("H89_process() not implemented yet for this device. Cmd received: %02x\n", cmdFrame.comnd);
    }
}

int ACSIFuji::get_disk_id(int drive_slot)
{
    return _fnDisks[drive_slot].disk_dev.id();
}

std::string ACSIFuji::get_host_prefix(int host_slot)
{
    return _fnHosts[host_slot].get_prefix();
}

#endif /* NEW_TARGET */
