/*
    Copyright (c) 2012 Spotify AB
    Copyright (c) 2012 Other contributors as noted in the AUTHORS file

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

#ifndef __ZMQ_XPUB_FILTER_HPP_INCLUDED__
#define __ZMQ_XPUB_FILTER_HPP_INCLUDED__

#include <map>
#include <set>

#include "stdint.hpp"
#include "blob.hpp"

namespace zmq
{
    class pipe_t;

    class xpub_filter_t
    {
    public:
        virtual ~xpub_filter_t () {}

        virtual bool add_rule (
            const unsigned char *data_, size_t size_, pipe_t *pipe_) = 0;

        virtual bool remove_rule (
            const unsigned char *data_, size_t size_, pipe_t *pipe_) = 0;

        virtual void remove_pipe (
            pipe_t *pipe_,
            void (*func_) (const unsigned char *, size_t, uint16_t, void *),
            void *arg_) = 0;

        virtual void match (
            const unsigned char *data_, size_t size_,
            void (*func_) (pipe_t *pipe_, void *arg_),
            void *arg_) = 0;

        //  This might be nice to have, to be able to remove no longer used
        //  filters
        //virtual size_t size () const = 0;
    };

    class xpub_default_filter_t :
        public xpub_filter_t
    {
    public:
        xpub_default_filter_t (uint16_t method_id_);

        virtual bool add_rule (
            const unsigned char *data_, size_t size_, pipe_t *pipe_);

        virtual bool remove_rule (
            const unsigned char *data_, size_t size_, pipe_t *pipe_);

        virtual void remove_pipe (
            pipe_t *pipe_,
            void (*func_) (const unsigned char *, size_t, uint16_t, void *),
            void *arg_);

        virtual void match (
            const unsigned char *data_, size_t size_,
            void (*func_) (pipe_t *pipe_, void *arg_),
            void *arg_);

    private:
        const uint16_t method_id;

        typedef std::set<pipe_t *> pipes_t;
        typedef std::map<blob_t, pipes_t> rules_t;
        rules_t rules;

        // pipe -> number of rules for that pipe
        //
        // We need a set of all pipes to do the "matching" in a reasonably fast
        // way, and we use the refcounts to maintain that set.
        typedef std::map<pipe_t *, unsigned int> refcounts_t;
        refcounts_t refcounts;
    };

}

#endif
