#ifndef ENV_H
#define ENV_H

#define ENV_STATE_ATTACK    1
#define ENV_STATE_SUSTAIN   2
#define ENV_STATE_DECAY     3
#define ENV_STATE_RELEASE   4
#define ENV_STATE_IDLE      0

class env
{
private:
    long t;
    float att_s;
    float sus_s;
    float dec_s;
    float rel_s;
    float prevValue;
public:
    int state;
    env();
    void ResetState(int newState);
    void UpdateAtt(long att);
    void UpdateSus( long sus);
    void UpdateDec( long dec);
    void UpdateRel( long rel);
    float newValue();
};

#endif // ENV_H
