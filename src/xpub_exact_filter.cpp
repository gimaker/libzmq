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

#include "xpub_exact_filter.hpp"
#include "err.hpp"

zmq::xpub_exact_filter_t::xpub_exact_filter_t ()
{
}


zmq::xpub_exact_filter_t::~xpub_exact_filter_t ()
{
}


bool zmq::xpub_exact_filter_t::add_rule (
    const unsigned char *data_, size_t size_, pipe_t *pipe_)
{
    return rules [blob_t (data_, size_)].insert (pipe_).second;
}


bool zmq::xpub_exact_filter_t::remove_rule (
    const unsigned char *data_, size_t size_, pipe_t *pipe_)
{
    rules_t::iterator it = rules.find (blob_t (data_, size_));
    zmq_assert (it != rules.end ());
    bool ret = it->second.erase (pipe_);
    if (it->second.empty ())
        rules.erase (it);
    return ret;
}


void zmq::xpub_exact_filter_t::remove_pipe (
    pipe_t *pipe_,
    void (*func_) (const unsigned char *, size_t, uint16_t, void *),
    void *arg_)
{
    for (rules_t::iterator it = rules.begin (), next_it; it != rules.end ();) {
        next_it = it;
        ++next_it;
        if (it->second.erase (pipe_)) {
            func_ (
                it->first.data (), it->first.size (), ZMQ_FILTER_EXACT, arg_);

            if (it->second.empty ())
                rules.erase (it);
        }

        it = next_it;
    }
}


void zmq::xpub_exact_filter_t::match (
    const unsigned char *data_, size_t size_,
    void (*func_) (pipe_t *pipe_, void *arg_),
    void *arg_)
{
    rules_t::iterator it = rules.find (blob_t (data_, size_));
    if (it != rules.end ()) {
        for (pipes_t::iterator jt  = it->second.begin ();
             jt != it->second.end (); ++jt) {
            func_ (*jt, arg_);
        }
    }
}

