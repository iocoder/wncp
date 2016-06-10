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
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "brain_cb_impl.h"

#define MIN(a,b) (a<b?a:b)

#define MODE_ENABLED   1
#define MODE_DISABLED  0

char hostname[256];
FILE *fres = NULL;

static int mode = MODE_DISABLED;
static int debug = 0;
static int trials = 0;

unsigned long long curtime = 0;
unsigned long long lasttime = 0;

/******************************************************************************/
/*                         Sender State Machine                               */
/******************************************************************************/

#define SENDER_IDLE     0
#define SENDER_SYNC     1
#define SENDER_ZERO     2
#define SENDER_LEN      3
#define SENDER_DATA     4
#define SENDER_ACK      5

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

int acked = 0;
int send_ack = 0;
int retries = 0;

void sched(double time, int len, const char *msg) {
    msg_to_send_t *msg_to_send = (msg_to_send_t*) malloc(sizeof(msg_to_send_t));
    msg_to_send->time = time;
    msg_to_send->msg = (char *) malloc((msg_to_send->len = len)+1);
    memcpy(msg_to_send->msg, msg, len);
    msg_to_send->next = NULL;
    if (send_buf_head == NULL) {
        /* empty list */
        send_buf_head = send_buf_tail = msg_to_send;
    } else {
        send_buf_tail->next = msg_to_send;
        send_buf_tail = msg_to_send;
    }
}

void sched_chunked(double time, int rep, int len, const char *msg) {
    int i, slen, chunk;
    const char *smsg;
    for (i = 0; i < rep; i++) {
        slen = len;
        smsg = msg;
        while (slen) {
            if (slen > 255) {
                chunk = 255;
            } else {
                chunk = slen;
            }
            sched(time, chunk, smsg);
            slen -= chunk;
            smsg += chunk;
        }
    }
}

void sched_str(double time, int rep, const char *str) {
    sched_chunked(time, rep, strlen(str), str);
}

void sched_file(double time, int rep, int size, const char *fname) {
    int i;
    char *buf;
    FILE *f;
    for (i = 0; i < rep; i++) {
        buf = (char *) malloc(size);
        f = fopen(fname, "r");
        fread(buf, size, 1, f);
        sched_chunked(time, 1, size, buf);
        fclose(f);
        free(buf);
    }
}

int send(char *buf) {
    msg_to_send_t *tmp;
    if (!mode) {
        *buf = 0;
        return 1;
    }
    switch(sender_state) {
        case SENDER_IDLE:
            /* idle time */
            if (send_buf_head) {
                /* something to send */
                sender_state++;
                sender_step = 0;
            } else {
                /* nothing to send */
                *buf = 0;
                return 1;
            }
        case SENDER_SYNC:
            /* sync pattern */
            if (sender_step < 8)
                *buf = 0x00;
            else
                *buf = 0x55;
            if (++sender_step == 14) {
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
            /*if (fres) {
                fprintf(fres, "%c", *buf);
            }*/
            if (++sender_step == send_buf_head->len) {
                /* done */
                sender_state = SENDER_ACK;
                sender_step  = 0;
                trials = 0;
                lasttime = curtime;
            }
            break;
        case SENDER_ACK:
            /* wait for an ack */
            if (!acked) {
                if (curtime - lasttime < 10000) {
                    *buf = 0;
                    return 0;
                } else {
                    printf("retired!\n");
                    retries++;
                    if (retries == 10) {
                        /* here we go */
                        mode = MODE_DISABLED;
                        retries = 0;
                    }
                }
            }
            tmp = send_buf_head;
            send_buf_head = tmp->next;
            free(tmp->msg);
            free(tmp);
            sender_state = SENDER_IDLE;
            sender_step = 0;
            acked = 0;
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
                printf("\n");
                if (recv_step == 0) {
                    /* 010101|1 */
                    recv_state = RECV_SYNC;
                    recv_step = 0;
                } else if (recv_step == 1) {
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
                if (!recv_len) {
                    /* it is an ACK */
                    acked = 1;
                    printf("\n");
                    recv_state = RECV_SYNC;
                    recv_step = 0;
                }
            }
            break;
        case RECV_DATA:
            recv_msg[recv_step/8] = (recv_msg[recv_step/8]<<1)|bit;
            if (recv_step%8 == 7) {
                printf("%c", recv_msg[recv_step/8]);
                if (fres) {
                    fprintf(fres, "%c", recv_msg[recv_step/8]);
                }
            }
            if (++recv_step == recv_len*8) {
                /* done */
                printf("\n");
                recv_state = RECV_SYNC;
                recv_step = 0;
                send_ack = 1;
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
        /* MPI */
        message_port_register_out(pmt::mp("ctl_out"));
        message_port_register_in(pmt::mp("ctl_in"));
        set_msg_handler(pmt::mp("ctl_in"),
            boost::bind(&brain_cb_impl::ctl_in, this, _1));
        /* get hostname */
        gethostname(hostname, sizeof(hostname));
        printf("host: %s\n", hostname);
        /* setup messages */
        if (!strcmp(hostname, "node1u")) {
            //sched_str(0.1, 1000, "This message is carried over radio waves!");
            //sched_file(0.1, 1, 0x100000, "/mnt/wncp/gr-prototype/music/music.raw");
            sched_file(0.1, 1000, 494, "/mnt/wncp/README");
            //fres = fopen("/mnt/wncp/gr-prototype/music/sent.raw", "w");
        } else /*if (!strcmp(hostname, "node2u"))*/ {
            fres = fopen("/mnt/wncp/gr-prototype/music/sent.raw", "w");
        }
    }

    /*
     * Our virtual destructor.
     */
    brain_cb_impl::~brain_cb_impl()
    {
        if (fres)
            fclose(fres);
    }

    void
    brain_cb_impl::ctl_in(pmt::pmt_t msg) {
        printf("protocol received a message!\n");
        mode = MODE_ENABLED;
        recv_state = RECV_SYNC;
        recv_step  = 0;
        recv_len   = 0;
    }

    void
    brain_cb_impl::ctl_out(int msg) {
        printf("protocol sends a message!\n");
        pmt::pmt_t command = pmt::cons(pmt::mp("cmd"), pmt::mp(1));
        message_port_pub(pmt::mp("ctl_out"), command);
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
        int i, j = 0;
        int premode = mode;

        // consume input
        for (i = 0; i < _ninput_items; i++) {
            recv(in[i]);
            curtime++;
            if (send_ack) {
                if (_noutput_items - j > 11) {
                    out[j++] = 0x00;
                    out[j++] = 0x55;
                    out[j++] = 0x55;
                    out[j++] = 0x55;
                    out[j++] = 0x55;
                    out[j++] = 0x55;
                    out[j++] = 0x55;
                    out[j++] = 0x00;
                    out[j++] = 0x00;
                    out[j++] = 0x00;
                    out[j++] = 0x00;
                    send_ack = 0;
                }
            }
        }

        // tell runtime system how many input items we consumed
        consume_each(_ninput_items);

        // output something
        for (; j < _noutput_items; j++) {
            if (!send(&out[j])) {
                /* nothing to send */
                break;
            }
            //printf("sending 0x%02X\n", out[i]);
        }
        if (premode && !mode) {
            ctl_out(0);
        }

        // tell runtime system how many output items we produced.
        return j;
    }

  } /* namespace prototype */
} /* namespace gr */

