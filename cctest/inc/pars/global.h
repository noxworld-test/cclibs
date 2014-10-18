/*---------------------------------------------------------------------------------------------------------*\
  File:     cctest/inc/pars/global.h                                                    Copyright CERN 2014

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

  Purpose:  Structure for the global parameters

  Authors:  Quentin.King@cern.ch
\*---------------------------------------------------------------------------------------------------------*/

#ifndef CCPARS_GLOBAL_H
#define CCPARS_GLOBAL_H

#include "ccCmds.h"

// GLOBALS should be defined in the source file where global variables should be defined

#ifdef GLOBALS
#define CCPARS_GLOBAL_EXT
#else
#define CCPARS_GLOBAL_EXT extern
#endif

// Constants

#define MAX_FUNCS               10              // Max number of functions than can be chained together
#define ZERO_MEAS_FACTOR        0.01            //  1% of positive limit
#define LOW_MEAS_FACTOR         0.1             // 10% of positive limit

// Function type enum - must match order of struct fgfunc funcs[] in ccRef.h

enum fg_types
{
    FG_START,
    FG_DIRECT,
    FG_PLEP,
    FG_RAMP,
    FG_PPPL,
    FG_TABLE,
    FG_STEPS,
    FG_SQUARE,
    FG_SINE,
    FG_COSINE,
    FG_LTRIM,
    FG_CTRIM,
};

CCPARS_GLOBAL_EXT struct ccpars_enum function_type[]
#ifdef GLOBALS
= {
    { FG_START,       "START"  },
    { FG_DIRECT,      "DIRECT" },
    { FG_PLEP,        "PLEP"   },
    { FG_RAMP,        "RAMP"   },
    { FG_PPPL,        "PPPL"   },
    { FG_TABLE,       "TABLE"  },
    { FG_STEPS,       "STEPS"  },
    { FG_SQUARE,      "SQUARE" },
    { FG_SINE,        "SINE"   },
    { FG_COSINE,      "COSINE" },
    { FG_LTRIM,       "LTRIM"  },
    { FG_CTRIM,       "CTRIM"  },
    { 0,               NULL    },
}
#endif
;

// Regulation mode enum constants from libreg

CCPARS_GLOBAL_EXT struct ccpars_enum reg_actuation[]
#ifdef GLOBALS
= {
    { REG_VOLTAGE_REF,    "VOLTAGE" },
    { REG_CURRENT_REF,    "CURRENT" },
    { 0,                   NULL     },
}
#endif
;

CCPARS_GLOBAL_EXT struct ccpars_enum reg_mode[]
#ifdef GLOBALS
= {
    { REG_NONE,       "NONE"    },
    { REG_VOLTAGE,    "VOLTAGE" },
    { REG_CURRENT,    "CURRENT" },
    { REG_FIELD,      "FIELD"   },
    { 0,               NULL     },
}
#endif
;

// Regulation error rate control enum

CCPARS_EXT struct ccpars_enum reg_err_rate[]
#ifdef GLOBALS
= {
    { REG_ERR_RATE_REGULATION,   "REGULATION"     },
    { REG_ERR_RATE_MEASUREMENT,  "MEASUREMENT"    },
    { 0,                          NULL            },
}
#endif
;

// Output format enum

enum cc_csv_format
{
    CC_NONE,
    CC_STANDARD,
    CC_FGCSPY,
    CC_LVDV,
};

CCPARS_GLOBAL_EXT struct ccpars_enum csv_format[]
#ifdef GLOBALS
= {
    { CC_NONE,         "NONE"     },
    { CC_STANDARD,     "STANDARD" },
    { CC_FGCSPY,       "FGCSPY"   },
    { CC_LVDV,         "LVDV"     },
    { 0,               NULL       },
}
#endif
;

// Global parameters structure

struct ccpars_global
{
    float                       run_delay;              // Delay given to libfg for each function
    float                       stop_delay;             // Time after end of last ref function
    uint32_t                    iter_period_us;         // Global iteration period (us)
    float                       abort_time;             // Time to abort the ref function (limits are required)
    uint32_t                    flot_points_max;        // Maximum number of allowed Flot points
    enum reg_actuation          actuation;              // Converter actuation (VOLTAGE REF or CURRENT REF)
    enum reg_enabled_disabled   reverse_time;           // Reverse time flag (tests ref function with decreasing time)
    enum reg_mode               reg_mode[MAX_FUNCS];    // Regulation modes (VOLTAGE, CURRENT or FIELD)
    enum fg_types               function[MAX_FUNCS];    // Ref function types
    float                       dyn_eco_time[2];        // Start/end time since start of function for dynamic economy
    enum reg_err_rate           reg_err_rate;           // Regulation error rate control
    uint32_t                    fg_limits;              // Enable limits for function generator initialisation
    uint32_t                    sim_load;               // Enable load simulation
    uint32_t                    stop_on_error;          // Enable stop on error - this will stop reading the file
    uint32_t                    csv_format;             // CSV output data format
    uint32_t                    flot_output;            // FLOT webplot output control (ENABLED or DISABLED)
    uint32_t                    debug_output;           // Debug output control (ENABLED or DISABLED)
    char *                      group;                  // Test group name (e.g. sandbox or tests)
    char *                      project;                // Project name (e.g. SPS_MPS)
    char *                      file;                   // Results filename root (exclude .csv or .html)
};

