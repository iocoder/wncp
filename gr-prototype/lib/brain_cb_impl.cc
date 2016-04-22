/* -*- c++ -*- */
/*
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "brain_cb_impl.h"

#define MIN(a,b) (a<b?a:b)

/******************************************************************************/
/*                         Sender State Machine                               */
/******************************************************************************/

#define SENDER_IDLE     0
#define SENDER_SYNC     1
#define SENDER_ZERO     2
#define SENDER_LEN      3
#define SENDER_DATA     4

typedef struct msg_to_send {
    struct msg_to_send *next;
    double time;
    int len;
    char *msg;
} msg_to_send_t;

msg_to_send_t *send_buf_head = NULL;
msg_to_send_t *send_buf_tail = NULL;

int sender_state = SENDER_IDLE;
int sender_step  = 0;

void sched(double time, const char *msg) {
    msg_to_send_t *msg_to_send = (msg_to_send_t*) malloc(sizeof(msg_to_send_t));
    msg_to_send->time = time;
    msg_to_send->msg = (char *) malloc((msg_to_send->len = strlen(msg))+1);
    strcpy(msg_to_send->msg, msg);
    msg_to_send->next = NULL;
    if (send_buf_head == NULL) {
        /* empty list */
        send_buf_head = send_buf_tail = msg_to_send;
    } else {
        send_buf_tail->next = msg_to_send;
        send_buf_tail = msg_to_send;
    }
}

int send(char *buf) {
    switch(sender_state) {
        case SENDER_IDLE:
            /* idle time */
            if (send_buf_head) {
                /* something to send */
                sender_state++;
            } else {
                /* nothing to send */
                return 0;
            }
        case SENDER_SYNC:
            /* sync pattern */
            *buf = 0x55;
            if (++sender_step == 6) {
                /* 6*8=48 sync bits sent */
                sender_state++;
                sender_step = 0;
            }
            break;
        case SENDER_ZERO:
            *buf = 0x00;
            if (++sender_step == 2) {
                /* 2*8=16 zero bits sent */
                sender_state++;
                sender_step = 0;
            }
            break;
        case SENDER_LEN:
            /* send length */
            *buf = send_buf_head->len;
            sender_state++;
            break;
        case SENDER_DATA:
            /* send data */
            *buf = send_buf_head->msg[sender_step];
            if (++sender_step == send_buf_head->len) {
                /* done */
                sender_state = SENDER_IDLE;
                sender_step = 0;
            }
            break;
        default:
            break;
    }
    return 1;
}

/******************************************************************************/
/*                        Receiver State Machine                              */
/******************************************************************************/

#define RECV_SYNC     1
#define RECV_ZERO     2
#define RECV_LEN      3
#define RECV_DATA     4

int recv_state = RECV_SYNC;
int recv_step  = 0;
int recv_len   = 0;
char *recv_msg = NULL;

int recv(char bit) {
    switch(recv_state) {
        case RECV_SYNC:
            if (bit == (recv_step&1)) {
                /* encountered expected bit */
                if (++recv_step == 6*8) {
                    /* synced 6*8=48 bits */
                    recv_state++;
                    recv_step = 0;
                }
            } else {
                if (bit == 1) {
                    /* .....01011 */
                    recv_step = 0;
                } else {
                    /* .....10100 */
                    recv_step = 1;
                }
            }
            break;
        case RECV_ZERO:
            if (!bit) {
                if (++recv_step == 16) {
                    /* received 16 zero bits */
                    recv_state++;
                    recv_step = 0;
                    recv_len = 0;
                }
            } else {
                if (recv_step == 1) {
                    /* 010101|01 */
                    recv_step = 0;
                } else {
                    /* ...000001 */
                    recv_state = RECV_SYNC;
                    recv_step = 2;
                }
            }
            break;
        case RECV_LEN:
            recv_len = (recv_len<<1) | bit;
            if (++recv_step == 8) {
                printf("Received (%d): ", recv_len);
                recv_state++;
                recv_step = 0;
                recv_msg = (char *) malloc(recv_len);
            }
            break;
        case RECV_DATA:
            recv_msg[recv_step/8] = (recv_msg[recv_step/8]<<1)|bit;
            if (recv_step%8 == 7) {
                printf("%c", recv_msg[recv_step/8]);
            }
            if (++recv_step == recv_len*8) {
                /* done */
                printf("\n");
                recv_state = RECV_SYNC;
                recv_step = 0;
            }
            break;
        default:
            break;
    }
}

/******************************************************************************/
/*                            GNU Radio Block                                 */
/******************************************************************************/

namespace gr {
  namespace prototype {

    brain_cb::sptr
    brain_cb::make()
    {
        return gnuradio::get_initial_sptr(new brain_cb_impl());
    }

    /*
     * The private constructor
     */
    brain_cb_impl::brain_cb_impl()
      : gr::block("brain_cb",
              gr::io_signature::make(0, 1, sizeof(char)), /* input */
              gr::io_signature::make(1, 1, sizeof(char))) /* output */
    {
        sched(0.1, "This message is carried over radio waves!");
    }

    /*
     * Our virtual destructor.
     */
    brain_cb_impl::~brain_cb_impl()
    {
    }

    void
    brain_cb_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
        unsigned ninputs = ninput_items_required.size();
        for(unsigned i = 0; i < ninputs; i++)
            ninput_items_required[i] = noutput_items;
    }

    int
    brain_cb_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        int _ninput_items = ninput_items[0];
        int _noutput_items = noutput_items;
        const char *in = (const char *) input_items[0];
        char *out = (char *) output_items[0];
        int i;

        // consume input
        for (i = 0; i < _ninput_items; i++) {
            recv(in[i]);
        }

        // tell runtime system how many input items we consumed
        consume_each(_ninput_items);

        // output something
        for (i = 0; i < _noutput_items; i++) {
            if (!send(&out[i])) {
                /* nothing to send */
                break;
            }
        }

        // tell runtime system how many output items we produced.
        return i;
    }

  } /* namespace prototype */
} /* namespace gr */

