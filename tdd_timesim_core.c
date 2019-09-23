/* -----------------------------------------------------------------
|
|   This simulates the advance of time ('SysTime') on the Unity TDD platform for
|   those modules which use SysTime (systime.c).
|
|   Public:
|      timeSim_Init()
|      timeSim_NewSeq()
|      doOneInstr()
|      timeSim_MultiStep()
|      timeSim_SinceInstrStart()
|
|   $Id $
|
|   $Log $
|
--------------------------------------------------------------------- */

#include "GenericTypeDefs.h"
#include "tdd_timesim_core.h"
#include "systime.h"
#include <stdio.h>

// For the report function callWhenDone()
#define _QuitBeforeTimeout FALSE
#define _TimeoutForcedQuit TRUE

PRIVATE S_Engine *currEngine;

#define _RptCntNever 0xFF   // A child sequenmce has not been executed, even once.

/* ----------------------------------- doOneInstr ----------------------------------

    Advance 1 instruction through the 'seq'uence.
*/

PRIVATE TimeSim_T_Rtn doOneInstr(S_Engine *eng, TimeSim_S_Sequence const *seq)
{
    S_TimeSimInstr const    *nowAt;
    BOOL                    nextInstr, rtn;

    if(eng->instrIdx >= seq->numInstrs)                         // Index to current instruction is out of range?
    {
        return _TimeSim_Rtn_Failed;                             // Oh dear.. how did that happen?
    }
    else                                                        // otherwise we are reading within 'seq'
    {
        nowAt = &seq->instrs[eng->instrIdx];                    // Cache the step we are currently at.

        // By default, for most commands, we advance to the next instruction/step
        // Except in '_TimeSim_Cmd_Step' when we are excecuting a shell for some (simulated)
        // time before moving on.
        nextInstr = TRUE;                                       // Unless not so (in _TimeSim_Cmd_Step)

        switch(nowAt->instr.cmd)                                // And the command in that step is.....
        {
            case _TimeSim_Cmd_Null:
                break;

            case _TimeSim_Cmd_SetAbs:
                SetSysTime(nowAt->time.abs);                    // --- Force absolute time
                break;

            case _TimeSim_Cmd_SetStep:
                eng->timeStep = nowAt->time.step;               // --- Set the timestep
                break;

            case _TimeSim_Cmd_Run:                              // --- Run one timestep, but remain on the current instruction
                AddToSysTime(eng->timeStep);                                // Advance time

                if(nowAt->instr.elapsedTime                                 // At end-time for this instruction? Which is read as either...
                    ? Elapsed(&eng->instrStart) >= nowAt->time.ends         // time elapsed since start of this instruction or,
                    : _Now() >= nowAt->time.ends)                           // absolute time (since simulator started)
                {
                    if(nowAt->callWhenDone)                                 // There's a report function
                        { nowAt->callWhenDone(_TimeoutForcedQuit); }        // then run it
                }
                else if(nowAt->runThis.whileTrue)                           // else remain in this instruction. There's a function to run?
                {
                    if(!(seq->runShell ?                                    // Run it! (inside shell if this exists) Returned FALSE?
                            seq->runShell(nowAt->runThis.whileTrue):        // ... either under shell
                            nowAt->runThis.whileTrue()))                    // ...or naked
                    {
                        if(nowAt->callWhenDone)                             // FALSE-> Time to report? (and maybe quit)
                            { nowAt->callWhenDone(_QuitBeforeTimeout); }    // then do so.

                        if(nowAt->instr.quitIfFalse)                        // Says quit whole sequence if function return false?
                        {                                                   // (before being timed out)?
                            return _TimeSim_Rtn_Break;                      // then we are done.
                        }
                    }
                    else                                                    // else function returned TRUE
                    {
                        nextInstr = FALSE;                                  // so we do NOT advance to the next instruction
                    }
                }
                break;

            case _TimeSim_Cmd_Hunt:                             // ---  Hunt forward until a terminating condition (TBD)
                break;

            case _TimeSim_Cmd_InNoTime:                         // --- Do something once in zero time
                if(nowAt->runThis.whileTrue)                                // There's a function to run? (there should be, otherwise why bother with this cmd)
                {
                    //rtn = seq->runShell ?                                 // Run it! (inside shell if this exists) Returned FALSE?
                    //        seq->runShell(nowAt->runThis.whileTrue):      // ... either under shell
                    //        nowAt->runThis.whileTrue();                   // ...or naked
                    rtn = nowAt->runThis.whileTrue();

                    if(nowAt->callWhenDone)                                 // Done!. Now report?
                        { nowAt->callWhenDone(_QuitBeforeTimeout); }        // then do so.

                    if(rtn == FALSE && nowAt->instr.quitIfFalse)            // Says break from sequence if function returned false?
                    {
                        return _TimeSim_Rtn_Break;                          // then we are done.
                    }
                }
                break;
            
            case _TimeSim_Cmd_CallSeq:                          // --- Call another sequence; which will return control to this one.
                
                if(nowAt->runThis.childSeq)                     // There's a child sequence to execute?
                {
                    if(nowAt->instr.repeats > 1 &&              // This child sequence gets repeated? AND
                       eng->rptCnt == _RptCntNever )            // it has not been executed at all yet.
                        { eng->rptCnt = nowAt->instr.repeats; } // then initialise the repeat counter to the number of repeats.

                    rtn = timeSim_MultiStep(                    // Run child sequence. No more than 'maxSteps' iterations. Keep SysTime running.
                        nowAt->runThis.childSeq, nowAt->time.maxSteps, _TimeSim_RetainTime);
                    
                    currEngine = eng;                           // Returned from child. So reconnect to the parent's 'S_Engine'.

                    if( rtn == _TimeSim_Rtn_Failed )            // Failed?...
                    {
                        eng->rptCnt = _RptCntNever;             // so reset the to 'Never' for any future '_Call' inctruction

                        if(nowAt->callWhenDone)                 // Report on leaving 'Call'?
                            { nowAt->callWhenDone(_QuitBeforeTimeout); }    // then do so.

                        return _TimeSim_Rtn_Failed;             // Failed? ... then will back all the way out
                    }                                           // but reset child repeat counter (above) in case we don't actually do that.
                    if( rtn == _TimeSim_Rtn_Break )             // Failed?...
                    {
                        eng->rptCnt = _RptCntNever;             // so reset the to 'Never' for any future '_Call' inctruction

                        if(nowAt->callWhenDone)                 // Report on leaving 'Call'?
                            { nowAt->callWhenDone(_QuitBeforeTimeout); }    // then do so.
                                                                // and will 'Continue' (at bottom)
                    }                                           // but reset child repeat counter (above) in case we don't actually do that.
                    else                                        // else child ran OK.
                    {
                        if(nowAt->instr.repeats > 1)            // This child is repeated at least twice?
                        {
                            if(eng->rptCnt == _TimeSim_Repeats_Unlimited)   // Unlimited repeats?
                            {
                                nextInstr = FALSE;              // and we stay on this instruction (to exceute the child again)
                            }
                            else if(eng->rptCnt > 1)             // else Needs to run at least once more?
                            {
                                eng->rptCnt--;                  // then decrement count
                                nextInstr = FALSE;              // and we stay on this instruction (to exceute the child again)
                            }
                            else                                // else repeats are done.
                            {
                                eng->rptCnt = _RptCntNever;     // so reset the to 'Never' for any future '_Call' inctruction
                            }                                   // and will advance to next instruction (because 'nextInstr' <- TRUE)
                        }
                        if(nextInstr && nowAt->callWhenDone)    // Leaving 'Call'? AND report on doing so?
                        {
                            nowAt->callWhenDone(_QuitBeforeTimeout);    // then report
                        }
                    }
                    // ---- Yup! I know the logic above could be condensed. But I want to be pretty explicit above what's happening here.
                }
                break;

            case _TimeSim_Cmd_End:                              // --- To quit (before the end of the instruction sequence)
                return _TimeSim_Rtn_Done;                       //     (becuase we may add skips/loops later)

            default:                                            // --- In case user makes mistakes in instruction table.
                return _TimeSim_Rtn_Failed;

        } // switch(nowAt->instr.cmd)

        if(nextInstr)                                           // Advance to next instruction?
        {
            eng->instrStart = _Now();                           // Mark the time we entered the new instruction.

            if(++eng->instrIdx >= seq->numInstrs)               // Current instruction was the last?
            {
                return _TimeSim_Rtn_Done;                       // then we are done
            }
        }

        // Didn't return 'Done' or 'Failed' above, so continue.
        return _TimeSim_Rtn_Continue;

    } //switch(seq->steps[eng->nowAt])
} // doOneInstr()



