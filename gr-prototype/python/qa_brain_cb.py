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
from gnuradio import uhd
from gnuradio import digital
import time
import prototype_swig as prototype

class qa_brain_cb(gr.top_block):

    def __init__(self):
        # initialize
        gr.top_block.__init__(self, "Top Block")
        # set up sampling rate
        self.samp_rate = 0.5e6
        # instantiate usrp source
        usrc = uhd.usrp_source(
            ",".join(("", "")),
            uhd.stream_args(cpu_format="fc32",channels=range(1))
        )
        usrc.set_samp_rate(self.samp_rate)
        usrc.set_gain(100, 0)
        usrc.set_center_freq(1.241e9, 0)
        # instantiate complex to mag converter
        pwr = blocks.complex_to_mag(1)
        # instantiate psk demodulator
        demod = digital.psk.psk_demod(
          constellation_points=2,
          differential=True,
          samples_per_symbol=4,
          excess_bw=0.35,
          phase_bw=6.28/100.0,
          timing_bw=6.28/100.0,
          mod_code="gray",
          verbose=False,
          log=False
        )
        # instantiate our block
        blk = prototype.brain_cb()
        # instantiate the brain
        brain = prototype.protocol()
        # instantiate psk modulator
        mod = digital.psk.psk_mod(
          constellation_points=2,
          mod_code="gray",
          differential=True,
          samples_per_symbol=4,
          excess_bw=0.35,
          verbose=False,
          log=False,
        )
        # instantiate usrp sink
        usnk = uhd.usrp_sink(
            ",".join(("", "")),
            uhd.stream_args(cpu_format="fc32",channels=range(1)),
        )
        usnk.set_samp_rate(self.samp_rate)
        usnk.set_gain(100, 0)
        usnk.set_center_freq(1.241e9, 0)
        # instantiate file sink
        fsnk = blocks.file_sink(itemsize=1,filename="/tmp/README")
        # straight connections
        self.connect(usrc,  demod)
        self.connect(demod, fsnk)
        self.connect(demod, blk)
        self.connect(blk,   mod)
        self.connect(mod,   usnk)
        # gay connections
        self.connect(usrc,  pwr)
        self.connect(pwr,   brain)
        self.msg_connect((brain, 'cmd'), (usrc, 'command'))
        self.msg_connect((brain, 'cmd_trans'), (usnk, 'command'))

def main():
    tb = qa_brain_cb()
    tb.start()
    time.sleep(1000)
    tb.stop()
    tb.wait()

if __name__ == '__main__':
    main()

#if __name__ == '__main__':
#    gr_unittest.run(qa_brain_cb, "qa_brain_cb.xml")
