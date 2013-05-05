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
 * Constants, structures and functions for NBD clients.
 */

#ifndef LINUX_NBD_CLIENT_H
#define LINUX_NBD_CLIENT_H

#include <sys/ioctl.h>

#include <nbd-common.h>

#define NBD_SET_SOCK		_IO( 0xab, 0 )
#define NBD_SET_BLKSIZE		_IO( 0xab, 1 )
#define NBD_SET_SIZE		_IO( 0xab, 2 )
#define NBD_DO_IT		_IO( 0xab, 3 )
#define NBD_CLEAR_SOCK		_IO( 0xab, 4 )
#define NBD_CLEAR_QUE		_IO( 0xab, 5 )
#define NBD_PRINT_DEBUG		_IO( 0xab, 6 )
#define NBD_SET_SIZE_BLOCKS	_IO( 0xab, 7 )
#define NBD_DISCONNECT  	_IO( 0xab, 8 )
#define NBD_SET_TIMEOUT 	_IO( 0xab, 9 )
#define NBD_SET_FLAGS 		_IO( 0xab, 10 )

/*
 * Initialize a nbd_option_t. Arguments are converted to network byte order
 * except for the data part.
 *
 * @param  opt   option request
 * @param  cmd   option type
 * @param  len   length of data
 * @param  data  pointer to <len> bytes of data (or NULL if len == 0)
 * @return       opt will be initialized in network byte order
 */
void nbd_option_init(nbd_option_t *opt, nbd_opt_cmd_t cmd, uint32_t len,
		     const void *data);

/*
 * Initialize a nbd_option_t to select an export name.
 *
 * @param  opt   option request
 * @param  name  name of export
 * @return       opt will be initialized in network byte order
 */
void nbd_option_init_export_name(nbd_option_t *opt, const char *name);

/*
 * Initialize a nbd_option_t to abort.
 *
 * @param  opt   option request
 * @return       opt will be initialized in network byte order
 */
void nbd_option_init_abort(nbd_option_t *opt);

/*
 * Initialize a nbd_option_t to list exports.
 *
 * @param  opt   option request
 * @return       opt will be initialized in network byte order
 */
void nbd_option_init_list(nbd_option_t *opt);

/*
 * Write option request to file descriptor.
 *
 * @param fd   file descriptor to write to
 * @param opt  option request to write
 * @return 0 - success, -1 - error
 */
int nbd_sync_write_option(int fd, const nbd_option_t *opt);

#endif // #ifndef LINUX_NBD_CLIENT_H
