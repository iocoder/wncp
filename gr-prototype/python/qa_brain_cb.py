#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# Copyright 2016 <+YOU OR YOUR COMPANY+>.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import prototype_swig as prototype

class qa_brain_cb(gr.top_block):

    def __init__(self):
        # initialize
        gr.top_block.__init__(self, "Top Block")
        # set up sampling rate
        self.samp_rate = 1
        # instantiate file source
        src = blocks.file_source(itemsize=1,filename="/mnt/wncp/README")
        # instantiate file sink
        snk = blocks.file_sink(itemsize=1,filename="/tmp/README")
        # instantiate the binary sink
        dst = blocks.null_sink(1)
        # instantiate the block
        blk = prototype.brain_cb()
        # connections
        self.connect(src, blk)
        self.connect(blk, snk)

def main():
    tb = qa_brain_cb()
    tb.start()
    try:
        raw_input()
    except EOFError:
        pass
    tb.stop()
    tb.wait()

if __name__ == '__main__':
    main()

#if __name__ == '__main__':
#    gr_unittest.run(qa_brain_cb, "qa_brain_cb.xml")
