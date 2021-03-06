/*---------------------------------------------------------------------------------------------------------*\
  File:     cctest/inc/func/test.h                                                      Copyright CERN 2014

  License:  This file is part of cctest.

            cctest is free software: you can redistribute it and/or modify
            it under the terms of the GNU Lesser General Public License as published by
            the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.

            This program is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
            GNU Lesser General Public License for more details.

            You should have received a copy of the GNU Lesser General Public License
            along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Purpose:  Structure for test function (SINE, COSINE, STEPS, SQUARE) parameters

  Authors:  Quentin.King@cern.ch
\*---------------------------------------------------------------------------------------------------------*/

#ifndef CCPARS_TEST_H
#define CCPARS_TEST_H

#include "ccTest.h"
#include "ccPars.h"
#include "libfg/test.h"

// GLOBALS is defined in source file where global variables should be defined

#ifdef GLOBALS
#define CCPARS_TEST_EXT
#else
#define CCPARS_TEST_EXT extern
#endif

// Libfg TEST parameter structures

CCPARS_TEST_EXT struct fg_test fg_test[CC_NUM_CYC_SELS];

// Test parameters structure

struct ccpars_test
{
    // cctest TEST parameters

    float                       initial_ref;                    // Initial reference
    enum fg_test_type           type;                           // Type of test function
    float                       amplitude_pp;                   // Ref peak-to-peak amplitude
    float                       num_cycles;                     // Number of cycles/steps. This is rounded to the nearest integer.
    float                       period;                         // Period
    enum reg_enabled_disabled   use_window;                     // Window control: true to use window for sine & cosine.
};

CCPARS_TEST_EXT struct ccpars_test ccpars_test[CC_NUM_CYC_SELS]
#ifdef GLOBALS
= {// Default value                Parameter
    {   0.0,                    // TEST INITIAL_REF
        FG_TEST_COSINE,         // Overwritten by init function (SINE, COSINE, STEPS or SQUARE)
        2.0,                    // TEST AMPLITUDE_PP
        3.0,                    // TEST NUM_CYCLES
        2.0,                    // TEST PERIOD
        REG_ENABLED      },     // TEST WINDOW
}
#endif
;

// Test data description structure

CCPARS_TEST_EXT struct ccpars test_pars[]
#ifdef GLOBALS
= {// "Signal name"   type,     max_n_els,*enum,                         *value,                   num_defaults      cyc_sel_step   flags
    { "INITIAL_REF",  PAR_FLOAT,    1,     NULL,                  { .f = &ccpars_test[0].initial_ref  }, 1, sizeof(struct ccpars_test), 0 },
    { "AMPLITUDE_PP", PAR_FLOAT,    1,     NULL,                  { .f = &ccpars_test[0].amplitude_pp }, 1, sizeof(struct ccpars_test), 0 },
    { "NUM_CYCLES",   PAR_FLOAT,    1,     NULL,                  { .f = &ccpars_test[0].num_cycles   }, 1, sizeof(struct ccpars_test), 0 },
    { "PERIOD",       PAR_FLOAT,    1,     NULL,                  { .f = &ccpars_test[0].period       }, 1, sizeof(struct ccpars_test), 0 },
    { "WINDOW",       PAR_ENUM,     1,     enum_enabled_disabled, { .u = &ccpars_test[0].use_window   }, 1, sizeof(struct ccpars_test), 0 },
    { NULL }
}
#endif
;

#endif
// EOF
