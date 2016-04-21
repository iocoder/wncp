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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "brain_cb_impl.h"

#define MIN(a,b) (a<b?a:b)

int cnt = 0, calls = 0;

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
        int consumption = MIN(_ninput_items, _noutput_items);

        // done?
//         if (cnt == 494) {
//             printf("No. of calls: %d\n", calls);
//             return WORK_DONE;
//         }

        // increase no of calls counter
        calls++;

        // consume input
        for (int i = 0; i < consumption; i++) {
            // debug
            //printf("%c", in[i]);

            // output something
            out[i] = in[i];

            // increase counter
            cnt++;
        }

        // tell runtime system how many input items we consumed
        consume_each(consumption);

        // tell runtime system how many output items we produced.
        return consumption;
    }

  } /* namespace prototype */
} /* namespace gr */

