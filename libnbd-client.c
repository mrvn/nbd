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
 * Initialize a nbd_opt_request_t. Arguments are converted to network byte order
 * except for the data part.
 *
 * @param  req   option request
 * @param  cmd   option type
 * @param  len   length of data
 * @param  data  pointer to <len> bytes of data (or NULL if len == 0)
 * @return       req will be initialized in network byte order
 */
void nbd_opt_request_init(nbd_opt_request_t *req, nbd_opt_cmd_t cmd, uint32_t len,
		     const void *data) {
	req->magic = htonll(NBD_MAGIC_OPTS);
	req->cmd   = htonl(cmd);
	req->len   = htonl(len);
	req->data  = data;
}

/*
 * Initialize a nbd_opt_request_t to select an export name.
 *
 * @param  req   option request
 * @param  name  name of export
 * @return       req will be initialized in network byte order
 */
void nbd_opt_request_init_export_name(nbd_opt_request_t *req, const char *name) {
	nbd_opt_request_init(req, NBD_OPT_EXPORT_NAME, strlen(name), name);
}

/*
 * Initialize a nbd_opt_request_t to abort.
 *
 * @param  req   option request
 * @return       opt will be initialized in network byte order
 */
void nbd_opt_request_init_abort(nbd_opt_request_t *req) {
	nbd_opt_request_init(req, NBD_OPT_ABORT, 0, NULL);
}

/*
 * Initialize a nbd_opt_request_t to list exports.
 *
 * @param  req   option request
 * @return       opt will be initialized in network byte order
 */
void nbd_opt_request_init_list(nbd_opt_request_t *req) {
	nbd_opt_request_init(req, NBD_OPT_LIST, 0, NULL);
}

/*
 * Write option request to file descriptor.
 *
 * @param fd   file descriptor to write to
 * @param req  option request to write
 * @return 0 - success, -1 - error
 */
int nbd_sync_write_opt_request(int fd, const nbd_opt_request_t *req) {
	// FIXME: use mywrite()
	if (write(fd, &req->magic, sizeof(req->magic)) < 0) {
		return NBD_EWRITE_MAGIC;
	}
	if (write(fd, &req->cmd, sizeof(req->cmd)) < 0) {
		return NBD_EWRITE_CMD;
	}
	if (write(fd, &req->len, sizeof(req->len)) < 0) {
		return NBD_EWRITE_LEN;
	}
	if (req->len > 0) {
		if (write(fd, req->data, req->len) < 0) {
			return NBD_EWRITE_DATA;
		}
	}
	return 0;
}

/*
 * Read option reply from file descriptor.
 *
 * @param fd   file descriptor to read from
 * @param res  option reply to fill
 * @param buf  buffer for extra data for reply
 * @param len  length of buf
 * @return 0 on success, error number otherwise,
 *         res will be filled in host byte order
 */
int nbd_sync_read_opt_reply(int fd, nbd_opt_reply_t *res, void *buf,
			    size_t len) {
	// FIXME: use myread()
	if(read(fd, &res->magic, sizeof(res->magic)) < 0) {
		return NBD_EREAD_MAGIC; // err("Reading magic from server: %m");
	}
	res->magic = ntohll(res->magic);
	if (res->magic != NBD_MAGIC_REP) {
		return NBD_EMAGIC; // err("Not enough magic from server");
	}
	if(read(fd, &res->cmd, sizeof(res->cmd)) < 0) {
		return NBD_EREAD_CMD; // err("Reading option: %m");
	}
	res->cmd = ntohl(res->cmd);
	if(read(fd, &res->result, sizeof(res->result)) <0) {
		return NBD_EREAD_RES; // err("Reading reply from server: %m");
	}
	res->result = ntohl(res->result);
	if(read(fd, &res->len, sizeof(res->len)) < 0) {
		return NBD_EREAD_LEN; // err("Reading length from server: %m");
	}
	res->len = ntohl(res->len);
	if (res->len > len) {
		return NBD_ETOOBIG; // too much data for buffer
	}
	if (res->len > 0) {
		res->data = buf;
		if(read(fd, buf, res->len) < 0) {
			return NBD_EREAD_DATA; // err("Reading data from server: %m");
		}
	} else {
		res->data = NULL;
	}
	return 0;
}
