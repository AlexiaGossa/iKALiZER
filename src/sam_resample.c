#include "sam_header.h"

/****************************************************************************
*
* Name: resamp.h
*
* Synopsis:
*
*   Resamples a signal.  For more information about resampling, see dspGuru's
*   Multirate FAQ at:
*
*       http://www.dspguru.com/info/faqs/mrfaq.htm
*
* Description: See function descriptons below.
*
* by Grant R. Griffin
* Provided by Iowegian's "dspGuru" service (http://www.dspguru.com).
* Copyright 2001, Iowegian International Corporation (http://www.iowegian.com)
*
*                          The Wide Open License (WOL)
*
* Permission to use, copy, modify, distribute and sell this software and its
* documentation for any purpose is hereby granted without fee, provided that
* the above copyright notice and this license appear in all source copies. 
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
* ANY KIND. See http://www.dspguru.com/wol.htm for more information.
*
*****************************************************************************/

/*****************************************************************************
Description:

    resamp - "resamples" a signal by changing its sampling rate by a
              rational factor via FIR filtering.

    Two similar implementations are supplied, resamp0 and resamp1, which can
    be selected via the "#define resamp" below.  resamp0 is simpler (and 
    therefore easier to understand), but resamp1 is faster.

Inputs:

    RATIO = interp_factor_L / decim_factor_M !!!

    interp_factor_L:
        the interpolation factor (must be >= 1)

    decim_factor_M:
        the decimation factor (must be >= 1)

    num_taps_per_phase:
        the number of taps per polyphase filter, which is the total number of
        taps in the filter divided by interp_factor_L.

    p_H:
        pointer to the array of coefficients for the resampling filter.
        (The number of total taps in p_H is interp_factor * num_taps_per_phase,
        so be sure to design your filter using a number of taps which is evenly
        divisible by interp_factor.)

    num_inp:
        the number of input samples

    p_inp:
        pointer to the input samples

Input/Outputs:

    p_current_phase:
        pointer to the current phase (must be >= 0 and <= interp_factor_L)

    p_Z:
        pointer to the delay line array (must have num_taps_per_phase
        elements)    

Outputs:

    p_out:
        pointer to the output sample array.

    p_num_out:
        pointer to the number of output samples

*****************************************************************************/

#define resamp resamp1          /* select resamp0 or resamp1 here */

/* resamp0 is simpler but slower */

void resamp0(int interp_factor_L, int decim_factor_M, int num_taps_per_phase,
             int *p_current_phase, const float *const p_H, float *const p_Z,
             int num_inp, const float *p_inp, float *p_out, int *p_num_out);

/* resamp1 is more complicated but faster */

void resamp1(int interp_factor_L, int decim_factor_M, int num_taps_per_phase,
             int *p_current_phase, const float *const p_H, float *const p_Z,
             int num_inp, const float *p_inp, float *p_out, int *p_num_out);



/****************************************************************************/
void resamp0(int interp_factor_L, int decim_factor_M, int num_taps_per_phase,
             int *p_current_phase, const float *const p_H,
             float *const p_Z, int num_inp, const float *p_inp,
             float *p_out, int *p_num_out)
{
    int tap, num_out, phase_num = *p_current_phase;
    const float *p_coeff;
    float sum;

    num_out = 0;
    while (num_inp > 0) {
        /* shift input samples into Z delay line */
        while (phase_num >= interp_factor_L) {
            /* decrease phase number by interpolation factor L */
            phase_num -= interp_factor_L;

            /* shift Z delay line up to make room for next sample */
            for (tap = num_taps_per_phase - 1; tap >= 1; tap--) {
                p_Z[tap] = p_Z[tap - 1];
            }

            /* copy next sample from input buffer to bottom of Z delay line */
            p_Z[0] = *p_inp++;

            if (--num_inp == 0) {
                break;
            }
        }

        /* calculate outputs */
        while (phase_num < interp_factor_L) {
            /* point to the current polyphase filter */
            p_coeff = p_H + phase_num;

            /* calculate FIR sum */
            sum = 0.0;
            for (tap = 0; tap < num_taps_per_phase; tap++) {
                sum += *p_coeff * p_Z[tap];
                p_coeff += interp_factor_L;  /* point to next coefficient */
            }
            *p_out++ = sum;     /* store sum and point to next output */
            num_out++;

            /* increase phase number by decimation factor M */
            phase_num += decim_factor_M;
        }
    }

    /* pass phase number and number of outputs back to caller */
    *p_current_phase = phase_num;
    *p_num_out = num_out;
}

/****************************************************************************/
void resamp1(int interp_factor_L, int decim_factor_M, int num_taps_per_phase,
             int *p_current_phase, const float *const p_H,
             float *const p_Z, int num_inp, const float *p_inp,
             float *p_out, int *p_num_out)
{
    int tap, num_out, num_new_samples, phase_num = *p_current_phase;
    const float *p_coeff;
    float sum;

    num_out = 0;
    while (num_inp > 0) {

        /* figure out how many new samples to shift into Z delay line */
        num_new_samples = 0;
        while (phase_num >= interp_factor_L) {
            /* decrease phase number by interpolation factor L */
            phase_num -= interp_factor_L;
            num_new_samples++;
            if (--num_inp == 0) {
                break;
            }
        }

        if (num_new_samples >= num_taps_per_phase) {
            /* the new samples are bigger than the size of Z:
               fill the entire Z with the tail of new inputs */
            p_inp += (num_new_samples - num_taps_per_phase);
            num_new_samples = num_taps_per_phase;
        }

        /* copy new samples into Z */

        /* shift Z delay line up to make room for next samples */
        for (tap = num_taps_per_phase - 1; tap >= num_new_samples; tap--) {
            p_Z[tap] = p_Z[tap - num_new_samples];
        }

        /* copy next samples from input buffer to bottom of Z */
        for (tap = num_new_samples - 1; tap >= 0; tap--) {
            p_Z[tap] = *p_inp++;
        }

        /* calculate outputs */
        while (phase_num < interp_factor_L) {
            /* point to the current polyphase filter */
            p_coeff = p_H + phase_num;

            /* calculate FIR sum */
            sum = 0.0;
            for (tap = 0; tap < num_taps_per_phase; tap++) {
                sum += *p_coeff * p_Z[tap];
                p_coeff += interp_factor_L;   /* point to next coefficient */
            }
            *p_out++ = sum;     /* store sum and point to next output */
            num_out++;

            /* decrease phase number by decimation factor M */
            phase_num += decim_factor_M;
        }
    }

    /* pass back to caller phase number (for next call) and number of
       outputs */
    *p_current_phase = phase_num;
    *p_num_out = num_out;
}
