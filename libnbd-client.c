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

#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "nbd-client.h"

/*
 * Functions for NBD clients.
 */

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
		     const void *data) {
	opt->magic = htonll(NBD_MAGIC_OPTS);
	opt->cmd   = htonl(cmd);
	opt->len   = htonl(len);
	opt->data  = data;
}

/*
 * Initialize a nbd_option_t to select an export name.
 *
 * @param  opt   option request
 * @param  name  name of export
 * @return       opt will be initialized in network byte order
 */
void nbd_option_init_export_name(nbd_option_t *opt, const char *name) {
	nbd_option_init(opt, NBD_OPT_EXPORT_NAME, strlen(name), name);
}

/*
 * Initialize a nbd_option_t to abort.
 *
 * @param  opt   option request
 * @return       opt will be initialized in network byte order
 */
void nbd_option_init_abort(nbd_option_t *opt) {
	nbd_option_init(opt, NBD_OPT_ABORT, 0, NULL);
}

/*
 * Initialize a nbd_option_t to list exports.
 *
 * @param  opt   option request
 * @return       opt will be initialized in network byte order
 */
void nbd_option_init_list(nbd_option_t *opt) {
	nbd_option_init(opt, NBD_OPT_LIST, 0, NULL);
}

/*
 * Write option request to file descriptor.
 *
 * @param fd   file descriptor to write to
 * @param opt  option request to write
 * @return 0 - success, -1 - error
 */
int nbd_sync_write_option(int fd, const nbd_option_t *opt) {
	// FIXME: use mywrite()
	if (write(fd, &opt->magic, sizeof(opt->magic)) < 0) {
		return -1;
	}
	if (write(fd, &opt->cmd, sizeof(opt->cmd)) < 0) {
		return -1;
	}
	if (write(fd, &opt->len, sizeof(opt->len)) < 0) {
		return -1;
	}
	if (opt->len > 0) {
		if (write(fd, opt->data, opt->len) < 0) {
			return -1;
		}
	}
}
