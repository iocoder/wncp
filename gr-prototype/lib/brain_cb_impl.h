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

#ifndef INCLUDED_PROTOTYPE_BRAIN_CB_IMPL_H
#define INCLUDED_PROTOTYPE_BRAIN_CB_IMPL_H

#include <prototype/brain_cb.h>

namespace gr {
  namespace prototype {

    class brain_cb_impl : public brain_cb
    {
     private:
      // Nothing to declare in this block.

     public:
      brain_cb_impl();
      ~brain_cb_impl();

      // Where all the action really happens
      void ctl_in(pmt::pmt_t msg);

      void ctl_out(int msg);

      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace prototype
} // namespace gr

#endif /* INCLUDED_PROTOTYPE_BRAIN_CB_IMPL_H */

