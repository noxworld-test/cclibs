/*---------------------------------------------------------------------------------------------------------*\
  File:     libreg.h                                                                    Copyright CERN 2014

  License:  This file is part of libreg.

            libreg is free software: you can redistribute it and/or modify
            it under the terms of the GNU Lesser General Public License as published by
            the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.

            This program is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
            GNU Lesser General Public License for more details.

            You should have received a copy of the GNU Lesser General Public License
            along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Purpose:  Converter Control Regulation library regulation header file

  Contact:  cclibs-devs@cern.ch

  Notes:    The RST algorithm is implemented based on Landau notation:

                ACTUATION x S = REFERENCE x T - MEASUREMENT x R

            The other common notation swaps the R and S polynomials.

            The library uses 32-bit floating point for most of the floating point variables.
            However, there are critical sections of the RST computation that require higher
            precision.  40-bit is sufficient - this is the level available on the TI TMS320C32 DSP.
            On newer processors, 64-bit double precision is needed.

\*---------------------------------------------------------------------------------------------------------*/

#ifndef LIBREG_RST_H
#define LIBREG_RST_H

#include <stdint.h>
#include <libreg/load.h>

// Constants

// REG_N_RST_COEFFS: Maximum number of RST coefficients. That number is hardcoded in several functions, which
// must be reworked when changing the number of RST coefficients: regRstInit(), regRstCalcAct(), regRstCalcRef().

#define REG_N_RST_COEFFS        10

// Regulation RST algorithm structures

enum reg_status                                                 // Regulation status
{
    REG_OK,
    REG_WARNING,
    REG_FAULT
};

enum reg_mode                                                   // Converter regulation mode
{
    REG_NONE,                                                   // No regulation mode set
    REG_VOLTAGE,                                                // Open loop (voltage reference)
    REG_CURRENT,                                                // Closed loop on current
    REG_FIELD                                                   // Closed loop on field
};

struct reg_rst                                                  // RST polynomial arrays and track delay
{
    float                       track_delay;                    // Track delay
    float                       r   [REG_N_RST_COEFFS];         // R polynomial coefficients (meas)
    float                       s   [REG_N_RST_COEFFS];         // S polynomial coefficients (act)
    float                       t   [REG_N_RST_COEFFS];         // T polynomial coefficients (ref)
};

struct reg_rst_pars                                             // RST algorithm parameters
{
    enum reg_status             status;                         // Regulation parameters status
    enum reg_mode               reg_mode;                       // Regulation mode (REG_CURRENT | REG_FIELD)
    uint32_t                    decimate_flag;                  // Decimate measurement flag
    uint32_t                    period_iters;                   // Regulation period (in iterations)
    float                       period;                         // Regulation period
    float                       freq;                           // Regulation frequency
    float                       inv_s0;                         // Store 1/S[0]
    float                       t0_correction;                  // Correction to t[0] for rounding errors
    float                       inv_corrected_t0;               // Store 1/(T[0]+ t0_correction)
    struct reg_rst              rst;                            // RST polynomials
};

struct reg_rst_vars                                             // RST algorithm variables
{
    float                       ref [REG_N_RST_COEFFS];         // RST calculated reference history
    float                       meas[REG_N_RST_COEFFS];         // RST measurement history
    float                       act [REG_N_RST_COEFFS];         // RST actuation history
};

#ifdef __cplusplus
extern "C" {
#endif

// RST regulation functions

uint32_t regRstInit             (struct reg_rst_pars *pars, float iter_period, uint32_t period_iters,
                                 struct reg_load_pars *load, float clbw, float clbw2, float z, float clbw3, float clbw4,
                                 float pure_delay, enum reg_mode reg_mode, uint32_t decimate_flag, struct reg_rst *manual);
float    regRstCalcAct          (struct reg_rst_pars *pars, struct reg_rst_vars *vars, float ref, float meas);
float    regRstCalcRef          (struct reg_rst_pars *pars, struct reg_rst_vars *vars, float act, float meas);
float    regRstHistory          (struct reg_rst_vars *vars);

#ifdef __cplusplus
}
#endif

#endif // LIBREG_RST_H

// EOF
