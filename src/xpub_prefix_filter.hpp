/*
    Copyright (c) 2011 250bpm s.r.o.
    Copyright (c) 2012 Spotify AB
    Copyright (c) 2011-2012 Other contributors as noted in the AUTHORS file

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

#ifndef __ZMQ_XPUB_PREFIX_FILTER_HPP_INCLUDED__
#define __ZMQ_XPUB_PREFIX_FILTER_HPP_INCLUDED__

#include <stddef.h>
#include <set>

#include "stdint.hpp"
#include "xpub_filter.hpp"

namespace zmq
{

    class pipe_t;

    //  Multi-trie. Each node in the trie is a set of pointers to pipes.

    class xpub_prefix_filter_t
        : public xpub_filter_t
    {
    public:

        xpub_prefix_filter_t ();
        virtual ~xpub_prefix_filter_t ();

        //  Add key to the trie. Returns true if it's a new subscription
        //  rather than a duplicate.
        virtual bool add_rule (
            const unsigned char *prefix_, size_t size_, pipe_t *pipe_);

        //  Remove all subscriptions for a specific peer from the trie.
        //  If there are no subscriptions left on some topics, invoke the
        //  supplied callback function.
        virtual void remove_pipe (
            pipe_t *pipe_, void (*func_) (
                const unsigned char *data_, size_t size_,
                uint16_t method_id_, void *arg_), void *arg_);

        //  Remove specific subscription from the trie. Return true is it was
        //  actually removed rather than de-duplicated.
        virtual bool remove_rule (
            const unsigned char *prefix_, size_t size_, pipe_t *pipe_);

        //  Signal all the matching pipes.
        virtual void match (const unsigned char *data_, size_t size_,
            void (*func_) (pipe_t *pipe_, void *arg_), void *arg_);

    private:

        bool add_helper (
            const unsigned char *prefix_, size_t size_, pipe_t *pipe_);

        void rm_helper (
            pipe_t *pipe_, unsigned char **buff_,
            size_t buffsize_, size_t maxbuffsize_,
            void (*func_) (
                const unsigned char *data_, size_t size_,
                uint16_t method_id_, void *arg_),
            void *arg_);

        bool rm_helper (
            const unsigned char *prefix_, size_t size_, pipe_t *pipe_);

        typedef std::set <zmq::pipe_t*> pipes_t;
        pipes_t pipes;

        unsigned char min;
        unsigned short count;
        union {
            class xpub_prefix_filter_t *node;
            class xpub_prefix_filter_t **table;
        } next;

        xpub_prefix_filter_t (const xpub_prefix_filter_t&);
        const xpub_prefix_filter_t &operator = (const xpub_prefix_filter_t&);
    };

}

#endif

