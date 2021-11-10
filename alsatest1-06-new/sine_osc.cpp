#include "sine_osc.h"
#include "math.h"

Sine_OSC::Sine_OSC()
{
    prevFreq=0;
    prev_x=1;
    prev_y=0;
    //sr=44100.0f;
    _pi=3.14159265358979323846;
}
float Sine_OSC::newSample(float f, float sr)
{
    if (f!=prevFreq)
    {//compute factor
        float a=2*_pi*f/sr;
        factor1=sin(a);
        factor2=cos(a);
        prevFreq=f;
    }

    float x=factor2*prev_x+factor1*prev_y;
    float y=-factor1*prev_x+factor2*prev_y;
    prev_x=x;
    prev_y=y;
    return y;
}

