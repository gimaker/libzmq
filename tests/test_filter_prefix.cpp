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
#include <string.h>

#include "../include/zmq.h"
#include "../include/zmq_utils.h"

int main (int argc, char *argv [])
{
    fprintf (stderr, "test_prefix_filter running...\n");

    void *ctx = zmq_init (1);
    assert (ctx);

    //  Create a subscriber.
    void *sub = zmq_socket (ctx, ZMQ_SUB);
    assert (sub);
    int rc = zmq_connect (sub, "tcp://127.0.0.1:5560");
    assert (rc == 0);

    //  Create a publisher.
    void *pub = zmq_socket (ctx, ZMQ_PUB);
    assert (pub);
    rc = zmq_bind (pub, "tcp://*:5560");
    assert (rc == 0);

    rc = zmq_setsockopt (sub, ZMQ_SUBSCRIBE, "a", 1);
    assert (rc == 0);

    //  Wait a bit till the subscription gets to the publisher.
    zmq_sleep (1);

    char sndbuff [32], rcvbuff [32];

    //  Send a matching message
    sprintf (sndbuff, "a");
    rc = zmq_send (pub, sndbuff, 1, 0);
    assert (rc == 1);

    rc = zmq_recv (sub, rcvbuff, sizeof (rcvbuff), 0);
    assert (rc == 1);
    assert (memcmp (sndbuff, rcvbuff, 1) == 0);

    //  2. Send a matching message
    sprintf (sndbuff, "abc");
    rc = zmq_send (pub, sndbuff, 3, 0);
    assert (rc == 3);

    rc = zmq_recv (sub, rcvbuff, sizeof (rcvbuff), 0);
    assert (rc == 3);
    assert (memcmp (sndbuff, rcvbuff, 3) == 0);

    //  3. Send a non-matching message
    sprintf (sndbuff, "def");
    rc = zmq_send (pub, sndbuff, 3, 0);
    assert (rc == 3);

    //  Wait for a while to make sure our non-matching message has
    //  received at the SUB socket
    zmq_sleep (1);

    rc = zmq_recv (sub, rcvbuff, sizeof (rcvbuff), ZMQ_DONTWAIT);
    assert (rc == -1);
    assert (errno == EAGAIN);

    //  Clean up.
    rc = zmq_close (pub);
    assert (rc == 0);
    rc = zmq_close (sub);
    assert (rc == 0);
    rc = zmq_term (ctx);
    assert (rc == 0);

    return 0 ;
}
