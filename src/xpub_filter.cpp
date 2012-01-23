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

#include "xpub_filter.hpp"
#include "err.hpp"


zmq::xpub_default_filter_t::xpub_default_filter_t (uint16_t method_id_)
    : method_id(method_id_)
{
}


bool zmq::xpub_default_filter_t::add_rule (
    const unsigned char *data_, size_t size_, pipe_t *pipe_)
{
    ++refcounts [pipe_];
    return rules [rules_t::key_type (data_, size_)].insert (pipe_).second;
}


bool zmq::xpub_default_filter_t::remove_rule (
    const unsigned char *data_, size_t size_, pipe_t *pipe_)
{
    rules_t::iterator it = rules.find (rules_t::key_type (data_, size_));
    zmq_assert (it != rules.end ());
    pipes_t &pipes_ = it->second;

    pipes_t::iterator jt = pipes_.find (pipe_);
    zmq_assert (jt != pipes_.end ());
    pipes_.erase (jt);

    refcounts_t::iterator kt = refcounts.find (pipe_);
    zmq_assert (kt != refcounts.end ());
    if (--(kt->second) == 0) {
        refcounts.erase (kt);
    }

    if (pipes_.empty ()) {
        rules.erase (it);
        return true;
    }
    else {
        return false;
    }
}


void zmq::xpub_default_filter_t::remove_pipe (
    pipe_t *pipe_,
    void (*func_) (const unsigned char *, size_t, uint16_t, void *),
    void *arg_)
{
    for (rules_t::iterator it = rules.begin (); it != rules.end ();) {
        rules_t::iterator next_it = it;
        ++next_it;

        pipes_t &pipes_ = it->second;
        pipes_t::iterator jt = pipes_.find (pipe_);
        if (jt != pipes_.end ()) {
            const rules_t::key_type &rule = it->first;
            func_ (rule.data (), rule.size (), method_id, arg_);

            pipes_.erase (jt);
            if (pipes_.empty ()) {
                rules.erase(it);
            }
        }

        it = next_it;
    }

    refcounts.erase(pipe_);
}


void zmq::xpub_default_filter_t::match (
    const unsigned char *data_, size_t size_,
    void (*func_) (pipe_t *pipe_, void *arg_),
    void *arg_)
{
    for (refcounts_t::iterator it = refcounts.begin ();
         it != refcounts.end (); ++it) {
        func_ (it->first, arg_);
    }
}
