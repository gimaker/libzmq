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

#ifndef __ZMQ_XSUB_FILTER_HPP_INCLUDED__
#define __ZMQ_XSUB_FILTER_HPP_INCLUDED__

#include <map>

#include "blob.hpp"
#include "stdint.hpp"

namespace zmq
{

    class xsub_filter_t
    {
    public:
        virtual ~xsub_filter_t () {}

        virtual bool add_rule (const unsigned char *data_, size_t size_) = 0;

        virtual bool remove_rule (const unsigned char *data_, size_t size_) = 0;

        virtual bool match (const unsigned char *data_, size_t size_) = 0;

        virtual void apply (
            void (*func_) (const unsigned char *data_, size_t size_,
                           uint16_t method_id_, void *arg_),
            void *arg_) = 0;
    };

    class xsub_default_filter_t :
        public xsub_filter_t
    {
    public:
        xsub_default_filter_t (uint16_t method_id_);

        virtual bool add_rule (const unsigned char *data_, size_t size_);

        virtual bool remove_rule (const unsigned char *data_, size_t size_);

        virtual bool match (const unsigned char *data_, size_t size_);

        virtual void apply (
            void (*func_) (const unsigned char *data_, size_t size_,
                           uint16_t method_id_, void *arg_),
            void *arg_);

    private:
        const uint16_t method_id;

        //  rule -> refcount
        typedef std::map <blob_t, unsigned int> rules_t;
        rules_t rules;
    };

}

#endif

