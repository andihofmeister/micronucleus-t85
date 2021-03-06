/*
 *   Created: May 2013
 *   by Andreas Hofmeister <andi@collax.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy of
 *   this software and associated documentation files (the "Software"), to deal in
 *   the Software without restriction, including without limitation the rights to
 *   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *   of the Software, and to permit persons to whom the Software is furnished to do
 *   so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

#include <stdio.h>
#include <memory.h>
#include <libmnflash/usb-device.h>
#include <libmnflash/uploader.h>
#include <libmnflash/firmware.h>
#include <libmnflash/hexdump.h>
#include <libmnflash/log.h>

#define DEV_BUF_LEN	8

static mnflash_device_info_t * mnflash_device_info_new(mnflash_usb_t * dev)
{
	mnflash_device_info_t * result = NULL;

	if ( (result = malloc(sizeof(*result))) == NULL )
		return NULL;

	memset(result,0,sizeof(*result));

	result->dev = dev;

	return result;
}

void mnflash_device_info_destroy(mnflash_device_info_t * info)
{
	free(info);
}

mnflash_device_info_t * mnflash_get_device_info(mnflash_usb_t * dev)
{
	uint8_t buffer[DEV_BUF_LEN];
	ssize_t readlen = 0;
	mnflash_device_info_t * result = NULL;

	if ( dev->mode != DEV_MODE_PROGRAMMING ) {
		mnflash_error( "device not in bootloader");
		return NULL;
	}

	readlen = mnflash_usb_custom_read(dev, BOOTLOADER_INFO, 0, 0, buffer, DEV_BUF_LEN);

	if ( readlen < 4 ) {
		mnflash_error( "cannot read bootloader info from device");
		if ( readlen < 0 ) {
			mnflash_error( "  %s", usb_strerror());
		}
		return NULL;
	}

	if ((result = mnflash_device_info_new(dev)) == NULL) {
		mnflash_error( "cannot initialize uploader struct");
		return NULL;
	}

	/* micronucleus progmem size is big endian */
	result->progmem_size = buffer[1] + (buffer[0] << 8);
	result->page_size    = buffer[2];
	result->write_sleep  = buffer[3];
	if ( readlen > 4 ) {
		result->device = buffer[4];
		result->wiring = buffer[5];
	} else {
		result->device = 255;
		result->wiring = 255;
	}

	return result;
}

void mnflash_info(mnflash_device_info_t * info)
{
	mnflash_msg( "           device info: %d/%d (%s)",
			info->device, info->wiring, mnflash_firmware_get_target_name(info));
	mnflash_msg( "available program size: %d bytes", info->progmem_size);
	mnflash_msg( "      device page size: %d bytes", info->page_size);
	mnflash_msg( "           write delay: %d ms",    info->write_sleep);
}

static int mnflash_erase(mnflash_device_info_t * info)
{
	mnflash_msg("erasing device ...");
	usleep( 2000 * info->write_sleep );
	if (mnflash_usb_custom_write_once(info->dev, BOOTLOADER_ERASE, 0, 0, NULL, 0) < 0) {
		// mnflash_error("cannot erase device%s", usb_strerror());
		// return 0;
	}

	mnflash_error( "sleeping for %dµs", (info->write_sleep * ((info->progmem_size/info->page_size) + 1) * 1000));
	usleep(info->write_sleep * ((info->progmem_size/info->page_size) + 1) * 1000);

	return 1;
}

static int mnflash_execute(mnflash_device_info_t * info)
{
	ssize_t written = 0;

	mnflash_msg("run application ...");
	usleep( 2000 * info->write_sleep );
	if ((written = mnflash_usb_custom_write_once(info->dev, BOOTLOADER_EXECUTE, 0, 0, NULL, 0)) < 0) {
		mnflash_error("cannot execute application: %zd", written);
		return 0;
	}
	return 1;
}

int mnflash_upload(mnflash_device_info_t * info, mnflash_firmware_t  * firmware)
{
	uint16_t page_now = 0;
	uint8_t	* page_buffer;

	/*
	 * We cannot read-modify-write pages on the device because the
	 * bootloader does not let us read a page. So for now, the firmware
	 * must start on a page boundry.
	 *
	 * Not much of a problem since usually the firmware starts at 0
	 */
	if ( (firmware->start % info->page_size) != 0 ) {
		mnflash_error( "Firmware does not start at a page boundry (page size is %d)", info->page_size);
		return 0;
	}

	if ( firmware->end >= info->progmem_size ) {
		mnflash_error( "Firmware image is too big");
		return 0;
	}

	if ( (page_buffer = malloc(info->page_size)) == NULL ) {
		mnflash_error( "Cannot allocate page buffer");
		return 0;
	}

	if ( ! mnflash_erase(info) ) {
		mnflash_error( "Failed to erase device flash memory");
		return 0;
	}

	for (page_now = firmware->start; page_now < firmware->end; page_now += info->page_size) {
		size_t page_bytes = 0;
		ssize_t writelen = 0;

		if (page_now + info->page_size < firmware->end) {
			page_bytes = info->page_size;
		} else {
			page_bytes = firmware->end - page_now;
		}

		memset(page_buffer, 0xFF, info->page_size);
		memcpy(page_buffer, firmware->data + page_now, page_bytes);

		mnflash_hexdump(page_now, page_buffer, page_bytes);

		usleep( 1000 * info->write_sleep );

		writelen = mnflash_usb_custom_write(
				info->dev,
				BOOTLOADER_WRITE_PAGE,
				page_now,
				info->page_size,
				page_buffer,
				info->page_size);


		if ( writelen != info->page_size ) {
			/*
			 * Our last write was not successfull, so what now ?
			 */
			mnflash_error( "bad write: addr=0x%x, wanted to write %zd bytes but wrote %zd",
					page_now, page_bytes, writelen);

			if ( writelen < 0 ) {
				mnflash_error( "  usb error: %s", usb_strerror() );
			}
		}
	}

	if ( (page_now / info->page_size) < (info->progmem_size / info->page_size) ) {

		ssize_t writelen = 0;

		page_now = (info->progmem_size / info->page_size) * info->page_size;
		mnflash_msg("writing extra page at 0x%x", page_now);

		memset(page_buffer, 0xFF, info->page_size);

		usleep( 2000 * info->write_sleep );

		writelen = mnflash_usb_custom_write(
				info->dev,
				BOOTLOADER_WRITE_PAGE,
				page_now,
				info->page_size,
				page_buffer,
				info->page_size);

		if ( writelen != info->page_size ) {
			mnflash_error( "could not write tiny table page, expected %d but wrote %zd",
					info->page_size, writelen);
			if ( writelen < 0 ) {
				mnflash_error( "  usb error: %s", usb_strerror() );
			}
			return 0;
		}
	}

	mnflash_execute(info);

	return 1;
}


