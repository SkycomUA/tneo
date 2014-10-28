/*******************************************************************************
 *
 * TNeoKernel: real-time kernel initially based on TNKernel
 *
 *    TNKernel:                  copyright � 2004, 2013 Yuri Tiomkin.
 *    PIC32-specific routines:   copyright � 2013, 2014 Anders Montonen.
 *    TNeoKernel:                copyright � 2014       Dmitry Frank.
 *
 *    TNeoKernel was born as a thorough review and re-implementation of
 *    TNKernel. The new kernel has well-formed code, inherited bugs are fixed
 *    as well as new features being added, and it is tested carefully with
 *    unit-tests.
 *
 *    API is changed somewhat, so it's not 100% compatible with TNKernel,
 *    hence the new name: TNeoKernel.
 *
 *    Permission to use, copy, modify, and distribute this software in source
 *    and binary forms and its documentation for any purpose and without fee
 *    is hereby granted, provided that the above copyright notice appear
 *    in all copies and that both that copyright notice and this permission
 *    notice appear in supporting documentation.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE DMITRY FRANK AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DMITRY FRANK OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/**
 *
 * \file
 *
 * PIC32 architecture-dependent routines
 *
 */

#ifndef  _TN_ARCH_PIC32_H
#define  _TN_ARCH_PIC32_H

//-- this include is needed to get build-time configuration
//   (TN_DEBUG is used)
#include "../../core/tn_common.h"


