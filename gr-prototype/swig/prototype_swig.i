/* -*- c++ -*- */

#define PROTOTYPE_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "prototype_swig_doc.i"

%{
#include "prototype/brain_cb.h"
%}


%include "prototype/brain_cb.h"
GR_SWIG_BLOCK_MAGIC2(prototype, brain_cb);
