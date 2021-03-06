
#ifndef LIBMNFLASH_USB_H
#define LIBMNFLASH_USB_H 1

#include <stdint.h>
#include <usb.h>

#define MICRONUCLEUS_VENDOR_ID   0x16d0
#define MICRONUCLEUS_PRODUCT_ID  0x0753

typedef enum {
	DEV_MODE_NONE,
	DEV_MODE_NORMAL,
	DEV_MODE_PROGRAMMING,
	DEV_MODE_DISCONNECTED,
} mnflash_usb_mode_t;

typedef struct {
	usb_dev_handle*		handle;
	mnflash_usb_mode_t	mode;
	void *			app_data;
	union {
		uint16_t	intversion;
		uint8_t		minormajor[2];
	} version;
} mnflash_usb_t;

#define minor_version	version.minormajor[0]
#define major_version	version.minormajor[1]

extern ssize_t mnflash_usb_custom_read(
	mnflash_usb_t * dev,
	uint16_t request,
	uint16_t index,
	uint16_t value,
	void * buffer,
	size_t buflen
);

extern ssize_t mnflash_usb_custom_write(
	mnflash_usb_t * dev,
	uint16_t request,
	uint16_t index,
	uint16_t value,
	void * buffer,
	size_t buflen
);

extern ssize_t mnflash_usb_custom_write_once(
	mnflash_usb_t * dev,
	uint16_t request,
	uint16_t index,
	uint16_t value,
	void * buffer,
	size_t buflen
);

extern void mnflash_usb_show(mnflash_usb_t * dev);

typedef usb_dev_handle * (*mnflash_usb_filter)(struct usb_device * dev, void * arg);

extern mnflash_usb_t * mnflash_usb_connect(mnflash_usb_filter filter, void * arg);

extern void mnflash_usb_destroy(mnflash_usb_t * dev);

#endif
