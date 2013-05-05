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

/*
 * Common constants, structures and functions for NBD clients and server.
 */

#ifndef LINUX_NBD_COMMON_H
#define LINUX_NBD_COMMON_H

#include <stdint.h>

/*
 * NEGOTIATION
 */

// magic numbers
#define	NBD_INIT_PASSWD "NBDMAGIC"
enum {
	NBD_MAGIC_CLISERV = 0x0000420281861253LLU,
	NBD_MAGIC_OPTS    = 0x49484156454F5054LLU,
	NBD_MAGIC_REP     = 0x0003E889045565A9LLU,
};

// Options that the client can select to the server
typedef enum nbd_opt_cmd {
	NBD_OPT_EXPORT_NAME = 1,	/* Client wants to select a named
					 * export, is followed by name of
					 * export
					 */
	NBD_OPT_ABORT       = 2,	// Client wishes to abort negotiation
	NBD_OPT_LIST        = 3,	/* Client request list of supported
					 * exports, not followed by data
					 */
	NBD_OPT_DO_NOT_USE_ = 1 << 31,	// force enum to be 32bit, DO NOT USE
} nbd_opt_cmd_t;

// Replies the server can send during negotiation
enum {
	NBD_REP_ACK        = 1,		/* ACK a request. Data: option number
				         * to be acked
				         */
	NBD_REP_SERVER     = 2,		/* Reply to NBD_OPT_LIST (one of these
					 * per server; must be followed by
					 * NBD_REP_ACK to signal the end of
					 * the list
					 */
	NBD_REP_FLAG_ERROR = 1 << 31,	/* If the high bit is set, the reply
					 * is an error
					 */
	NBD_REP_ERR_UNSUP = 1 | NBD_REP_FLAG_ERROR, /* Client requested an
						     * option not understood
						     * by this version of the
						     * server
						     */
	NBD_REP_ERR_POLICY = 2 | NBD_REP_FLAG_ERROR, /* Client requested an
						      * option not allowed by
						      * server configuration.
						      * (e.g., the option was
						      * disabled)
						      */
	NBD_REP_ERR_INVALID = 3 | NBD_REP_FLAG_ERROR, /* Client issued an
						       * invalid request
						       */
	NBD_REP_ERR_PLATFORM = 4 | NBD_REP_FLAG_ERROR, /* Option not supported
							* on this platform
							*/

// Global flags
	NBD_GFLAG_FIXED_NEWSTYLE = 1 << 0,	/* new-style export that
						 * actually supports extending
						 */
	
// Flags from client to server. Only one such option currently.
	NBD_CFLAG_FIXED_NEWSTYLE = NBD_GFLAG_FIXED_NEWSTYLE,

// per-export flags
	NBD_EBIT_HAS_FLAGS,	// Flags are there
	NBD_EBIT_READ_ONLY,	// Device is read-only
	NBD_EBIT_SEND_FLUSH,	// Send FLUSH
	NBD_EBIT_SEND_FUA,	// Send FUA (Force Unit Access)
	NBD_EBIT_ROTATIONAL,	// Use elevator algorithm - rotational media
	NBD_EBIT_SEND_TRIM,	// Send TRIM (discard)


	NBD_EFLAG_HAS_FLAGS  = 1 << NBD_EBIT_HAS_FLAGS,
	NBD_EFLAG_READ_ONLY  = 1 << NBD_EBIT_READ_ONLY,
	NBD_EFLAG_SEND_FLUSH = 1 << NBD_EBIT_SEND_FLUSH,
	NBD_EFLAG_SEND_FUA   = 1 << NBD_EBIT_SEND_FUA,
	NBD_EFLAG_ROTATIONAL = 1 << NBD_EBIT_ROTATIONAL,
	NBD_EFLAG_SEND_TRIM  = 1 << NBD_EBIT_SEND_TRIM,
};

/*
 * This is the packet used for requesting options from the server.
 * All data is stored in network byte order. Access through helpers only.
 */
typedef struct nbd_option {
	uint64_t      magic;
	nbd_opt_cmd_t cmd;
	uint32_t      len;
	const void   *data;
} __attribute__ ((packed)) nbd_option_t;

/*
 * REQUESTS AND REPLIES
 */

enum {
	NBD_MAGIC_REQUEST = 0x25609513,	// magic in nbd_request_t
	NBD_MAGIC_REPLY   = 0x67446698, // magic in nbd_reply_t
	// Do *not* use magics: 0x12560953 0x96744668.
};

// request commands
typedef enum {
	NBD_CMD_READ = 0,
	NBD_CMD_WRITE = 1,
	NBD_CMD_DISC = 2,
	NBD_CMD_FLUSH = 3,
	NBD_CMD_TRIM = 4,
	NBD_CMD_MASK_COMMAND = 0x0000ffff,
	NBD_CMD_FLAG_FUA = (1<<16),
} nbd_cmd_t;

/*
 * This is the packet used for communication between client and
 * server.
 */
typedef struct nbd_request {
	uint32_t  magic;
	nbd_cmd_t type;
	char      handle[8];
	uint64_t  from;
	uint32_t  len;
} __attribute__ ((packed)) nbd_request_t;

/*
 * This is the reply packet that nbd-server sends back to the client after
 * it has completed an I/O request (or an error occurs).
 */
typedef struct nbd_reply {
	uint32_t magic;
	uint32_t error;		// 0 = ok, else error
	char     handle[8];	// handle you got from request
} __attribute__ ((packed)) nbd_reply_t;

/*
 * Convert between host and network byte order.
 *
 * @param a  64bit integer to convert
 * @retrurn converted 64bit integer
 */
uint64_t ntohll(uint64_t a);
uint64_t htonll(uint64_t a);

#endif // #ifndef LINUX_NBD_COMMON_H