#ifdef __cplusplus
extern "C"  {     /*}*/
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define  _TN_PIC32_INTSAVE_DATA_INVALID   0xffff

#if TN_DEBUG
#  define   _TN_PIC32_INTSAVE_CHECK()                          \
{                                                              \
   if (tn_save_status_reg == _TN_PIC32_INTSAVE_DATA_INVALID){  \
      _TN_FATAL_ERROR("");                                     \
   }                                                           \
}
#else
#  define   _TN_PIC32_INTSAVE_CHECK()  /* nothing */
#endif

/**
 * FFS - find first set bit. Used in `_find_next_task_to_run()` function.
 * Say, for `0xa8` it should return `3`.
 *
 * May be not defined: in this case, naive algorithm will be used.
 */
//#define  _TN_FFS(x) (32 - __builtin_clz((x) & (0 - (x))))

/**
 * Used by the kernel as a signal that something really bad happened.
 * Indicates TNeoKernel bugs as well as illegal kernel usage
 * (e.g. sleeping in the idle task callback)
 *
 * Typically, set to assembler instruction that causes debugger to halt.
 */
#define  _TN_FATAL_ERROR(error_msg, ...)         \
   {__asm__ volatile(".pword 0xDA4000"); __asm__ volatile ("nop");}



/**
 * \def TN_ARCH_STK_ATTR_BEFORE
 *
 * Compiler-specific attribute that should be placed **before** declaration of
 * array used for stack. It is needed because there are often additional 
 * restrictions applied to alignment of stack, so, to meet them, stack arrays
 * need to be declared with these macros.
 *
 * @see TN_ARCH_STK_ATTR_AFTER
 */

/**
 * \def TN_ARCH_STK_ATTR_AFTER
 *
 * Compiler-specific attribute that should be placed **after** declaration of
 * array used for stack. It is needed because there are often additional 
 * restrictions applied to alignment of stack, so, to meet them, stack arrays
 * need to be declared with these macros.
 *
 * @see TN_ARCH_STK_ATTR_BEFORE
 */

#if defined (__C30__)
#  define TN_ARCH_STK_ATTR_BEFORE
#  define TN_ARCH_STK_ATTR_AFTER
#else
#  error "Unknown compiler"
#endif

/**
 * Minimum task's stack size, in words, not in bytes; includes a space for
 * context plus for parameters passed to task's body function.
 */
#define  TN_MIN_STACK_SIZE          36//TODO

/**
 * Width of `int` type.
 */
#define  TN_INT_WIDTH               16

/**
 * Unsigned integer type whose size is equal to the size of CPU register.
 * Typically it's plain `unsigned int`.
 */
typedef  unsigned int               TN_UWord;


/**
 * Maximum number of priorities available, this value usually matches
 * `#TN_INT_WIDTH`.
 *
 * @see TN_PRIORITIES_CNT
 */
#define  TN_PRIORITIES_MAX_CNT      TN_INT_WIDTH

/**
 * Value for infinite waiting, usually matches `UINT_MAX`
 */
#define  TN_WAIT_INFINITE           0xFFFF

/**
 * Value for initializing the task's stack
 */
#define  TN_FILL_STACK_VAL          0xFEED




/**
 * Declares variable that is used by macros `TN_INT_DIS_SAVE()` and
 * `TN_INT_RESTORE()` for storing status register value.
 *
 * It is good idea to initially set it to some invalid value,
 * and if TN_DEBUG is non-zero, check it in TN_INT_RESTORE().
 * Then, we can catch bugs if someone tries to restore interrupts status
 * without saving it first.
 *
 * @see `TN_INT_DIS_SAVE()`
 * @see `TN_INT_RESTORE()`
 */
#define  TN_INTSAVE_DATA            \
   int tn_save_status_reg = _TN_PIC32_INTSAVE_DATA_INVALID;

/**
 * The same as `#TN_INTSAVE_DATA` but for using in ISR together with
 * `TN_INT_IDIS_SAVE()`, `TN_INT_IRESTORE()`.
 *
 * @see `TN_INT_IDIS_SAVE()`
 * @see `TN_INT_IRESTORE()`
 */
#define  TN_INTSAVE_DATA_INT        TN_INTSAVE_DATA

/**
 * \def TN_INT_DIS_SAVE()
 *
 * Disable interrupts and return previous value of status register,
 * atomically. Similar `tn_arch_sr_save_int_dis()`, but implemented
 * as a macro, so it is potentially faster.
 *
 * Uses `#TN_INTSAVE_DATA` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA`
 * @see `tn_arch_sr_save_int_dis()`
 */

/**
 * \def TN_INT_RESTORE()
 *
 * Restore previously saved status register.
 * Similar to `tn_arch_sr_restore()`, but implemented as a macro,
 * so it is potentially faster.
 *
 * Uses `#TN_INTSAVE_DATA` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA`
 * @see `tn_arch_sr_save_int_dis()`
 */

#  define TN_INT_DIS_SAVE()   tn_save_status_reg = tn_arch_sr_save_int_dis()
#  define TN_INT_RESTORE()    _TN_PIC32_INTSAVE_CHECK();                      \
                              tn_arch_sr_restore(tn_save_status_reg)

/**
 * The same as `TN_INT_DIS_SAVE()` but for using in ISR.
 *
 * Uses `#TN_INTSAVE_DATA_INT` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA_INT`
 */
#define TN_INT_IDIS_SAVE()       TN_INT_DIS_SAVE()

/**
 * The same as `TN_INT_RESTORE()` but for using in ISR.
 *
 * Uses `#TN_INTSAVE_DATA_INT` as a temporary storage.
 *
 * @see `#TN_INTSAVE_DATA_INT`
 */
#define TN_INT_IRESTORE()        TN_INT_RESTORE()

/**
 * Returns nonzero if interrupts are disabled, zero otherwise.
 */
#define TN_IS_INT_DISABLED()     (_tn_arch_is_int_disabled())

/**
 * Pend context switch from interrupt.
 */
#define _TN_CONTEXT_SWITCH_IPEND_IF_NEEDED()          \
   _tn_context_switch_pend_if_needed()

/**
 * Converts size in bytes to size in `#TN_UWord`.
 * For 32-bit platforms, we should shift it by 2 bit to the right;
 * for 16-bit platforms, we should shift it by 1 bit to the right.
 */
#define _TN_SIZE_BYTES_TO_UWORDS(size_in_bytes)    ((size_in_bytes) >> 1)



#endif   //-- DOXYGEN_SHOULD_SKIP_THIS










// ---------------------------------------------------------------------------

extern volatile unsigned int   __tn_p24_dspic_inside_isr;

   #define _avixInternalISR_Prologue                        \
"   push   w0;                             \n"   \
"   push   w1;                             \n"   \
"   push   w15;                            \n"   \
"   mov #__tn_p24_dspic_inside_isr, w0;      \n"   \
"   mov #1, w1;      \n"   \
"   mov w1, [w0];      \n"   \


   #define _avixInternalISR_Activate                        \
         "   rcall    1f;                              \n"

   #define _avixInternalISR_Epilogue                        \
"   mov #__tn_p24_dspic_inside_isr, w0;      \n"   \
"   mov #0, w1;      \n"   \
"   mov w1, [w0];      \n"   \
"   pop      w15;                             \n"   \
"   pop      w1;                              \n"   \
"   pop      w0;                              \n"   \
"   retfie;                                 \n"   \
"1: mov      SR, w0;                           \n"   \
"   mov.b   w0, [w15-1];                         "


   #define _avixDeclareISRInternalUse(f,p,s)                  \
                                                   \
      void __attribute__((__interrupt__                     \
         (                                          \
            __preprologue__                              \
            (   _avixInternalISR_Prologue                  \
               _avixInternalISR_Activate                  \
               _avixInternalISR_Epilogue                  \
            )                                       \
         ),                                          \
         p, s)) f(void)




#define  tn_soft_isr(f,p)  _avixDeclareISRInternalUse(f,p,)




#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif   // _TN_ARCH_PIC32_H

