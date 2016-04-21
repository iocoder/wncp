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

int cnt = 0;

namespace gr {
  namespace prototype {

    brain_cb::sptr
    brain_cb::make()
    {
      return gnuradio::get_initial_sptr
        (new brain_cb_impl());
    }

    /*
     * The private constructor
     */
    brain_cb_impl::brain_cb_impl()
      : gr::block("brain_cb",
              gr::io_signature::make(0, 10, sizeof(int)), /* input */
              gr::io_signature::make(0, 10, sizeof(int))) /* output */
    {
        printf("Hello World!\n");
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
    }

    int
    brain_cb_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      printf("%d\n", cnt++);
      const int *in = (const int *) input_items[0];
      int *out = (int *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace prototype */
} /* namespace gr */

