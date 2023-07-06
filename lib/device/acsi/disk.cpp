#ifdef BUILD_H89

#include "disk.h"

#include <memory.h>
#include <string.h>

#include "../../include/debug.h"

#include "media.h"
#include "utils.h"

ACSIDisk::ACSIDisk()
{
}

// Destructor
ACSIDisk::~ACSIDisk()
{
}

mediatype_t ACSIDisk::mount(FILE *f, const char *filename, uint32_t disksize, mediatype_t disk_type)
{
    mediatype_t mt = MEDIATYPE_UNKNOWN;

    Debug_printf("disk MOUNT %s\n", filename);

    // Destroy any existing MediaType
    if (_media != nullptr)
    {
        delete _media;
        _media = nullptr;
    }

    // Determine MediaType based on filename extension
    if (disk_type == MEDIATYPE_UNKNOWN && filename != nullptr)
        disk_type = MediaType::discover_mediatype(filename, disksize);

    if (disk_type != MEDIATYPE_UNKNOWN) {
        _media = new MediaTypeIMG();
        mt = _media->mount(f, disksize, disk_type);
        device_active = true;
        Debug_printf("disk MOUNT mediatype = %d: active\n", disk_type);
    } else {
        device_active = false;
        Debug_printf("disk MOUNT unknown: deactive\n");
    }

    return mt;
}

void ACSIDisk::unmount()
{
    Debug_print("disk UNMOUNT\n");

    if (_media != nullptr) {
        _media->unmount();
        device_active = false;
    }
}

void ACSIDisk::process(uint32_t commanddata, uint8_t checksum)
{
}

#endif /* NEW_TARGET */
