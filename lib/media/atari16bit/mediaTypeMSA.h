#ifndef _MEDIATYPE_MSA_
#define _MEDIATYPE_MSA_

#include <string>

#include <utility>

#include "mediaType.h"

/**
 * ST media type. Atari ST Magic Shadow Archiver Floppy image.
 * No header just the disk data as follows side0track0, side1 track0, side0 track1 side1 track1 ...
*/




class MediaTypeMSA: public MediaType
{
private:
    uint32_t _sector_to_offset(uint16_t sectorNum);

public:
    struct CpmDiskImageDetails {
        std::string file_extension;
        uint32_t media_size;
        CPM_DPB dpb;
    };

public:
    virtual bool read(uint16_t sectornum, uint16_t *readcount) override;
    virtual bool write(uint16_t sectornum, bool verify) override;

    virtual bool format(uint16_t *respopnsesize) override;

    virtual mediatype_t mount(FILE *f, uint32_t disksize, mediatype_t disk_type) override;

    virtual void status(uint8_t statusbuff[4]) override;

    static bool create(FILE *f, uint16_t sectorSize, uint16_t numSectors);
};


#endif // _MEDIATYPE_MSA_