/* ----------------------------------- timeSim_Init ------------------------------

    Reset the whole simulator. This zeros Time. Also boots the shell, if there's a
    boot function.
*/

PUBLIC void timeSim_Init(TimeSim_S_Sequence const *seq)
{
    ZeroSysTime();
    if(seq->bootShell) { seq->bootShell(); }    // If there is a boot function for the shell then run it.
}


/* ----------------------------------- timeSim_NewSeq ------------------------------

    Reset for a new 'seq'uence. Do not zero time if the thing you are testing
    has already 'marked' the current time outside the Sim.
*/

PUBLIC void timeSim_NewSeq(S_Engine *eng, TimeSim_S_Sequence const *seq, BOOL resetTime)
{
    if(resetTime)
    { 
        ZeroSysTime();
        if(seq->bootShell) { seq->bootShell(); }
    }

    eng->instrIdx =  0;                      // 1st step in sequence
    eng->timeStep = seq->initialTimeStep;    // and timestep is this.
    eng->instrStart = _Now();
    eng->rptCnt = _RptCntNever;              // Init the child sequence repeat counter to 'never repeated'.
}



/* ----------------------------------- timeSim_MultiStep ----------------------------------

    Run up to 'maxSteps' total of 'seq'. Return the '_Rtn_' code of the last step.
    So, if 'rtn' was '_TimeSim_Rtn_Continue', then we ran out of steps, otherwise
    '_Done' or '_Failed'.
*/


PUBLIC TimeSim_T_Rtn timeSim_MultiStep(TimeSim_S_Sequence const *seq, U32 maxSteps, BOOL resetTime)
{
   U32 stepCnt;
   TimeSim_T_Rtn rtn;
   S_Engine eng;

   currEngine = &eng;
   timeSim_NewSeq(&eng, seq, resetTime);

   stepCnt = 0;

   while( (rtn = doOneInstr(&eng, seq)) == _TimeSim_Rtn_Continue &&
           stepCnt < maxSteps)
                { stepCnt++; }
   return rtn;
}



/* ----------------------------------- timeSim_SinceInstrStart ----------------------------------

   Return since the start of the current ofthe latest instruction (which is presumably a multi-step
   otherwise why would you use this).
*/

PUBLIC T_Time timeSim_SinceInstrStart(void)
{
    return Elapsed(&(currEngine->instrStart));
}





// ----------------------------- eof ----------------------------------------