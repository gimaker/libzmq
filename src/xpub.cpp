/*
    Copyright (c) 2012 Spotify AB
    Copyright (c) 2010-2011 250bpm s.r.o.
    Copyright (c) 2011 VMware, Inc.
    Copyright (c) 2010-2011 Other contributors as noted in the AUTHORS file

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

#include <string.h>

#include "xpub.hpp"
#include "pipe.hpp"
#include "err.hpp"
#include "msg.hpp"
#include "xpub_filter.hpp"
#include "xpub_prefix_filter.hpp"
#include "xpub_exact_filter.hpp"

zmq::xpub_t::xpub_t (class ctx_t *parent_, uint32_t tid_) :
    socket_base_t (parent_, tid_),
    more (false)
{
    options.type = ZMQ_XPUB;
}

zmq::xpub_t::~xpub_t ()
{
    for (filters_t::iterator it = filters.begin ();
         it != filters.end (); ++it) {
        delete it->second;
    }
}

void zmq::xpub_t::xattach_pipe (pipe_t *pipe_)
{
    zmq_assert (pipe_);
    dist.attach (pipe_);

    //  The pipe is active when attached. Let's read the subscriptions from
    //  it, if any.
    xread_activated (pipe_);
}

void zmq::xpub_t::xread_activated (pipe_t *pipe_)
{
    //  There are some subscriptions waiting. Let's process them.
    msg_t sub;
    sub.init ();
    while (true) {

        //  Grab next subscription.
        if (!pipe_->read (&sub)) {
            sub.close ();
            return;
        }

        //  Update our filters with the subscription
        unsigned char *data = (unsigned char*) sub.data ();
        size_t size = sub.size ();
        bool unique;

        zmq_assert (size >= 4);
        uint16_t cmd_id = data[0] << 8 | data[1];
        zmq_assert (cmd_id == 0 || cmd_id == 1);
        uint16_t method_id = data[2] << 8 | data[3];

        xpub_filter_t *filter = 0;
        filters_t::iterator it = filters.find (method_id);
        if (it == filters.end ()) {
            filter = create_filter (method_id);
            zmq_assert (filter);
            filters.insert (std::make_pair (method_id, filter));
        }
        else  {
            filter = it->second;
        }

        if (cmd_id == 1)
            unique = filter->add_rule (data + 4, size - 4, pipe_);
        else // cmd_id == 0
            unique = filter->remove_rule (data + 4, size - 4, pipe_);

        //  If the subscription is not a duplicate store it so that it can be
        //  passed to used on next recv call.
        if (unique && options.type != ZMQ_PUB)
            pending.push_back (blob_t ((unsigned char*) sub.data (),
                sub.size ()));
    }
}

void zmq::xpub_t::xwrite_activated (pipe_t *pipe_)
{
    dist.activated (pipe_);
}

void zmq::xpub_t::xterminated (pipe_t *pipe_)
{
    //  Remove the pipe from the trie. If there are topics that nobody
    //  is interested in anymore, send corresponding unsubscriptions
    //  upstream.
    for (filters_t::iterator it = filters.begin ();
         it != filters.end (); ++it) {
        it->second->remove_pipe (pipe_, send_unsubscription, this);
    }

    dist.terminated (pipe_);
}

void zmq::xpub_t::mark_as_matching (pipe_t *pipe_, void *arg_)
{
    xpub_t *self = (xpub_t*) arg_;
    self->dist.match (pipe_);
}

zmq::xpub_filter_t *zmq::xpub_t::create_filter (uint16_t method_id)
{
    xpub_filter_t *ret = 0;
    switch (method_id) {
    case ZMQ_FILTER_PREFIX:
        ret = new (std::nothrow) xpub_prefix_filter_t ();
        break;

    case ZMQ_FILTER_EXACT:
        ret = new (std::nothrow) xpub_exact_filter_t ();
        break;

    default:
        ret = new (std::nothrow) xpub_default_filter_t (method_id);
    }

    zmq_assert (ret);
    return ret;
}

int zmq::xpub_t::xsend (msg_t *msg_, int flags_)
{
    bool msg_more = msg_->flags () & msg_t::more ? true : false;

    //  For the first part of multi-part message, find the matching pipes.
    if (!more) {
        for (filters_t::iterator it = filters.begin ();
             it != filters.end (); ++it) {
            it->second->match (
                (const unsigned char*) msg_->data (), msg_->size (),
                mark_as_matching, this);
        }
    }

    //  Send the message to all the pipes that were marked as matching
    //  in the previous step.
    int rc = dist.send_to_matching (msg_, flags_);
    if (rc != 0)
        return rc;

    //  If we are at the end of multi-part message we can mark all the pipes
    //  as non-matching.
    if (!msg_more)
        dist.unmatch ();

    more = msg_more;

    return 0;
}

bool zmq::xpub_t::xhas_out ()
{
    return dist.has_out ();
}

int zmq::xpub_t::xrecv (msg_t *msg_, int flags_)
{
    //  If there is at least one 
    if (pending.empty ()) {
        errno = EAGAIN;
        return -1;
    }

    int rc = msg_->close ();
    errno_assert (rc == 0);
    rc = msg_->init_size (pending.front ().size ());
    errno_assert (rc == 0);
    memcpy (msg_->data (), pending.front ().data (),
        pending.front ().size ());
    pending.pop_front ();
    return 0;
}

bool zmq::xpub_t::xhas_in ()
{
    return !pending.empty ();
}

void zmq::xpub_t::send_unsubscription (
    const unsigned char *data_, size_t size_, uint16_t method_id_, void *arg_)
{
    xpub_t *self = (xpub_t*) arg_;

    if (self->options.type != ZMQ_PUB) {
        //  Place the unsubscription to the queue of pending (un)sunscriptions
        //  to be retrived by the user later on.
        xpub_t *self = (xpub_t*) arg_;
        blob_t unsub (size_ + 4, 0);
        //  cmd_id: Unsubscribe
        unsub [0] = unsub [1] = 0;
        //  method_id, in network byte order
        unsub [2] = method_id_ >> 8;
        unsub [3] = method_id_ & 0xFF;
        memcpy (&unsub [4], data_, size_);
        self->pending.push_back (unsub);
    }
}

zmq::xpub_session_t::xpub_session_t (io_thread_t *io_thread_, bool connect_,
      socket_base_t *socket_, const options_t &options_,
      const char *protocol_, const char *address_) :
    session_base_t (io_thread_, connect_, socket_, options_, protocol_,
        address_)
{
}

zmq::xpub_session_t::~xpub_session_t ()
{
}

