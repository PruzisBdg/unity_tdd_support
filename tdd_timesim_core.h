/* -----------------------------------------------------------------
|
|   This simulates the advance of time ('SysTime') on the Unity TDD platform for
|   those modules which use Systime.
|
|   $Id $
|
|   $Log $
|
--------------------------------------------------------------------- */

#ifndef TDD_TIMESIM_CORE_H
#define TDD_TIMESIM_CORE_H

#include "GenericTypeDefs.h"
#include "systime.h"

typedef U16 TimeSim_T_Instr;            // Holds a command and flags
typedef U8  TimeSim_T_InstrIdx;         // Up to 255 steps in a test sequence.
typedef U8  TimeSim_T_Rtn;              // from timeSim_Step()
typedef U8  TimeSim_T_Repeats;          // from timeSim_Step()

#define _TimeSim_Repeats_Unlimited 0xFF // Infinite repeat count; until stop for some other reason.

#define _TimeSim_Rtn_Continue   0       // More steps to run
#define _TimeSim_Rtn_Done       1       // Completed an instruction sequence
#define _TimeSim_Rtn_Failed     2       // Failed somehow. Quit instruction sequence, backing out of calls if necessary.
#define _TimeSim_Rtn_Break      3       // Broke early from repeats of an instruction sequence (using _CallSeq)

typedef struct
{
    TimeSim_T_Instr
        cmd             :4,     // commands (see below)
        quitIfFalse     :1,     // Quit whole sequence if runWhileTrue() returns FALSE before being timed out.
        elapsedTime     :1;     // 'time.ends' for this instruction is read as elaped time (else it's absolute time)

    TimeSim_T_Repeats   
        repeats;                // If calling a child sequence, the number of times to reapeat thats sequence.
                                // If this parm <- 0 (or ommited) then the child runs once.
} TimeSim_S_Instr;

// Commands: Unless otherwise stated, advance to next instruction after executng this command
#define _TimeSim_Cmd_Null       0
#define _TimeSim_Cmd_SetAbs     1       // Force absolute time
#define _TimeSim_Cmd_SetStep    2       // Set timestep
#define _TimeSim_Cmd_Run        3       // Stay here; advance in timesteps until an exit condition, then next instruction
#define _TimeSim_Cmd_Hunt       4       // Stay here. Hunt for the TRUE->FALSE change in quitIfFalse() (which must have no internal states). Then next instruction
#define _TimeSim_Cmd_InNoTime   5       // Do whatever, do not advance time. (but goto next instruction, of course)
#define _TimeSim_Cmd_CallSeq    6       // Call another sequence, one or more times.
#define _TimeSim_Cmd_End        7       // Quit sequence

#define _TimeSim_Flag_ContinueDespiteFalse  0
#define _TimeSim_Flag_QuitIfFalse           1

#define _TimeSim_Flag_TimeoutIsAbsolute  0
#define _TimeSim_Flag_TimeoutIsElapsed   1

typedef BOOL TimeSim_T_RunFuncRtn;
typedef void TimeSim_T_RunFuncParm;

typedef struct
{
    TimeSim_S_Instr
                instr;      // Command and maybe flags
    union
    {                       // A time, which may be...
        T_Time  ends,       // when to end this step, unless runWhileTrue() returns FALSE 1st.
                abs,        // an absolute time to set SysTime to.
                step,       // the timestep until further notice.
                maxSteps;   // When calling another sequence, the max steps we allow to execute.
    } time;

    union                   // May run either of these...
    {
        TimeSim_T_RunFuncRtn (*whileTrue)(TimeSim_T_RunFuncParm);   // ... this function, until FALSE or 'ends', depending on flags.

        void const *childSeq;                                       // ... or this sequence, which returns to it's caller when done.
                                                                    // Note: Must be void because we haven't defined 'TimeSim_S_Sequence' yet (circular)
    } runThis;    
                            // Call this upon leaving this step. Usually a test & report function.
    void (*callWhenDone)(BOOL timeoutForcedQuit);
} S_TimeSimInstr;

#define _TimeSim_NoReportFunc 0     // If there's no report function then use this for clarity

typedef struct
{
    S_TimeSimInstr const *instrs;                   // The instructions which comprise the test sequence...
    TimeSim_T_InstrIdx   numInstrs;                 // ...and how many
    T_Timer              initialTimeStep;           // The starting timestep (which may be changed with '_TimeSim_Cmd_SetStep'.
                                                    // In each 'S_TimeSimInstr' 'runWhileTrue' is run inside this shell (if
                                                    // it exists). runShell() usually returns result of runFunc().
    BOOL(*runShell)( TimeSim_T_RunFuncRtn(*runFunc)(TimeSim_T_RunFuncParm) );
    BOOL(*bootShell)(void);                         // If this exists, it is called before TimeSim_ runs an instruction list.
} TimeSim_S_Sequence;

#define _TimeSim_NullRunShell 0     // If there's no 'runShell()' then use this for clarity

typedef struct
{
    TimeSim_T_InstrIdx
                instrIdx;           // Index to current instruntion (in the instruction list)
    T_Time      timeStep,           // the increment of fake time.
                instrStart;         // When entered latest instruction
    TimeSim_T_Repeats
                rptCnt;             // When repeating a child sequence, this counts thru the repeats
} S_Engine;

PUBLIC void          timeSim_Init(TimeSim_S_Sequence const *seq);
PUBLIC void          timeSim_NewSeq(S_Engine *eng, TimeSim_S_Sequence const *seq, BOOL resetTime);
PUBLIC TimeSim_T_Rtn timeSim_MultiStep(TimeSim_S_Sequence const *seq, U32 maxSteps, BOOL resetTime);
PUBLIC T_Time        timeSim_SinceInstrStart(void);

#define _TimeSim_RetainTime FALSE
#define _TimeSim_ResetTime  TRUE


#endif // TDD_TIMESIM_CORE_H

// ----------------------------- eof ----------------------------------------
