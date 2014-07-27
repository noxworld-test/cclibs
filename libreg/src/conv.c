/*---------------------------------------------------------------------------------------------------------*\
  File:     conv.c                                                                       Copyright CERN 2014

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

  Purpose:  This provides a higher level access to libreg with functions that combine all the elements
            needed to regulate current or field in converter.
\*---------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "libreg.h"

//-----------------------------------------------------------------------------------------------------------
// Non-Real-Time Functions - do not call these from the real-time thread or interrupt
//-----------------------------------------------------------------------------------------------------------
uint32_t regConvRstInit(struct reg_conv      *conv,
                        enum reg_mode         mode,
                        enum reg_rst_source   rst_source,
                        uint32_t              period_iters,
                        float                 auxpole1_hz,
                        float                 auxpoles2_hz,
                        float                 auxpoles2_z,
                        float                 auxpole4_hz,
                        float                 auxpole5_hz,
                        float                 pure_delay_periods,
                        float                 track_delay_periods,
                        double                manual_r[REG_N_RST_COEFFS],
                        double                manual_s[REG_N_RST_COEFFS],
                        double                manual_t[REG_N_RST_COEFFS])
{
    struct reg_conv_rst_pars   *conv_rst_pars;
    struct reg_rst              manual;
    uint32_t                    use_next_pars = 0;

    // Set pointer to the reg_conv_rst_pars structure for FIELD/CURRENT regulation with OPERATIONAL/TEST parameters

    if(mode == REG_FIELD)
    {
        conv_rst_pars = (rst_source == REG_OPERATIONAL_RST_PARS ? &conv->b.op_rst_pars : &conv->b.test_rst_pars);
    }
    else
    {
        conv_rst_pars = (rst_source == REG_OPERATIONAL_RST_PARS ? &conv->i.op_rst_pars : &conv->i.test_rst_pars);
    }
printf("regConvRstInit %u\n",mode);

    // If switch flag is not set

    if(conv_rst_pars->use_next_pars == 0)
    {
        // If next pointer was never set then initialise both next and active pointers

        if(conv_rst_pars->next == NULL)
        {
            conv_rst_pars->next   = &conv_rst_pars->pars[0];
            conv_rst_pars->active = &conv_rst_pars->pars[1];
        }

        // Prepare structure with manual RST coefficients

        memcpy(manual.r, manual_r, sizeof(manual_r));
        memcpy(manual.s, manual_s, sizeof(manual_s));
        memcpy(manual.t, manual_t, sizeof(manual_t));

        // if new RST parameters are valid then request switch of RST parameters by RT thread

        if(regRstInit(conv_rst_pars->next, conv->iter_period, period_iters, &conv->load_pars,
                      auxpole1_hz, auxpoles2_hz, auxpoles2_z, auxpole4_hz, auxpole5_hz,
                      pure_delay_periods, track_delay_periods, mode, &manual) != REG_FAULT)
        {
            // conv_rst_pars->use_next_pars can be reset at any time by the real-time thread
            // so the local use_next_pars is returned

            conv_rst_pars->use_next_pars = 1;
        }

        // Take a copy of the newly initialized RST parameters for debug reporting to the user

        conv_rst_pars->debug = *conv_rst_pars->next;
        use_next_pars = 1;
    }

    // use_next_pars will be zero if conv_rst_pars->use_next_pars is already 1 from a previous call to the function

    return(use_next_pars);
}
//-----------------------------------------------------------------------------------------------------------
float regConvPureDelay(struct reg_conv *conv, struct reg_meas_filter *meas_filter, uint32_t reg_period_iters)
/*---------------------------------------------------------------------------------------------------------*\
  This function will estimate the pure loop delay in regulation periods needed by regRstInit().
\*---------------------------------------------------------------------------------------------------------*/
{
    return((conv->sim_vs_pars.v_ref_delay_iters + conv->sim_vs_pars.vs_delay_iters +
            meas_filter->delay_iters[meas_filter->reg_select]) / (float)reg_period_iters);
}
/*---------------------------------------------------------------------------------------------------------*/
void regConvInitSimLoad(struct reg_conv *conv, enum reg_mode mode, float sim_load_tc_error)
/*---------------------------------------------------------------------------------------------------------*\
  This function will initialise the simulated load structures with the specified load parameters
\*---------------------------------------------------------------------------------------------------------*/
{
    regSimLoadTcError(&conv->sim_load_pars, &conv->load_pars, sim_load_tc_error);

    regSimLoadInit(&conv->sim_load_pars, conv->iter_period);

    switch(conv->mode = mode)
    {
    default:

        regSimLoadSetVoltage(&conv->sim_load_pars, &conv->sim_load_vars, conv->v.meas);
        break;

    case REG_CURRENT:

        regSimLoadSetCurrent(&conv->sim_load_pars, &conv->sim_load_vars, conv->i.meas.signal[REG_MEAS_UNFILTERED]);
        break;

    case REG_FIELD:

        regSimLoadSetField(&conv->sim_load_pars, &conv->sim_load_vars, conv->b.meas.signal[REG_MEAS_UNFILTERED]);
        break;
    }

    conv->v.meas = conv->sim_load_vars.circuit_voltage;
    
    conv->i.meas.signal[REG_MEAS_FILTERED] = conv->i.meas.signal[REG_MEAS_UNFILTERED] = conv->sim_load_vars.circuit_current;
    conv->b.meas.signal[REG_MEAS_FILTERED] = conv->b.meas.signal[REG_MEAS_UNFILTERED] = conv->sim_load_vars.magnet_field;
}
//-----------------------------------------------------------------------------------------------------------
void regConvInitMeas(struct reg_conv *conv, struct reg_meas_signal *v_meas_p, struct reg_meas_signal *i_meas_p, struct reg_meas_signal *b_meas_p)
{
    static struct reg_meas_signal null_signal = { 0.0, REG_MEAS_SIGNAL_OK };

    conv->b.input_p = (b_meas_p == NULL ? &null_signal : b_meas_p);;
    conv->i.input_p = (i_meas_p == NULL ? &null_signal : i_meas_p);;
    conv->v.input_p = (v_meas_p == NULL ? &null_signal : v_meas_p);
}
//-----------------------------------------------------------------------------------------------------------
// Real-Time Functions
//-----------------------------------------------------------------------------------------------------------
void regConvSetMeasRT(struct reg_conv *conv, uint32_t use_sim_meas)
{
    if(use_sim_meas == 0)
    {
        // Use measured field, current and voltage measurement and status supplied by application

        conv->b.input = *conv->b.input_p;
        conv->i.input = *conv->i.input_p;
        conv->v.input = *conv->v.input_p;
    }
    else
    {
        // Use simulated measurements which are always OK

        conv->b.input.signal = conv->b.sim.signal;
        conv->i.input.signal = conv->i.sim.signal;
        conv->v.input.signal = conv->v.sim.signal;

        conv->b.input.status = REG_MEAS_SIGNAL_OK;
        conv->i.input.status = REG_MEAS_SIGNAL_OK;
        conv->v.input.status = REG_MEAS_SIGNAL_OK;
    }
}
//-----------------------------------------------------------------------------------------------------------
static void regConvSetModeVoltageRT(struct reg_conv *conv)
{
    // Initialise voltage references according to previous regulation mode

    switch(conv->mode)
    {
        case REG_FIELD:

            conv->v.ref = regRstAverageVrefRT(&conv->rst_vars);
            conv->v.ref_sat = conv->v.ref;
            break;

        case REG_CURRENT:

            conv->v.ref = regRstAverageVrefRT(&conv->rst_vars);
            conv->v.ref_sat = regLoadVrefSatRT(&conv->load_pars, conv->rst_vars.meas[0], conv->v.ref);
            break;

        default:    // NONE

            conv->v.ref_sat = conv->v.ref = 0.0;
            break;
    }

    conv->v.ref_limited = conv->v.ref_sat;

    // Calculate the ref advance for voltage mode

    conv->ref_advance = conv->iter_period * (conv->sim_vs_pars.v_ref_delay_iters + conv->sim_vs_pars.vs_delay_iters);

    // Clear field and current regulation variables

    conv->ref         = 0.0;
    conv->ref_limited = 0.0;
    conv->ref_rst     = 0.0;
    conv->meas        = 0.0;
    conv->rst_vars.meas_track_delay_periods = 0.0;
    regErrResetLimitsVarsRT(&conv->i.err);
    regErrResetLimitsVarsRT(&conv->b.err);
}
//-----------------------------------------------------------------------------------------------------------
static void regConvSetModeFieldOrCurrentRT(struct reg_conv *conv, enum reg_mode mode, uint32_t iteration_counter)
{
    uint32_t             idx;
    float                ref_offset;
    float                rate;
    float                v_ref;
    struct reg_conv_signal   *r;
    struct reg_rst_pars *rst_pars;
    struct reg_rst_vars *rst_vars = &conv->rst_vars;

    // If closing loop on current, adjust v_ref for magnet saturation assuming current is invariant.
    // This assumes it is unlikely that the current regulation will start with the current ramping
    // fast while deep into the magnet saturation zone.

    r = conv->r       = (mode == REG_FIELD ? &conv->b : &conv->i);
    rst_pars         = r->rst_pars;
    v_ref            = (conv->mode == REG_CURRENT ? regLoadInverseVrefSatRT(&conv->load_pars, conv->i.meas.signal[REG_MEAS_UNFILTERED], conv->v.ref_limited) : conv->v.ref_limited);
    rate             = (conv->mode != REG_NONE    ? r->rate.estimate : 0.0);
    conv->meas        = r->meas.signal[r->meas.reg_select] - rate * iteration_counter * conv->iter_period;
    conv->ref_advance = rst_pars->track_delay_periods * rst_pars->period - r->meas.delay_iters[r->meas.reg_select] * conv->iter_period;

    rst_pars->ref_delay_periods = rst_pars->track_delay_periods +
                                 (r->meas.delay_iters[REG_MEAS_FILTERED] - r->meas.delay_iters[r->meas.reg_select]) / (float)rst_pars->period_iters;
    regErrResetLimitsVarsRT(&r->err);

    // Prepare RST histories - assuming that v_ref has been constant when calculating rate

    conv->period            = rst_pars->period;
    conv->iteration_counter = iteration_counter;

    ref_offset  = rate * conv->ref_advance;

    for(idx = 0; idx < REG_N_RST_COEFFS; idx++)
    {
        rst_vars->act [idx] = v_ref;
        rst_vars->meas[idx] = conv->meas - rate * (float)(REG_N_RST_COEFFS - 1 - idx) * conv->period;
        rst_vars->ref [idx] = rst_vars->meas[idx] + ref_offset;
    }

    rst_vars->history_index = idx - 1;

    conv->ref_delayed = regRstDelayedRefRT(rst_pars, &conv->rst_vars, 0);

    conv->ref = conv->ref_limited = conv->ref_rst = rst_vars->ref[rst_vars->history_index];
}
//-----------------------------------------------------------------------------------------------------------
static void regConvSwitchRstParsRT(struct reg_conv_signal *conv_signal, enum reg_rst_source rst_source)
{
    struct reg_rst_pars *rst_pars;

    // Switch operation RST parameter pointers when switch flag is active

    if(conv_signal->op_rst_pars.use_next_pars != 0)
    {
        rst_pars                        = conv_signal->op_rst_pars.next;
        conv_signal->op_rst_pars.next   = conv_signal->op_rst_pars.active;
        conv_signal->op_rst_pars.active = rst_pars;
        conv_signal->op_rst_pars.use_next_pars = 0;

        if(rst_source == REG_OPERATIONAL_RST_PARS)
        {
            conv_signal->rst_pars = rst_pars;
        }
    }

    // Switch test RST parameter pointers when switch flag is active

    if(conv_signal->test_rst_pars.use_next_pars != 0)
    {
        rst_pars                          = conv_signal->test_rst_pars.next;
        conv_signal->test_rst_pars.next   = conv_signal->test_rst_pars.active;
        conv_signal->test_rst_pars.active = rst_pars;
        conv_signal->test_rst_pars.use_next_pars = 0;

        if(rst_source == REG_TEST_RST_PARS)
        {
            conv_signal->rst_pars = rst_pars;
        }
    }
}
//-----------------------------------------------------------------------------------------------------------
void regConvSetModeRT(struct reg_conv *conv, enum reg_mode mode, enum reg_rst_source rst_source, uint32_t iteration_counter)
{
    // Switch RST parameters when required by non-real-time thread

    regConvSwitchRstParsRT(&conv->b, conv->reg_rst_source);
    regConvSwitchRstParsRT(&conv->i, conv->reg_rst_source);

    // If regulation mode has changed

    if(mode != conv->mode)
    {
        switch(mode)
        {
            case REG_NONE:

                conv->v.ref = conv->v.ref_sat = conv->v.ref_limited = 0.0;
                break;

            case REG_VOLTAGE:

                regConvSetModeVoltageRT(conv);
                break;

            default:    // CURRENT or FIELD

                regConvSetModeFieldOrCurrentRT(conv, mode, iteration_counter);
                break;
        }

        // Store the new regulation mode

        conv->mode = mode;
    }

    // Reset max abs error whenever regSetMode is called, even if the mode doesn't change

    conv->i.err.max_abs_err = conv->b.err.max_abs_err = 0.0;
}
//-----------------------------------------------------------------------------------------------------------
void regConvValidateMeas(struct reg_conv *conv)
{
    if(conv->mode != REG_VOLTAGE)
    {
        conv->ref_delayed = regRstDelayedRefRT(conv->r->rst_pars, &conv->rst_vars, conv->iteration_counter);
    }

    // Check voltage measurement

    if(conv->v.input.status != REG_MEAS_SIGNAL_OK)
    {
        // If input voltage measurement is invalid then use voltage source model instead

        conv->v.meas = conv->v.err.delayed_ref;
        conv->v.invalid_input_counter++;
    }
    else
    {
        conv->v.meas = conv->v.input.signal;
    }

    // Check current measurement

    if(conv->i.input.status != REG_MEAS_SIGNAL_OK)
    {
        if(conv->mode == REG_CURRENT)
        {
            // If regulating current then use delayed ref adjusted by the regulation error as the measurement

            conv->i.meas.signal[REG_MEAS_UNFILTERED] = conv->ref_delayed - conv->i.err.err;
        }
        else
        {
            // If not regulating current then extrapolate previous value using the current rate of change

            conv->i.meas.signal[REG_MEAS_UNFILTERED] += conv->i.rate.estimate * conv->iter_period;
        }

        conv->i.invalid_input_counter++;
    }
    else
    {
        conv->i.meas.signal[REG_MEAS_UNFILTERED] = conv->i.input.signal;
    }

    // Check field measurement

    if(conv->b.input.status != REG_MEAS_SIGNAL_OK)
    {
        if(conv->mode == REG_FIELD)
        {
            // If regulating field then use delayed ref adjusted by the regulation error as the measurement

            conv->b.meas.signal[REG_MEAS_UNFILTERED] = conv->ref_delayed - conv->b.err.err;
        }
        else
        {
            // If not regulating current then extrapolate previous value using the current rate of change

            conv->b.meas.signal[REG_MEAS_UNFILTERED] += conv->b.rate.estimate * conv->iter_period;
        }

        conv->b.invalid_input_counter++;
    }
    else
    {
        conv->b.meas.signal[REG_MEAS_UNFILTERED] = conv->b.input.signal;
    }

}
//-----------------------------------------------------------------------------------------------------------
uint32_t regConvRT(struct reg_conv *conv,                 // Regulation structure
                        float           *ref,                 // Ref for voltage, current or field
                        uint32_t         enable_max_abs_err)  // Enable max abs error calculation
{
    struct reg_conv_signal  *reg_r =  conv->r;  // Pointer to active regulation structure (reg->b or reg->i)

    // Switch RST parameters when required by non-real-time thread

    regConvSwitchRstParsRT(&conv->b, conv->reg_rst_source);
    regConvSwitchRstParsRT(&conv->i, conv->reg_rst_source);

    // Return immediately when REG_NONE

    if(conv->mode == REG_NONE)
    {
        conv->v.ref = conv->v.ref_sat = conv->v.ref_limited = 0.0;
        return(0);
    }

    // New iteration - validate measurements before using them

    conv->iteration_counter++;

    regConvValidateMeas(conv);

    // Calculate and check the voltage regulation limits

    regErrCheckLimitsRT(&conv->v.err, 1, 1, conv->v.err.delayed_ref, conv->v.meas);

    // Check current measurement limits

    regLimMeasRT(&conv->i.lim_meas, conv->i.meas.signal[REG_MEAS_UNFILTERED]);

    // Check field measurement limits only when regulating field

    if(conv->mode == REG_FIELD)
    {
        regLimMeasRT(&conv->b.lim_meas, conv->b.meas.signal[REG_MEAS_UNFILTERED]);
    }

    // Calculate voltage reference limits for the measured current (V limits can depend on current)

    regLimVrefCalcRT(&conv->v.lim_ref, conv->i.meas.signal[REG_MEAS_UNFILTERED]);

    // Filter the field and current measurements and prepare to estimate measurement rate

    regMeasFilterRT(&conv->b.meas);
    regMeasRateRT  (&conv->b.rate,conv->b.meas.signal[REG_MEAS_FILTERED],conv->b.rst_pars->period,conv->b.rst_pars->period_iters);

    regMeasFilterRT(&conv->i.meas);
    regMeasRateRT  (&conv->i.rate,conv->i.meas.signal[REG_MEAS_FILTERED],conv->i.rst_pars->period,conv->i.rst_pars->period_iters);

    // If open-loop (voltage regulation) mode - apply voltage ref limits

    if(conv->mode == REG_VOLTAGE)
    {
        conv->v.ref = conv->v.ref_sat = *ref;              // Don't apply magnet saturation compensation

        conv->v.ref_limited = regLimRefRT(&conv->v.lim_ref, conv->iter_period, conv->v.ref, conv->v.ref_limited);

        conv->flags.ref_clip = conv->v.lim_ref.flags.clip;
        conv->flags.ref_rate = conv->v.lim_ref.flags.rate;

        *ref = conv->v.ref_limited;

        conv->iteration_counter = 0;
    }
    else  // else closed-loop on current or field
    {
        // Regulate current or field at the regulation period

        if(conv->iteration_counter >= reg_r->rst_pars->period_iters)
        {
            float v_ref;
            float unfiltered_meas;

            conv->iteration_counter = 0;

            conv->ref        = *ref;
            conv->meas       = reg_r->meas.signal[reg_r->meas.reg_select];
            unfiltered_meas = conv->i.meas.signal[REG_MEAS_UNFILTERED];
            regRstIncHistoryIndexRT(&conv->rst_vars);

            // Apply current reference clip and rate limits

            conv->ref_limited = regLimRefRT(&reg_r->lim_ref, conv->period, conv->ref, conv->ref_limited);

            // Calculate voltage reference using RST algorithm

            conv->v.ref = regRstCalcActRT(reg_r->rst_pars, &conv->rst_vars, conv->ref_limited, conv->meas);

            // Calculate magnet saturation compensation when regulating current only

            conv->v.ref_sat = (conv->mode == REG_CURRENT ? regLoadVrefSatRT(&conv->load_pars, unfiltered_meas, conv->v.ref) : conv->v.ref);

            // Apply voltage reference clip and rate limits

            conv->v.ref_limited = regLimRefRT(&conv->v.lim_ref, conv->period, conv->v.ref_sat, conv->v.ref_limited);

            // If voltage reference has been clipped

            if(conv->v.lim_ref.flags.clip || conv->v.lim_ref.flags.rate)
            {
                // Back calculate the new v_ref before the saturation compensation when regulating current only

                v_ref = (conv->mode == REG_CURRENT ? regLoadInverseVrefSatRT(&conv->load_pars, unfiltered_meas, conv->v.ref_limited) : conv->v.ref_limited);

                // Back calculate new current reference to keep RST histories balanced

                conv->ref_rst = regRstCalcRefRT(reg_r->rst_pars, &conv->rst_vars, v_ref, conv->meas);

                // Mark current reference as rate limited

                reg_r->lim_ref.flags.rate = 1;
            }
            else
            {
                conv->ref_rst = conv->ref_limited;
            }

            conv->flags.ref_clip = reg_r->lim_ref.flags.clip;
            conv->flags.ref_rate = reg_r->lim_ref.flags.rate;

            regRstTrackDelayRT(&conv->rst_vars, conv->period,reg_r->lim_ref.rate_clip);

            *ref = conv->ref_rst;
        }

        // Monitor regulation error using the delayed reference and the unfiltered measurement

        if(reg_r->err_rate == REG_ERR_RATE_MEASUREMENT || conv->iteration_counter == 0)
        {
            regErrCheckLimitsRT(&reg_r->err, 1, enable_max_abs_err,
                                conv->ref_delayed, reg_r->meas.signal[REG_MEAS_FILTERED]);
        }
    }

    // Return 1 if first iteration of the regulation period

    return(conv->iteration_counter == 0);
}
//-----------------------------------------------------------------------------------------------------------
uint32_t regConvRegulate
RT(struct reg_conv *conv,                 // Regulation structure
                        float           *ref,                 // Ref for voltage, current or field
                        uint32_t         enable_max_abs_err)  // Enable max abs error calculation
{
    struct reg_conv_signal  *reg_r =  conv->r;  // Pointer to active regulation structure (reg->b or reg->i)

    // Switch RST parameters when required by non-real-time thread

    regConvSwitchRstParsRT(&conv->b, conv->reg_rst_source);
    regConvSwitchRstParsRT(&conv->i, conv->reg_rst_source);

    // Return immediately when REG_NONE

    if(conv->mode == REG_NONE)
    {
        conv->v.ref = conv->v.ref_sat = conv->v.ref_limited = 0.0;
        return(0);
    }

    // New iteration - validate measurements before using them

    conv->iteration_counter++;

    regConvValidateMeas(conv);

    // Calculate and check the voltage regulation limits

    regErrCheckLimitsRT(&conv->v.err, 1, 1, conv->v.err.delayed_ref, conv->v.meas);

    // Check current measurement limits

    regLimMeasRT(&conv->i.lim_meas, conv->i.meas.signal[REG_MEAS_UNFILTERED]);

    // Check field measurement limits only when regulating field

    if(conv->mode == REG_FIELD)
    {
        regLimMeasRT(&conv->b.lim_meas, conv->b.meas.signal[REG_MEAS_UNFILTERED]);
    }

    // Calculate voltage reference limits for the measured current (V limits can depend on current)

    regLimVrefCalcRT(&conv->v.lim_ref, conv->i.meas.signal[REG_MEAS_UNFILTERED]);

    // Filter the field and current measurements and prepare to estimate measurement rate

    regMeasFilterRT(&conv->b.meas);
    regMeasRateRT  (&conv->b.rate,conv->b.meas.signal[REG_MEAS_FILTERED],conv->b.rst_pars->period,conv->b.rst_pars->period_iters);

    regMeasFilterRT(&conv->i.meas);
    regMeasRateRT  (&conv->i.rate,conv->i.meas.signal[REG_MEAS_FILTERED],conv->i.rst_pars->period,conv->i.rst_pars->period_iters);

    // If open-loop (voltage regulation) mode - apply voltage ref limits

    if(conv->mode == REG_VOLTAGE)
    {
        conv->v.ref = conv->v.ref_sat = *ref;              // Don't apply magnet saturation compensation

        conv->v.ref_limited = regLimRefRT(&conv->v.lim_ref, conv->iter_period, conv->v.ref, conv->v.ref_limited);

        conv->flags.ref_clip = conv->v.lim_ref.flags.clip;
        conv->flags.ref_rate = conv->v.lim_ref.flags.rate;

        *ref = conv->v.ref_limited;

        conv->iteration_counter = 0;
    }
    else  // else closed-loop on current or field
    {
        // Regulate current or field at the regulation period

        if(conv->iteration_counter >= reg_r->rst_pars->period_iters)
        {
            float v_ref;
            float unfiltered_meas;

            conv->iteration_counter = 0;

            conv->ref        = *ref;
            conv->meas       = reg_r->meas.signal[reg_r->meas.reg_select];
            unfiltered_meas = conv->i.meas.signal[REG_MEAS_UNFILTERED];
            regRstIncHistoryIndexRT(&conv->rst_vars);

            // Apply current reference clip and rate limits

            conv->ref_limited = regLimRefRT(&reg_r->lim_ref, conv->period, conv->ref, conv->ref_limited);

            // Calculate voltage reference using RST algorithm

            conv->v.ref = regRstCalcActRT(reg_r->rst_pars, &conv->rst_vars, conv->ref_limited, conv->meas);

            // Calculate magnet saturation compensation when regulating current only

            conv->v.ref_sat = (conv->mode == REG_CURRENT ? regLoadVrefSatRT(&conv->load_pars, unfiltered_meas, conv->v.ref) : conv->v.ref);

            // Apply voltage reference clip and rate limits

            conv->v.ref_limited = regLimRefRT(&conv->v.lim_ref, conv->period, conv->v.ref_sat, conv->v.ref_limited);

            // If voltage reference has been clipped

            if(conv->v.lim_ref.flags.clip || conv->v.lim_ref.flags.rate)
            {
                // Back calculate the new v_ref before the saturation compensation when regulating current only

                v_ref = (conv->mode == REG_CURRENT ? regLoadInverseVrefSatRT(&conv->load_pars, unfiltered_meas, conv->v.ref_limited) : conv->v.ref_limited);

                // Back calculate new current reference to keep RST histories balanced

                conv->ref_rst = regRstCalcRefRT(reg_r->rst_pars, &conv->rst_vars, v_ref, conv->meas);

                // Mark current reference as rate limited

                reg_r->lim_ref.flags.rate = 1;
            }
            else
            {
                conv->ref_rst = conv->ref_limited;
            }

            conv->flags.ref_clip = reg_r->lim_ref.flags.clip;
            conv->flags.ref_rate = reg_r->lim_ref.flags.rate;

            regRstTrackDelayRT(&conv->rst_vars, conv->period,reg_r->lim_ref.rate_clip);

            *ref = conv->ref_rst;
        }

        // Monitor regulation error using the delayed reference and the unfiltered measurement

        if(reg_r->err_rate == REG_ERR_RATE_MEASUREMENT || conv->iteration_counter == 0)
        {
            regErrCheckLimitsRT(&reg_r->err, 1, enable_max_abs_err,
                                conv->ref_delayed, reg_r->meas.signal[REG_MEAS_FILTERED]);
        }
    }

    // Return 1 if first iteration of the regulation period

    return(conv->iteration_counter == 0);
}
//-----------------------------------------------------------------------------------------------------------
void regConvSimulateRT(struct reg_conv *conv, float v_perturbation)
/*---------------------------------------------------------------------------------------------------------*\
  This function will simulate the voltage source and load and the measurements of the voltage, current
  and field. The voltage reference comes from conv->v.ref_limited which is calculated by calling
  regConverter().  A voltage perturbation can be included in the simulation via the v_perturbation parameter.
\*---------------------------------------------------------------------------------------------------------*/
{
    float v_circuit;      // Simulated v_circuit without V_REF_DELAY

    // Simulate voltage source response to v_ref without taking into account V_REF_DELAY

    v_circuit = regSimVsRT(&conv->sim_vs_pars, &conv->sim_vs_vars, conv->v.ref_limited);

    // Simulate load current and field in response to sim_advanced_v_circuit plus the perturbation

    regSimLoadRT(&conv->sim_load_pars, &conv->sim_load_vars, conv->sim_vs_pars.vs_undersampled_flag, v_circuit + v_perturbation);

    // Use delays to estimate the measurement of the magnet's field and the circuit's current and voltage

    conv->b.sim.signal = regDelaySignalRT(&conv->b.sim.meas_delay, conv->sim_load_vars.magnet_field,
                                      conv->sim_vs_pars.vs_undersampled_flag && conv->sim_load_pars.load_undersampled_flag);
    conv->i.sim.signal = regDelaySignalRT(&conv->i.sim.meas_delay, conv->sim_load_vars.circuit_current,
                                      conv->sim_vs_pars.vs_undersampled_flag && conv->sim_load_pars.load_undersampled_flag);
    conv->v.sim.signal = regDelaySignalRT(&conv->v.sim.meas_delay, conv->sim_load_vars.circuit_voltage,
                                      conv->sim_vs_pars.vs_undersampled_flag);

    // Store simulated voltage measurement without noise as the delayed ref for the v_err calculation

    conv->v.err.delayed_ref = conv->v.sim.signal;

    // Simulate noise and tone on simulated measurement of the magnet's field and the circuit's current and voltage

    conv->b.sim.signal += regMeasNoiseAndToneRT(&conv->b.sim.noise_and_tone);
    conv->i.sim.signal += regMeasNoiseAndToneRT(&conv->i.sim.noise_and_tone);
    conv->v.sim.signal += regMeasNoiseAndToneRT(&conv->v.sim.noise_and_tone);
}
// EOF
