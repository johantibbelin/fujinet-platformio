#ifndef DISK_H
#define DISK_H

#include "bus.h"
#include "media.h"

#define STATUS_OK        0
#define STATUS_BAD_BLOCK 1
#define STATUS_NO_BLOCK  2
#define STATUS_NO_MEDIA  3
#define STATUS_NO_DRIVE  4

class ACSIDisk : public virtualDevice
{
private:
    MediaType *_media = nullptr;
    void process(uint32_t commanddata, uint8_t checksum) override;

public:
    ACSIDisk();

    mediatype_t mount(FILE *f, const char *filename, uint32_t disksize, mediatype_t disk_type = MEDIATYPE_UNKNOWN);
    void unmount();

    bool device_active = false;

    ~ACSIDisk();
};

#endif /* ACSI_DISK_H */
