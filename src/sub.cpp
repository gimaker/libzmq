/*
    Copyright (c) 2012 Spotify AB
    Copyright (c) 2009-2011 250bpm s.r.o.
    Copyright (c) 2007-2009 iMatix Corporation
    Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sub.hpp"
#include "msg.hpp"

zmq::sub_t::sub_t (class ctx_t *parent_, uint32_t tid_) :
    xsub_t (parent_, tid_),
    filter_method (ZMQ_FILTER_PREFIX)
{
    options.type = ZMQ_SUB;

    //  Switch filtering messages on (as opposed to XSUB which where the
    //  filtering is off).
    options.filter = true;
}

zmq::sub_t::~sub_t ()
{
}

int zmq::sub_t::xsetsockopt (int option_, const void *optval_,
    size_t optvallen_)
{
    if (option_ == ZMQ_FILTER) {
        if (optvallen_ != sizeof (int)) {
            errno = EINVAL;
            return -1;
        }
        int method = *((const int *)optval_);
        if (method != ZMQ_FILTER_PREFIX && method != ZMQ_FILTER_EXACT) {
            errno = EINVAL;
            return -1;
        }
        filter_method = method;
        return 0;
    }
    else if (option_ == ZMQ_SUBSCRIBE || option_ == ZMQ_UNSUBSCRIBE) {
        //  Create the subscription message.
        msg_t msg;
        int rc = msg.init_size (optvallen_ + 4);
        errno_assert (rc == 0);
        unsigned char *data = (unsigned char*) msg.data ();
        uint16_t cmd_id = option_ == ZMQ_SUBSCRIBE ? 1 : 0;
        data [0] = cmd_id >> 8;
        data [1] = cmd_id & 0xFF;
        data [2] = filter_method >> 8;
        data [3] = filter_method & 0xFF;
        memcpy (data + 4, optval_, optvallen_);

        //  Pass it further on in the stack.
        int err = 0;
        rc = xsub_t::xsend (&msg, 0);
        if (rc != 0)
            err = errno;
        int rc2 = msg.close ();
        errno_assert (rc2 == 0);
        if (rc != 0)
            errno = err;
        return rc;
    }
    else {
        errno = EINVAL;
        return -1;
    }
}

int zmq::sub_t::xsend (msg_t *msg_, int flags_)
{
    //  Overload the XSUB's send.
    errno = ENOTSUP;
    return -1;
}

bool zmq::sub_t::xhas_out ()
{
    //  Overload the XSUB's send.
    return false;
}

zmq::sub_session_t::sub_session_t (io_thread_t *io_thread_, bool connect_,
      socket_base_t *socket_, const options_t &options_,
      const char *protocol_, const char *address_) :
    xsub_session_t (io_thread_, connect_, socket_, options_, protocol_,
        address_)
{
}

zmq::sub_session_t::~sub_session_t ()
{
}

