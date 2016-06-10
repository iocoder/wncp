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

#include <gnuradio/io_signature.h>
#include "protocol_impl.h"

#define MODE_IDLE       0
#define MODE_TRANSIT    1
#define MODE_SCANNING   2

static int mode = MODE_SCANNING;

static int channels[11] = {
    2412, 2417, 2422, 2427, 2432,
    2437, 2442, 2447, 2452, 2457,
    2462
};

static int ranking[11] = {0};

static int cur_channel = 0;
static int no_samples  = 0;

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
              gr::io_signature::make(2, 2, sizeof(char))) /* output */ {
        /* constructor */
        printf("Hello World!\n");
    }

    /*
     * Our virtual destructor.
     */
    protocol_impl::~protocol_impl()
    {
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
      const float *usrp_in = (const float *) input_items[0];

      /* process input */
      if (_ninput_items) {
            if (mode == MODE_IDLE) {

            } else if (mode == MODE_TRANSIT) {

            } else if (mode == MODE_SCANNING) {

            }
      }

      /* how many input items were consumed? */

      char *out = (char *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace prototype */
} /* namespace gr */

