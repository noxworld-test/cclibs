/*!
 * @file    table.h
 * @date    May 2014
 * @brief   Generate linearly interpolated table functions.
 * @see     http://cern.ch/cclibs
 *
 * The same TABLE configuration can be used with the SPLINE function
 * to smoothly iterpolate the table of points.
 *
 * fgTableGen() receives time as a pointer to constant float rather than as a
 * float value. This allows the user to initialise an array of pointers to
 * functions with the pointer to fgTableGen(). If time is passed by value then
 * the compiler promotes the float to double and prevents the correct initialisation.
 *
 * <h2>Contact</h2>
 *
 * cclibs-devs@cern.ch
 *
 * <h2>Copyright</h2>
 *
 * Copyright CERN 2014. This project is released under the GNU Lesser General
 * Public License version 3.
 * 
 * <h2>License</h2>
 *
 * This file is part of libfg.
 *
 * libfg is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBFG_TABLE_H
#define LIBFG_TABLE_H

#include "libfg.h"

// Types

/*!
 * Table function configuration
 */
struct fg_table_config
{
    float       *ref;               //!< Table reference values
    float       *time;              //!< Table time values
    uintptr_t   ref_n_elements;     //!< Number of elements in reference array
    uintptr_t   time_n_elements;    //!< Number of elements in time array
};

/*!
 * Table function parameters
 */
struct fg_table_pars
{
    uint32_t    seg_idx;            //!< Segment index
    uint32_t    prev_seg_idx;       //!< Previous segment index for which gradient was calculated
    uintptr_t   n_elements;         //!< Number of elements in table
    float       delay;              //!< Time before start of function
    float       *ref;               //!< Table reference values
    float       *time;              //!< Table time values
    float       seg_grad;           //!< Gradient of reference for segment fg_table_pars::prev_seg_idx
};

#ifdef __cplusplus
extern "C" {
#endif

// External functions

uint32_t        fgTableGen (struct fg_table_pars *pars, const double *time, float *ref);
enum fg_error   fgTableInit(struct fg_limits *limits, enum fg_limits_polarity limits_polarity,
                            struct fg_table_config *config, float delay, float min_time_step,
                            struct fg_table_pars *pars, struct fg_meta *meta);

#ifdef __cplusplus
}
#endif

#endif
// EOF