CCPARS_GLOBAL_EXT struct ccpars_global ccpars_global
#ifdef GLOBALS
= {//   Default value                  Parameter
        1.0,                        // GLOBAL RUN_DELAY
        1.0,                        // GLOBAL STOP_DELAY
        1000,                       // GLOBAL ITER_PERIOD_US
        0.0,                        // GLOBAL ABORT_TIME
        100000,                     // GLOBAL FLOT_POINTS_MAX
        REG_VOLTAGE_REF,            // GLOBAL ACTUATION
        REG_DISABLED,               // GLOBAL REVERSE_TIME
        { REG_VOLTAGE },            // GLOBAL REG_MODE
        { FG_SINE },                // GLOBAL FUNCTION
        { 0.0, 0.0 },               // GLOBAL DYN_ECO_TIME
        REG_ERR_RATE_REGULATION,    // GLOBAL REG_ERR_RATE
        REG_DISABLED,               // GLOBAL FG_LIMITS
        REG_DISABLED,               // GLOBAL SIM_LOAD
        REG_ENABLED,                // GLOBAL STOP_ON_ERROR
        CC_NONE,                    // GLOBAL CSV_FORMAT
        REG_ENABLED,                // GLOBAL FLOT_OUTPUT
        REG_ENABLED,                // GLOBAL DEBUG_OUTPUT
}
#endif
;

// Global parameters description structure

enum global_pars_index_enum
{
    GLOBAL_RUN_DELAY         ,
    GLOBAL_STOP_DELAY        ,
    GLOBAL_ITER_PERIOD_US    ,
    GLOBAL_ABORT_TIME        ,
    GLOBAL_FLOT_POINTS_MAX   ,
    GLOBAL_ACTUATION         ,
    GLOBAL_REVERSE_TIME      ,
    GLOBAL_REG_MODE          ,
    GLOBAL_FUNCTION          ,
    GLOBAL_DYN_ECO_TIME      ,
    GLOBAL_REG_ERR_RATE      ,
    GLOBAL_FG_LIMITS         ,
    GLOBAL_SIM_LOAD          ,
    GLOBAL_STOP_ON_ERROR     ,
    GLOBAL_CSV_FORMAT        ,
    GLOBAL_FLOT_OUTPUT       ,
    GLOBAL_DEBUG_OUTPUT      ,
    GLOBAL_GROUP             ,
    GLOBAL_PROJECT           ,
    GLOBAL_FILE
};

CCPARS_GLOBAL_EXT struct ccpars global_pars[]
#ifdef GLOBALS
= {// "Signal name"        type,     max_n_els,    *enum,                    *value,                        num_defaults
    { "RUN_DELAY",         PAR_FLOAT,    1,         NULL,             { .f = &ccpars_global.run_delay          }, 1 },
    { "STOP_DELAY",        PAR_FLOAT,    1,         NULL,             { .f = &ccpars_global.stop_delay         }, 1 },
    { "ITER_PERIOD_US",    PAR_UNSIGNED, 1,         NULL,             { .i = &ccpars_global.iter_period_us     }, 1 },
    { "ABORT_TIME",        PAR_FLOAT,    1,         NULL,             { .f = &ccpars_global.abort_time         }, 1 },
    { "FLOT_POINTS_MAX",   PAR_UNSIGNED, 1,         NULL,             { .i = &ccpars_global.flot_points_max    }, 1 },
    { "ACTUATION",         PAR_ENUM,     1,         reg_actuation,    { .i = &ccpars_global.actuation          }, 1 },
    { "REVERSE_TIME",      PAR_ENUM,     1,         enabled_disabled, { .i = &ccpars_global.reverse_time       }, 1 },
    { "REG_MODE",          PAR_ENUM,     MAX_FUNCS, reg_mode,         { .i =  ccpars_global.reg_mode           }, 1 },
    { "FUNCTION",          PAR_ENUM,     MAX_FUNCS, function_type,    { .i =  ccpars_global.function           }, 1 },
    { "DYN_ECO_TIME",      PAR_FLOAT,    2,         NULL,             { .f =  ccpars_global.dyn_eco_time       }, 2 },
    { "REG_ERR_RATE",      PAR_ENUM,     1,         reg_err_rate,     { .i = &ccpars_global.reg_err_rate       }, 1 },
    { "FG_LIMITS",         PAR_ENUM,     1,         enabled_disabled, { .i = &ccpars_global.fg_limits          }, 1 },
    { "SIM_LOAD",          PAR_ENUM,     1,         enabled_disabled, { .i = &ccpars_global.sim_load           }, 1 },
    { "STOP_ON_ERROR",     PAR_ENUM,     1,         enabled_disabled, { .i = &ccpars_global.stop_on_error      }, 1 },
    { "CSV_FORMAT",        PAR_ENUM,     1,         csv_format,       { .i = &ccpars_global.csv_format         }, 1 },
    { "FLOT_OUTPUT",       PAR_ENUM,     1,         enabled_disabled, { .i = &ccpars_global.flot_output        }, 1 },
    { "DEBUG_OUTPUT",      PAR_ENUM,     1,         enabled_disabled, { .i = &ccpars_global.debug_output       }, 1 },
    { "GROUP",             PAR_STRING,   1,         NULL,             { .s = &ccpars_global.group              }, 1 },
    { "PROJECT",           PAR_STRING,   1,         NULL,             { .s = &ccpars_global.project            }, 1 },
    { "FILE",              PAR_STRING,   1,         NULL,             { .s = &ccpars_global.file               }, 1 },
    { NULL }
}
#endif
;

#endif
// EOF
