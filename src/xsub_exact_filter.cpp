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

#include "err.hpp"
#include "xsub_exact_filter.hpp"

zmq::xsub_exact_filter_t::xsub_exact_filter_t ()
{
}


zmq::xsub_exact_filter_t::~xsub_exact_filter_t ()
{
}


bool zmq::xsub_exact_filter_t::add_rule (
    const unsigned char *data_, size_t size_)
{
    return rules [blob_t (data_, size_)]++ == 0;
}


bool zmq::xsub_exact_filter_t::remove_rule (
    const unsigned char *data_, size_t size_)
{
    rules_t::iterator it = rules.find (blob_t (data_, size_));
    zmq_assert (it != rules.end ());
    if (--(it->second) == 0) {
        rules.erase (it);
        return true;
    }
    else {
        return false;
    }
}


bool zmq::xsub_exact_filter_t::match (
    const unsigned char *data_, size_t size_)
{
    rules_t::iterator it = rules.find (blob_t (data_, size_));
    return it != rules.end ();
}


void zmq::xsub_exact_filter_t::apply (
    void (*func_) (
        const unsigned char *data_, size_t size_,
        uint16_t method_id_, void *arg_),
    void *arg_)
{
    for (rules_t::iterator it = rules.begin (); it != rules.end (); ++it) {
        func_ (it->first.data (), it->first.size (), ZMQ_FILTER_EXACT, arg_);
    }
}
