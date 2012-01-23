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

#ifndef __ZMQ_XSUB_PREFIX_FILTER_HPP_INCLUDED__
#define __ZMQ_XSUB_PREFIX_FILTER_HPP_INCLUDED__

#include <stddef.h>

#include "xsub_filter.hpp"

namespace zmq
{

    class xsub_prefix_filter_t :
        public xsub_filter_t
    {
    public:

        xsub_prefix_filter_t ();
        virtual ~xsub_prefix_filter_t ();

        //  Add key to the xsub_prefix_filter. Returns true if this is a new item in the xsub_prefix_filter
        //  rather than a duplicate.
        virtual bool add_rule (const unsigned char *prefix_, size_t size_);

        //  Remove key from the xsub_prefix_filter. Returns true if the item is actually
        //  removed from the xsub_prefix_filter.
        virtual bool remove_rule (const unsigned char *prefix_, size_t size_);

        //  Check whether particular key is in the xsub_prefix_filter.
        virtual bool match (const unsigned char *data_, size_t size_);

        //  Apply the function supplied to each subscription in the xsub_prefix_filter.
        virtual void apply (
            void (*func_) (
                const unsigned char *data_, size_t size_,
                uint16_t method_id_, void *arg_), void *arg_);

    private:

        void apply_helper (
            unsigned char **buff_, size_t buffsize_, size_t maxbuffsize_,
            void (*func_) (
                const unsigned char *data_, size_t size_,
                uint16_t method_id_, void *arg_),
            void *arg_);

        uint32_t refcnt;
        unsigned char min;
        unsigned short count;
        union {
            class xsub_prefix_filter_t *node;
            class xsub_prefix_filter_t **table;
        } next;

        xsub_prefix_filter_t (const xsub_prefix_filter_t&);
        const xsub_prefix_filter_t &operator = (const xsub_prefix_filter_t&);
    };

}

#endif

