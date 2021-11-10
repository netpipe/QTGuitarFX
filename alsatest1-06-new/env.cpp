#include "env.h"

env::env()
{
    t=0;
    state=ENV_STATE_IDLE;
    prevValue=0;
    att_s=2205;
    sus_s=441000;
    dec_s=13230;
    rel_s=30870;
}
void env::UpdateAtt(long att)
{
    att_s=att;
}
void env::UpdateSus(long sus)
{
    sus_s=sus;
}
void env::UpdateDec(long dec)
{
    dec_s=dec;
}
void env::UpdateRel(long rel)
{
    rel_s=rel;
}
void env::ResetState(int newState)
{
    state=newState;
    t=0;
}
float env::newValue()
{
    t++;
    if (state==ENV_STATE_ATTACK)
    {
        if ((prevValue>=1)||(t>=att_s))
        {
            state=ENV_STATE_SUSTAIN;
            t=0;
            prevValue=1;
            return 1.0;
        }
        prevValue=prevValue+2*att_s/((t+att_s)*(t-1+att_s));
        return prevValue;
    }
    if (state==ENV_STATE_SUSTAIN)
    {
        if (t>=sus_s)
        {
            state=ENV_STATE_DECAY;
            t=0;
            return 1.0;
        }
        prevValue=1;
        return 1;
    }
    if (state==ENV_STATE_DECAY)
    {
        if (t>=(dec_s-(dec_s/5)))
        {
            if (dec_s==0)
            {
                return 1;
                // if no decay (dec==0) then sustain until you receive a note off and pass to the release state
            }
            state=ENV_STATE_RELEASE;
            t=0;
        }
        else
        {
            prevValue=prevValue-(t-2)/(dec_s*dec_s);
            return prevValue;
        }
    }
    if (state==ENV_STATE_RELEASE)
    {
        if ((t>=rel_s)||(prevValue<=0))
        {
            state=ENV_STATE_IDLE;
            t=0;
            prevValue=0;
            return 0;
        }
        else
        {
            prevValue=prevValue-(2*rel_s)/((t+rel_s)*(t-1+rel_s));
            return prevValue;
        }
    }
}
