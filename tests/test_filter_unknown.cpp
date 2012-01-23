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

#include <assert.h>
#include <stdio.h>

#include "../include/zmq.h"
#include "../include/zmq_utils.h"

int main (int argc, char *argv [])
{
    fprintf (stderr, "test_filter_unknown running...\n");

    void *ctx = zmq_init (1);
    assert (ctx);

    //  Create a subscriber.
    void *sub = zmq_socket (ctx, ZMQ_SUB);
    assert (sub);
    int rc = zmq_connect (sub, "tcp://127.0.0.1:5560");
    assert (rc == 0);

    // Subscribe to "a" using exact matching
    int filter = 0xdeadbeef;
    rc = zmq_setsockopt (sub, ZMQ_FILTER, &filter, sizeof (filter));
    assert (rc == -1);
    assert (errno == EINVAL);

    //  Clean up.
    rc = zmq_close (sub);
    assert (rc == 0);
    rc = zmq_term (ctx);
    assert (rc == 0);

    return 0 ;
}
