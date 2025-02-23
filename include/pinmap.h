/* FujiNet Hardware Pin Mapping */
#ifndef PINMAP_H
#define PINMAP_H

#ifdef ESP_PLATFORM
#include <hal/gpio_types.h>

#include "pinmap/a2_fn10.h"
#include "pinmap/a2_rev0.h"
#include "pinmap/a2_d32pro.h"
#include "pinmap/mac_rev0.h"
#include "pinmap/atariv1.h"
#include "pinmap/adamv1.h"
#include "pinmap/iec.h"
#include "pinmap/iec-d32pro.h"
#include "pinmap/coco_devkitc.h"
#include "pinmap/coco_cart.h"
#include "pinmap/coco_esp32s3.h"
#include "pinmap/foenix_os9_d32pro.h"
#include "pinmap/iec-nugget.h"
#include "pinmap/fujiloaf-rev0.h"
#include "pinmap/fujiapple-iec.h"
#include "pinmap/esp32s3.h"
#include "pinmap/esp32s3-wroom-1.h"
#include "pinmap/lynx.h"
#include "pinmap/rs232_rev0.h"
#include "pinmap/cx16.h"
#include "pinmap/rc2014spi_rev0.h"
#include "pinmap/heathkit_h89.h"
#include "pinmap/atari2600.h"


/* LED Strip NEW */
#define RGB_LED_DATA_PIN        PIN_LED_RGB
#define RGB_LED_BRIGHTNESS      15 // max mA the LED can use determines brightness
#define RGB_LED_COUNT           5
#define RGB_LED_TYPE            WS2812B
#define RGB_LED_ORDER           GRB

#ifndef PIN_DEBUG
#define PIN_DEBUG		PIN_IEC_SRQ
#endif

#include "pinmap/atari16bit_rev0.h"
#endif // ESP_PLATFORM
#endif /* PINMAP_H */
