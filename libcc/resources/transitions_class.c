/*---------------------------------------------------------------------------------------------------------*\
  File:         transitions_class.c

  Contents:

  Notes:


\*---------------------------------------------------------------------------------------------------------*/

#define TRANSITIONS_CLASS_GLOBALS   // define pc_str[], pc_transitions[], trans_FO[], trans_OF[], trans_FS[], trans_SP[], trans_ST[], trans_SA[], trans_TS[], trans_SB[], trans_IL[], trans_TC[], trans_AR[], trans_RN[], trans_AB[], trans_CY[], pc_states[]

#include <transitions_class.h>
#include <cc_types.h>           // basic typedefs
#include <sta_class.h>
#include <sta.h>                // for sta global variable
#include <memmap_mcu.h>         // for DIG_IP1_PWRFAILURE_MASK16
#include <fbs_class.h>          // for FAULTS
#include <fbs.h>                // for fbs global variable
#include <macros.h>             // for Test() macro

/*---------------------------------------------------------------------------------------------------------*/
INT16U OFtoFO(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    // Implementation note: On class 53 the second part of the condition is always FALSE because there are no
    //                      FAST_ABORT and PC_PERMIT faults. There is a compiler warning associated with that.

    return(Test(sta.inputs,DIG_IP1_PWRFAILURE_MASK16) ||
           Test(FAULTS,FGC_FLT_FAST_ABORT|FGC_FLT_NO_PC_PERMIT));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U FStoFO(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(!Test(sta.inputs,DIG_IP1_VSPOWERON_MASK16) &&
           sta.first_faults);
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U FOtoOF(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    // Implementation note: On class 53 the second part of the condition is always FALSE because there are no
    //                      FAST_ABORT and PC_PERMIT faults. There is a compiler warning associated with that.

    return(!(Test(sta.inputs,DIG_IP1_PWRFAILURE_MASK16) ||
             Test(FAULTS,FGC_FLT_FAST_ABORT|FGC_FLT_NO_PC_PERMIT)));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U SPtoOF(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(!Test(sta.flags,STAF_STOP) &&
           !Test(sta.inputs,DIG_IP1_VSPOWERON_MASK16));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U STtoFS(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    // Implementation note: On class 53 the second part of the condition is always FALSE because there are no
    //                      FAST_ABORT and PC_PERMIT faults. There is a compiler warning associated with that.

    return(Test(sta.inputs,DIG_IP1_PWRFAILURE_MASK16) ||
           Test(FAULTS,FGC_FLT_FAST_ABORT|FGC_FLT_NO_PC_PERMIT));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U XXtoFS(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    // Implementation note: On class 53 the last part of the condition is always FALSE because there is no
    //                      PC_PERMIT fault. There is a compiler warning associated with that.

    return(Test(sta.inputs,DIG_IP1_PWRFAILURE_MASK16|DIG_IP1_FASTABORT_MASK16) ||
           (!Test(sta.inputs,DIG_IP1_VSREADY_MASK16) &&
            Test(FAULTS,FGC_FLT_NO_PC_PERMIT)));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U STtoSP(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_STOP) ||
           (!Test(sta.flags,STAF_START) &&
            !Test(sta.inputs,DIG_IP1_VSRUN_MASK16)));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U XXtoSP(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_STOP) ||
           !Test(sta.inputs,DIG_IP1_VSREADY_MASK16) ||
           !Test(sta.inputs,DIG_IP1_VSRUN_MASK16));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U OFtoST(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_START));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U XXtoSA(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.inputs,DIG_IP1_INTLKSPARE_MASK16) ||
           Test(sta.flags,STAF_SLOW_ABORT));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U STtoTS(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.inputs,DIG_IP1_VSPOWERON_MASK16) &&
           Test(sta.inputs,DIG_IP1_VSREADY_MASK16));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U XXtoTS(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_TO_STANDBY) &&
           !Test(sta.flags,STAF_CYCLING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U TStoSB(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(!Test(sta.flags,STAF_TO_STANDBY));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U TStoAB(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_ABORTING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U SBtoIL(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_IDLE));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U ARtoIL(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_IDLE));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U RNtoIL(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(!Test(sta.flags,STAF_RUNNING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U ABtoIL(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(!Test(sta.flags,STAF_ABORTING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U SAtoAB(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_ABORTING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U ILtoAR(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_ARMED));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U ILtoTC(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_TO_CYCLING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U ARtoRN(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_RUNNING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U RNtoAB(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_ABORTING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U SBtoTC(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_TO_CYCLING));
}
/*---------------------------------------------------------------------------------------------------------*/
INT16U TCtoCY(void)
/*---------------------------------------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------------------------------------*/
{
    return(Test(sta.flags,STAF_CYCLING));
}
/*---------------------------------------------------------------------------------------------------------*\
  End of file: transitions_class.c
\*---------------------------------------------------------------------------------------------------------*/
