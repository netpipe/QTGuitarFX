#ifndef SINE_OSC_H
#define SINE_OSC_H

namespace oscil {class Sine_OSC;}
class Sine_OSC
{
public:
    Sine_OSC();
    float newSample(float f, float sr);
private:
    float factor1;
    float factor2;
    //float sr;
    float prev_x;
    float prev_y;
    float prevFreq;
    double _pi;
};

#endif // SINE_OSC_H
