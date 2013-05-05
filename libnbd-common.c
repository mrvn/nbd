/* This file is part of the NBD Library.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * 1999 Copyright (C) Pavel Machek, pavel@ucw.cz. This code is GPL.
 * 1999/11/04 Copyright (C) 1999 VMware, Inc. (Regis "HPReg" Duchesne)
 *            Made nbd_end_request() use the io_request_lock
 * 2001 Copyright (C) Steven Whitehouse
 *            New nbd_end_request() for compatibility with new linux block
 *            layer code.
 * 2003/06/24 Louis D. Langholtz <ldl@aros.net>
 *            Removed unneeded blksize_bits field from nbd_device struct.
 *            Cleanup PARANOIA usage & code.
 * 2004/02/19 Paul Clements
 *            Removed PARANOIA, plus various cleanup and comments
 * 2013/05/04 Goswin von Brederlow
 *            Create libnbd-common, libnbd-client, libnbd-server.
 */

#include <arpa/inet.h>

#include "nbd-common.h"
#include "config.h"

/*
 * Convert between host and network byte order.
 *
 * @param a  64bit integer to convert
 * @retrurn converted 64bit integer
 */
#ifdef WORDS_BIGENDIAN
uint64_t ntohll(uint64_t a) {
	return a;
}
#else
uint64_t ntohll(uint64_t a) {
	uint32_t lo = a & 0xffffffff;
	uint32_t hi = a >> 32U;
	lo = ntohl(lo);
	hi = ntohl(hi);
	return ((uint64_t) lo) << 32U | hi;
}
#endif

uint64_t htonll(uint64_t a) {
    return ntohll(a);
}

const char * err_msgs[] = {
    "success",				/* NBD_SUCCESS */
    "magic did not match",		/* NBD_EMAGIC */
    "data larger than buffer",		/* NBD_ETOOBIG */
    "could not read magic",		/* NBD_EREAD_MAGIC */
    "could not read option cmd",	/* NBD_EREAD_CMD */
    "could not read length",		/* NBD_EREAD_LEN */
    "could not read data",		/* NBD_EREAD_DATA */
    "could not read option result",	/* NBD_EREAD_RES */
    "could not write magic",		/* NBD_EWRITE_MAGIC */
    "could not write option cmd",	/* NBD_EWRITE_CMD */
    "could not write length",		/* NBD_EWRITE_LEN */
    "could not write data",		/* NBD_EWRITE_DATA */
    "could not write option result",	/* NBD_EWRITE_RES */
};

/*
 * Return string describing error number
 *
 * @param errnum  error number
 * @return string describing error number
 */
const char *nbd_strerror(nbd_err_t errnum) {
    if (errnum >= sizeof(err_msgs) / sizeof(err_msgs[0])) {
	return "Unknown error";
    }
    return err_msgs[errnum];
}
