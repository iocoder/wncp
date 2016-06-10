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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>

#include <gnuradio/io_signature.h>
#include "protocol_impl.h"

#define MODE_IDLE       0
#define MODE_TRANSIT    1
#define MODE_SCANNING   2
#define MODE_INIT       3

#define SAMPLES_COUNT   200000
#define ITERS_COUNT     1

static int mode = MODE_INIT;

static double channels[11] = {
    1.412e9, 2.417e9, 2.422e9, 2.427e9, 2.432e9,
    2.437e9, 2.442e9, 2.447e9, 2.452e9, 2.457e9,
    2.462e9
};

static float ranking[11] = {0};

static int cur_channel = 0;
static int no_samples  = 0;
static int act_samples = 0;
static int iteration   = 0;

namespace gr {
  namespace prototype {

    protocol::sptr
    protocol::make()
    {
      return gnuradio::get_initial_sptr
        (new protocol_impl());
    }

    /*
     * The private constructor
     */
    protocol_impl::protocol_impl()
      : gr::block("protocol",
              gr::io_signature::make(1, 1, sizeof(float)), /* input */
              gr::io_signature::make(0, 1, sizeof(char)))  /* output */ {
        /* constructor */
        printf("Hello World!\n");
        message_port_register_out(pmt::mp("cmd"));
        message_port_register_out(pmt::mp("cmd_trans"));
        message_port_register_out(pmt::mp("ctl_out"));
        message_port_register_in(pmt::mp("ctl_in"));
        set_msg_handler(pmt::mp("ctl_in"),
            boost::bind(&protocol_impl::ctl_in, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    protocol_impl::~protocol_impl()
    {
    }

    void
    protocol_impl::switch_channel(int new_channel, int trans) {
        double freq = channels[new_channel];
        cur_channel = new_channel;
        pmt::pmt_t command = pmt::cons(pmt::mp("freq"), pmt::mp(freq));
        message_port_pub(pmt::mp("cmd"), command);
        if (trans == 0) {
            command = pmt::cons(pmt::mp("gain"), pmt::mp(0));
            message_port_pub(pmt::mp("cmd_trans"), command);
        } else {
            command = pmt::cons(pmt::mp("freq"), pmt::mp(freq));
            message_port_pub(pmt::mp("cmd_trans"), command);
            command = pmt::cons(pmt::mp("gain"), pmt::mp(100));
            message_port_pub(pmt::mp("cmd_trans"), command);
        }
    }

    void
    protocol_impl::ctl_in(pmt::pmt_t msg) {
        printf("brain received a message!\n");
        mode = MODE_INIT;
    }

    void
    protocol_impl::ctl_out(int msg) {
        printf("brain sends a message!\n");
        pmt::pmt_t command = pmt::cons(pmt::mp("cmd"), pmt::mp(1));
        message_port_pub(pmt::mp("ctl_out"), command);
    }

    void
    protocol_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    protocol_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      int nusrp_items = ninput_items[0];
      int nuctl_items = noutput_items;
      const float *usrp_in = (const float *) input_items[0];
      char *out = (char *) output_items[0];

      /* process input */
      if (nusrp_items) {
            if (mode == MODE_IDLE) {
                if (no_samples < 0) {
                } else if (no_samples > 1000000) {
                    ctl_out(1);
                    no_samples = -1;
                } else {
                    no_samples += nusrp_items;
                }
            } else if (mode == MODE_TRANSIT) {

            } else if (mode == MODE_SCANNING) {
                for (int i = 0; i < nusrp_items; i++) {
                    //ranking[cur_channel] += usrp_in[i];
                    if (usrp_in[i] > ranking[cur_channel])
                        ranking[cur_channel] = usrp_in[i];
                    act_samples++;
                    if (++no_samples == SAMPLES_COUNT) {
                        /* move to next channel */
                        no_samples = 0;
                        if (++cur_channel == 11) {
                            cur_channel = 0;
                            /* done */
                            int selected = -1;
                            if (++iteration == ITERS_COUNT) {
                                iteration = 0;
                                for (int j = 0; j < 11; j++) {
                                    printf("channel %d: %f %d %f\n", j+1,
                                           ranking[j], act_samples,
                                           ranking[j]/act_samples);
                                    if (selected == -1 && ranking[j] < 0.4)
                                        selected = j;
                                }
                                if (selected == -1) {
                                    printf("failed to find free channel!\n");
                                } else {
                                    printf("selected: %d\n", selected+1);
                                    cur_channel = selected;
                                    switch_channel(cur_channel, 1);
                                }
                                mode = MODE_IDLE;
                                no_samples = 0;
                            } else {
                                /* switch to channel 0 */
                                switch_channel(0, 0);
                            }
                        } else {
                            /* switch to channel */
                            switch_channel(cur_channel, 0);
                        }
                        act_samples = 0;
                        break;
                    }
                }
            } else if (mode == MODE_INIT) {
                if (no_samples == 0) {
                    switch_channel(0, 0);
                }
                no_samples += nusrp_items;
                if (no_samples > 1000000) {
                    cur_channel = 0;
                    no_samples = 0;
                    act_samples = 0;
                    iteration = 0;
                    mode = MODE_SCANNING;
                    switch_channel(0, 0);
                }
            }
      }

      /* how many input items were consumed? */
      consume(0, ninput_items[0]);
      //consume(1, ninput_items[1]);

      /* something to output? */



      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace prototype */
} /* namespace gr */

