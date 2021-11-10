#include <iostream>
#include <stdio.h>
#include <jack/jack.h>
#include <jack/types.h>
#include <jack/midiport.h>
#include <handleMIDI.cpp>


bool useJack=true;
// FX - Globals
long SN=0;//Sample Number
long SamplingRate=44100;
jack_client_t* client;
// Main output//
long Volume=32000;
long iVolume=32000;
long outVolume=0;
long inVolume=0;
long previusSampleMagnitude=0;
long previusInputSampleMagnitude=0;
bool ascending = false;
bool iascending = false;
float MedVal =0;//used by compressor
bool First_Run = true;
//Synth-Tuner
float CurrentFreq=0;
///////////////////////////////////////////////////////////////////////////
//  Looper Params - Global
///////////////////////////////////////////////////////////////////////////
bool Enabled_Looper=false;
float Rec_Buffer  [600*2*44100/*52920000*/];//600*2*44100;
float Play_Buffer [600*2*44100/*52920000*/];//600*2*44100;
Looper looper(SamplingRate,Rec_Buffer, Play_Buffer);

///////////////////////////////////////////////////////////////////////////
//  FFT Params - Global
///////////////////////////////////////////////////////////////////////////
const long FFTSize =10000;
const long N_FFT=10000;
const long Kmax_FFT=256;
const long CosOffset_FFT=2500;
float FFTBufferL[FFTSize];
float FFTBufferR[FFTSize];
float FFTMagnitudeL[Kmax_FFT];
float FFTPhaseL[Kmax_FFT];
float FFTMagnitudeR[Kmax_FFT];
float FFTPhaseR[Kmax_FFT];
float LUT[FFTSize];
float FFTFrequency[Kmax_FFT];
float AFFTL[Kmax_FFT];
float BFFTL[Kmax_FFT];
float AFFTR[Kmax_FFT];
float BFFTR[Kmax_FFT];
int kFFT, jFFT, qFFT, iFFT;
float new_valFFTL, old_valFFTL, diffFFTL,new_valFFTR, old_valFFTR, diffFFTR;
///////////////////////////////////////////////////////////////////////////
//  Synth Params - Global                                                //
///////////////////////////////////////////////////////////////////////////
bool Enabled_Synth=false;
int notes[36]={19,20,21,22,24,25,27,28,30,32,33,35,37,40,42,45,47,50,53,56,59,63,67,71,75,79,84,89,94,100,106,112,118,125,132,139};
const int Polyphony_Synth=9;
float PlayedFreq_Synth[Polyphony_Synth];
float PlayedMagn_Synth[Polyphony_Synth];
Sine_OSC oscGT[Polyphony_Synth];
//char *note_names[36]={"A","B"};
float note_freq[36]={};
float Threshold_Synth=0.006;
float LoThreshold_Synth=0.003;
float LowOct_Synth=0.3;
float HiOct_Synth=0.3;
float MidOct_Synth=0.5;
float VLOct_Synth=0.2;
float SinMag_Synth=0.8;
float SqrMag_Synth=0.05;
float TriMag_Synth=0.5;
float SawMag_Synth=0.05;
float Dry_Synth=0.0;
float Wet_Synth=0.9;
////////////////////////////////////////////////////////////////////////
//  Noise Filter Params - Globals                                     //
//       - an implementation of the moving average filter             //
//         against white noise           !!!AMAZING!!!                //
////////////////////////////////////////////////////////////////////////
bool Enable_Filter=false;
float Filter_Buffer[26];//13-point filter sample needed for recursive calculation * 2 channels
int Points_Filter= 13; //[5-31] must be an odd number
float Gain_Filter = 1;
//lets just start with a simple 7-point non-variable length filter
bool FirstRun_Filter = true;
int RunCount_Filter = 0;
float sum_previous_L=0;
float x_previous_L=0;
float sum_previous_R=0;
float x_previous_R=0;
////////////////////////////////////////////////////////////////////////
//  Noise Filter Params - Globals                                     //
//       - an implementation of the moving average filter             //
//         against white noise           !!!AMAZING!!!                //
////////////////////////////////////////////////////////////////////////
bool Enable_OFilter=false;
float Filter_OBuffer[26];//13-point filter sample needed for recursive calculation * 2 channels
int Points_OFilter= 13; //[5-31] must be an odd number
float Gain_OFilter = 1;
//lets just start with a simple 7-point non-variable length filter
bool FirstRun_OFilter = true;
int RunCount_OFilter = 0;
float sum_previous_LO=0;
float x_previous_LO=0;
float sum_previous_RO=0;
float x_previous_RO=0;
////////////////////////////////////////////////////////////////////////
//  Distortion Params - Globals                                       //
////////////////////////////////////////////////////////////////////////
bool Enabled_Dist = false;
float GainFactor_Dist = 1.5;
float Threshold_Dist = 4000;
float ClippingFactor_Dist=3; //[1-1000]
bool asymmetrical_Dist = false;
////////////////////////////////////////////////////////////////////////
//  Tremolo    Params - Globals                                       //
////////////////////////////////////////////////////////////////////////
bool Enabled_Tr = false;
float Depth_Tr = 0.3;
float Period_Tr= 0.3; //sec
float Stereo_Tr=0;
Sine_OSC TrOSC_L;
Sine_OSC TrOSC_R;
////////////////////////////////////////////////////////////////////////
//  Delay      Params - Globals                                       //
////////////////////////////////////////////////////////////////////////
bool Enabled_D = false;
const float Max_Delay_Time= 5; //(sec)
float DTime_D= 0.3; //sec
float FeedBack_D= 0.3;
float Mix_D=0.3;
float *Delay_Buffer;
////////////////////////////////////////////////////////////////////////
//  Vibrato    Params - Globals                                       //
////////////////////////////////////////////////////////////////////////
bool Enabled_Vib= false;
float *Vibrato_Buffer;
float Depth_Vib=0.0005; //sec [min=0.0001, max=0.001]
float Speed_Vib=3; //Hz [0.1-50] - but i think it works better in [0.2-20]
float Wet_Vib=0.5; //[0-1]
float Dry_Vib=0.5; //[0-1]
float FeedBack_Vib=0; // [0-1]
float Stereo_Vib=0.50;
Sine_OSC VibOSC;
////////////////////////////////////////////////////////////////////////
//  Pitch Shifter  Params - Globals                                   //
////////////////////////////////////////////////////////////////////////
bool Enabled_Sh= false;
const long lMaxD= 5000;
float Shifter_B2 [2*lMaxD];
float *Shifter_Buffer;
float Shift_Sh=0.01;  // []
float MaxD_Sh=500;  // max delay - samples [10000]
float Wet_Sh=0.5;
float Dry_Sh=0.5;
float FeedBack_Sh=0;
////////////////////////////////////////////////////////////////////////
//  Chorus     Params - Globals                                       //
////////////////////////////////////////////////////////////////////////
bool Enabled_Ch= false;
float *Chorus_Buffer;
float Depth_Ch=50; //samples [1-500]
float Speed_Ch=3; //Hz [0.1-50] - but i think it works better in [0.2-20]
float Wet_Ch=0.5; //[0-1]
float Dry_Ch=0.5; //[0-1]
float FeedBack_Ch=0; // [0-1]
bool asc_Ch = false;
////////////////////////////////////////////////////////////////////////
//  Compr  Params - Globals                                           //
////////////////////////////////////////////////////////////////////////
//no lookahead though...
bool Enabled_Comp= false;
int BufferLen_Comp = 303;
float Comp_Buffer[606];
float GateThresh_Comp =0.05;// [0-1] % must be < LimiterThresh_Comp
float GateFactor_Comp=0.60; //[0-1] %
float LimiterThresh_Comp=0.50; //[0-1] % must be > GateThresh_Comp
float LimiterFactor_Comp=0.80; //[0-1] %
float Compression_Comp = 0.30; //  [0-1] %
float outGain_Comp = 0.2; //[1-10]

double tatt=0.0001; //attack time - sec
double trel=0.003; //release time - sec

double att=0;
double rel=0;

bool FirstRun_Comp = true;
int RunCount_Comp = 0;
double sum_previous_Comp=0;
double x_previous_Comp=0;
double env=0;
////////////////////////////////////////////////////////////////////////
//  Phaser  Params - Globals                                          //
////////////////////////////////////////////////////////////////////////
// 4 stages all-pass  filter, feedback, gain and sweep...
bool Enabled_Ph=false;
float Dry_Ph = 0.5; //%
float Wet_Ph = 0.5; //%
float SweepRate_Ph = 1.5; //Hz best in range [1-30]
float SweepDepth_Ph = 5 ; //Octaves
float FeedBack_Ph=0.4;
float BaseFreq_Ph=500; //hz [1760-7018] !!!
float OffsetFreq_Ph=7000;
float Stereo_Ph=0;

double lfoInc=0;
double lfoPhase=0;
double lfoPhaseR=0;
double dmin=0;
double dmax=0;

float wp=0;
float minWp=0;
float maxWp=0;
float step=0;
float currentStep=0;
float freqRange=0;

float yL_ph=0;
float y1L_ph =0;
float y2L_ph =0;
float y3L_ph =0;
float y4L_ph =0;
float y5L_ph =0;
float y6L_ph =0;
float x1L_ph =0;
float x2L_ph =0;
float x3L_ph =0;
float x4L_ph =0;
float x5L_ph =0;
float x6L_ph =0;
float yR_ph=0;
float y1R_ph =0;
float y2R_ph =0;
float y3R_ph =0;
float y4R_ph =0;
float y5R_ph =0;
float y6R_ph =0;
float x1R_ph =0;
float x2R_ph =0;
float x3R_ph =0;
float x4R_ph =0;
float x5R_ph =0;
float x6R_ph =0;
////////////////////////////////////////////////////////////////////////
//  Wah     Params - Globals                                          //
////////////////////////////////////////////////////////////////////////
// 2-tap bi-quad band-pass filter with sawtooth lfo
bool Enabled_Wah=false;
float Dry_Wah = 0.09; //%
float Wet_Wah = 0.09; //%
float SweepRate_Wah =5; //Hz best in range [0-30]
float BaseFreq_Wah=500; //hz [1760-7018] !!!
float OffsetFreq_Wah=7000;

double Inc_Wah=0;
double Phase_Wah=0;
double Phase_Wah_R=_pi/2;
double Stereo_Wah=0;
double dmin_Wah=0;
double dmax_Wah=0;
double R_Wah=0.97;
int RunCount_Wah=0;

float yL_wah=0;
float y1L_wah=0;
float y2L_wah =0;
float x1L_wah =0;
float x2L_wah =0;
float yR_wah=0;
float y1R_wah=0;
float y2R_wah =0;
float x1R_wah =0;
float x2R_wah =0;
////////////////////////////////////////////////////////////////////////
//  EQ      Params - Globals                                          //
////////////////////////////////////////////////////////////////////////
// We implement 1 LoW-Pass, 5 Band-Pass and a High-Pass Filter running in parallel
////////////////////////////////////////////////////////////////////////
//General Params
bool Enabled_EQ=false;
float LP_Gain=1;
float BP1_Gain=1;
float BP2_Gain=1;
float BP3_Gain=1;
float BP4_Gain=1;
float BP5_Gain=1;
float HP_Gain=1;
//LP IIR 2nd order - cutoff 63Hz
double a0_LP=2.00149E-05;
double a1_LP=4.00298E-05;
double a2_LP=2.00149E-05;
double b1_LP=-1.98731;
double b2_LP=0.987386;
double y_LP=0;
double y1_LP=0;
double y2_LP =0;
double x1_LP =0;
double x2_LP =0;
//1st BP IIR 2nd order - center=250hz, band=400hz
double a0_BP1=0.0298463;
double a1_BP1=0;
double a2_BP1=-0.0298463;
double b1_BP1=-1.94413;
double b2_BP1=0.944574;
double y_BP1=0;
double y1_BP1=0;
double y2_BP1 =0;
double x1_BP1 =0;
double x2_BP1 =0;
//2nd BP IIR 2nd order - center=1000hz, band=1000hz
double a0_BP2=0.0686301;
double a1_BP2=0;
double a2_BP2=-0.0686301;
double b1_BP2=-1.85257;
double b2_BP2=0.866788;
double y_BP2=0;
double y1_BP2=0;
double y2_BP2 =0;
double x1_BP2 =0;
double x2_BP2 =0;
//3rd BP IIR 2nd order - center=4000hz, band=4000hz
double a0_BP3=0.232166;
double a1_BP3=0;
double a2_BP3=-0.232166;
double b1_BP3=-1.35713;
double b2_BP3=0.546882;
double y_BP3=0;
double y1_BP3=0;
double y2_BP3 =0;
double x1_BP3 =0;
double x2_BP3 =0;
//4th BP IIR 2nd order - center=8000hz, band=4000hz
double a0_BP4=0.227051;
double a1_BP4=0;
double a2_BP4=-0.227051;
double b1_BP4=-0.673396;
double b2_BP4=0.546882;
double y_BP4=0;
double y1_BP4=0;
double y2_BP4 =0;
double x1_BP4 =0;
double x2_BP4 =0;
//5th BP IIR 2nd order - center=12000hz, band=4000hz
double a0_BP5=0.226604;
double a1_BP5=0;
double a2_BP5=-0.226604;
double b1_BP5=0.223193;
double b2_BP5=0.546882;
double y_BP5=0;
double y1_BP5=0;
double y2_BP5 =0;
double x1_BP5 =0;
double x2_BP5 =0;
//HP IIR 2nd order - cutoff=16000hz
double a0_HP=0.113569;
double a1_HP=-0.227138;
double a2_HP=0.113569;
double b1_HP=0.847139;
double b2_HP=0.301416;
double y_HP=0;
double y1_HP=0;
double y2_HP =0;
double x1_HP =0;
double x2_HP =0;
////////////////////////////////////////////////////////////////////////
//  Reverb   Params - Globals                                         //
////////////////////////////////////////////////////////////////////////
bool Enabled_Rv= false;
float Rv_BufferSize=44100;
float *Rv1_Buffer;
float *Rv2_Buffer;
float *Rv3_Buffer;
float *Rv4_Buffer;
float *Rv5_Buffer;
float *Rv6_Buffer;
float *RvFb1_Buffer;
float *RvFb2_Buffer;
float *RvFb3_Buffer;
float RvWet=0.50;
float RvDry=0.50;
float d1_Rv=400;
float d2_Rv=150;
float d3_Rv=1000;
float d4_Rv=350;
float d5_Rv=900;
float d6_Rv=300;
float g1=0.8;
float g2=0.8;
float g3=0.8;
float dFb1=300;
float dFb2=500;
float dFb3=400;
//Rotation angles
float fi1=_pi/3.3;
float cosfi1=0.999998644;
float sinfi1=0.00164658;
float fi2=_pi*1.8;
float cosfi2=0.995133498;
float sinfi2=0.098535891;
float fi3=_pi/0.9;
float cosfi3=0.998144739;
float sinfi3=0.060885803;
float fi4=_pi/9;
float cosfi4=0.999981442;
float sinfi4=0.006092311;
float fi5=_pi*1.1;
float cosfi5=0.99818164;
float sinfi5=0.060277687;
float fi6=_pi/1.9;
float cosfi6=0.999583623;
float sinfi6=0.028854487;
float th3d=_pi/9;
float costh3d=0.999981442;
float sinth3d=0.006092311;
float fi3d=_pi*1.3;
float cosfi3d=0.997460622;
float sinfi3d=0.07122013;
float ps3d=_pi/1.6;
float cosps3d=0.999981442;
float sinps3d=0.006092311;
///////////////////////////////////////////////////////////////////////////
//LP IIR 4th order - cutoff 1700Hz - used by FFT
///////////////////////////////////////////////////////////////////////////
double a0_FFT=0.000159709;
double a1_FFT=0.000638836;
double a2_FFT=0.000958254;
double a3_FFT=0.000638836;
double a4_FFT=0.000159709;
double b1_FFT=-3.36781;
double b2_FFT=4.29568;
double b3_FFT=-2.45539;
double b4_FFT=0.53007;
double yL_FFT=0;
double y1L_FFT=0;
double y2L_FFT =0;
double y3L_FFT =0;
double y4L_FFT =0;
double x1L_FFT =0;
double x2L_FFT =0;
double x3L_FFT =0;
double x4L_FFT =0;
double yR_FFT=0;
double y1R_FFT=0;
double y2R_FFT =0;
double y3R_FFT =0;
double y4R_FFT =0;
double x1R_FFT =0;
double x2R_FFT =0;
double x3R_FFT =0;
double x4R_FFT =0;
///////////////////////////////////////////////////////////////////////////

void Rotate2Df(float *y1, float *y2, float _x1,float _x2, float cosfi, float sinfi)
{
    float x1=_x1;
    float x2=_x2;
    *y1=cosfi*x1+sinfi*x2;
    *y2=sinfi*x1-cosfi*x2;
}
void Rotate2D(float *y1, float *y2, float _x1,float _x2, float fi)
{
    float x1=_x1;
    float x2=_x2;
    *y1=cos(fi)*x1+sin(fi)*x2;
    *y2=sin(fi)*x1-cos(fi)*x2;
}
void Rotate3Df(float *y1, float *y2,float *y3, float _x1,float _x2,float _x3, float costh, float sinth, float cosfi, float sinfi, float cosps, float sinps)
{
    float x1=_x1;
    float x2=_x2;
    float x3=_x3;
    *y1=costh*cosps*x1+(-cosfi*sinps+sinfi*sinth*cosps)*x2+(sinfi*sinps+cosfi*sinth*cosps)*x3;
    *y2=costh*sinps*x1+(cosfi*cosps+sinfi*sinth*sinps)*x2+(-sinfi*cosps+cosfi*sinth*sinps)*x3;
    *y3=-sinth*x1+sinfi*costh*x2+cosfi*costh*x3;
}
void Rotate3D(float *y1, float *y2,float *y3, float _x1,float _x2,float _x3, float th, float fi, float ps)
{
    float x1=_x1;
    float x2=_x2;
    float x3=_x3;
    *y1=cos(th)*cos(ps)*x1+(-cos(fi)*sin(ps)+sin(fi)*sin(th)*cos(ps))*x2+(sin(fi)*sin(ps)+cos(fi)*sin(th)*cos(ps))*x3;
    *y2=cos(th)*sin(ps)*x1+(cos(fi)*cos(ps)+sin(fi)*sin(th)*sin(ps))*x2+(-sin(fi)*cos(ps)+cos(fi)*sin(th)*sin(ps))*x3;
    *y3=-sin(th)*x1+sin(fi)*cos(th)*x2+cos(fi)*cos(th)*x3;
}
void Sample_Reverb(float *x_L, float *x_R)
{
    float sr = SamplingRate;
    long size = Rv_BufferSize;
    float y1,y2,y3;
    long dt,bp;
    float x2=0;

    //Left Channel
    float x1=*x_L;
    //Rotate2D(&y1,&y2,x1,x1,fi1);
    Rotate2Df(&y1,&y2,x1,x1,cosfi1, sinfi1);
    //first delay line
    //store one output in the delay line
    Rv1_Buffer[(SN%size)*2]=y2;
    //get the delayed sample
    dt=(SN%size)-d1_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    y2=Rv1_Buffer[bp*2];
    //Rotate2D(&x1,&x2,y1,y2,fi2);
    Rotate2Df(&x1,&x2,y1,y2,cosfi2, sinfi2);
    //second delay line
    //store one output in the delay line
    Rv2_Buffer[(SN%size)*2]=x2;
    //get the delayed sample
    dt=(SN%size)-d2_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    x2=Rv2_Buffer[bp*2];
    //Rotate2D(&y1,&y2,x1,x2,fi3);
    Rotate2Df(&y1,&y2,x1,x2,cosfi3,sinfi3);
    //third delay line
    //store one output in the delay line
    Rv3_Buffer[(SN%size)*2]=y2;
    //get the delayed sample
    dt=(SN%size)-d3_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    y2=Rv3_Buffer[bp*2];
    //Rotate2D(&x1,&x2,y1,y2,fi4);
    Rotate2Df(&x1,&x2,y1,y2,cosfi4,sinfi4);
    //fourth delay line
    //store one output in the delay line
    Rv4_Buffer[(SN%size)*2]=x2;
    //get the delayed sample
    dt=(SN%size)-d4_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    x2=Rv4_Buffer[bp*2];
    //Rotate2D(&y1,&y2,x1,x2,fi5);
    Rotate2Df(&y1,&y2,x1,x2,cosfi5, sinfi5);
    //fifth delay line
    //store one output in the delay line
    Rv5_Buffer[(SN%size)*2]=y2;
    //get the delayed sample
    dt=(SN%size)-d5_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    y2=Rv5_Buffer[bp*2];
    //Rotate2D(&x1,&x2,y1,y2,fi6);
    Rotate2Df(&x1,&x2,y1,y2,cosfi6, sinfi6);
    //fourth delay line
    //store one output in the delay line
    Rv6_Buffer[(SN%size)*2]=x2;
    //get the delayed sample
    dt=(SN%size)-d6_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    x2=Rv6_Buffer[bp*2];

    //Retrieve FeedBack
    //get the delayed samples from the feedback buffers
    bp=(SN%size)-dFb1;
    if (bp<0) bp+=size;
    if (bp>=size) bp-=size;
    y1=RvFb1_Buffer[bp*2];

    bp=(SN%size)-dFb2;
    if (bp<0) bp+=size;
    if (bp>=size) bp-=size;
    y2=RvFb2_Buffer[bp*2];

    bp=(SN%size)-dFb3;
    if (bp<0) bp+=size;
    if (bp>=size) bp-=size;
    y3=RvFb3_Buffer[bp*2];

    //Rotate the feedback
    //Rotate3D(&y1,&y2,&y3,y1,y2,y3,th3d,fi3d,ps3d);
    Rotate3Df(&y1,&y2,&y3,y1,y2,y3,costh3d,sinth3d,cosfi3d,sinfi3d,cosps3d,sinps3d);
    y1=g1*y1+x2;
    y2=g2*y2+x1;
    y3*=g3;
    //Put the results to FeedBack Buffer
    RvFb1_Buffer[(SN%size)*2]=y1;
    RvFb2_Buffer[(SN%size)*2]=y2;
    RvFb3_Buffer[(SN%size)*2]=y3;
    //Get the output
    *x_L= (*x_L)*RvDry + RvWet*(y1+y2)/2;

    //Right Channel
    x1=*x_R;
    //Rotate2D(&y1,&y2,x1,x1,fi1);
    Rotate2Df(&y1,&y2,x1,x1,cosfi1, sinfi1);
    //first delay line
    //store one output in the delay line
    Rv1_Buffer[(SN%size)*2+1]=y2;
    //get the delayed sample
    dt=(SN%size)-d1_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    y2=Rv1_Buffer[bp*2+1];
    //Rotate2D(&x1,&x2,y1,y2,fi2);
    Rotate2Df(&x1,&x2,y1,y2,cosfi2, sinfi2);
    //second delay line
    //store one output in the delay line
    Rv2_Buffer[(SN%size)*2+1]=x2;
    //get the delayed sample
    dt=(SN%size)-d2_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    x2=Rv2_Buffer[bp*2+1];
    //Rotate2D(&y1,&y2,x1,x2,fi3);
    Rotate2Df(&y1,&y2,x1,x2,cosfi3, sinfi3);
    //third delay line
    //store one output in the delay line
    Rv3_Buffer[(SN%size)*2+1]=y2;
    //get the delayed sample
    dt=(SN%size)-d3_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    y2=Rv3_Buffer[bp*2+1];
    //Rotate2D(&x1,&x2,y1,y2,fi4);
    Rotate2Df(&x1,&x2,y1,y2,cosfi4, sinfi4);
    //fourth delay line
    //store one output in the delay line
    Rv4_Buffer[(SN%size)*2+1]=x2;
    //get the delayed sample
    dt=(SN%size)-d4_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    x2=Rv4_Buffer[bp*2+1];
    //Rotate2D(&y1,&y2,x1,x2,fi5);
    Rotate2Df(&y1,&y2,x1,x2,cosfi5, sinfi5);
    //fifth delay line
    //store one output in the delay line
    Rv5_Buffer[(SN%size)*2+1]=y2;
    //get the delayed sample
    dt=(SN%size)-d5_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    y2=Rv5_Buffer[bp*2+1];
    //Rotate2D(&x1,&x2,y1,y2,fi6);
    Rotate2Df(&x1,&x2,y1,y2,cosfi6, sinfi6);
    //fourth delay line
    //store one output in the delay line
    Rv6_Buffer[(SN%size)*2+1]=x2;
    //get the delayed sample
    dt=(SN%size)-d6_Rv;
    bp=dt;
    if (dt<0) bp+=size;
    if (dt>=size) bp-=size;
    x2=Rv6_Buffer[bp*2+1];

    //Retrieve FeedBack
    //get the delayed samples from the feedback buffers
    bp=(SN%size)-dFb1;
    if (bp<0) bp+=size;
    if (bp>=size) bp-=size;
    y1=RvFb1_Buffer[bp*2+1];

    bp=(SN%size)-dFb2;
    if (bp<0) bp+=size;
    if (bp>=size) bp-=size;
    y2=RvFb2_Buffer[bp*2+1];

    bp=(SN%size)-dFb3;
    if (bp<0) bp+=size;
    if (bp>=size) bp-=size;
    y3=RvFb3_Buffer[bp*2+1];

    //Rotate the feedback
    //Rotate3D(&y1,&y2,&y3,y1,y2,y3,th3d,fi3d,ps3d);
    Rotate3Df(&y1,&y2,&y3,y1,y2,y3,costh3d,sinth3d,cosfi3d,sinfi3d,cosps3d,sinps3d);
    y1=g1*y1+x2;
    y2=g2*y2+x1;
    y3*=g3;

    //Put the results to FeedBack Buffer
    RvFb1_Buffer[(SN%size)*2+1]=y1;
    RvFb2_Buffer[(SN%size)*2+1]=y2;
    RvFb3_Buffer[(SN%size)*2+1]=y3;

    //Get the output
    *x_R= (*x_R)*RvDry+RvWet*(y1+y2)/2;
}

float cubic_interpolate( float y0, float y1, float y2, float y3, float mu ) {

   float a0, a1, a2, a3, mu2;

   mu2 = mu*mu;
   a0 = y3 - y2 - y0 + y1; //p
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return ( a0*mu*mu2 + a1*mu2 + a2*mu + a3 );
}

float Sample_addGain(float value, float GainFactor,float MaxGain)
{
    GainFactor =(GainFactor<MaxGain)?GainFactor:MaxGain;
    value*=GainFactor;
    return value;
}
double Sample_LP(float value)
{
    double x = (value);
    // Calculate difference equation
    y_LP= (a0_LP*x) + (a1_LP*x1_LP) + (a2_LP*x2_LP) - (b1_LP*y1_LP) - (b2_LP*y2_LP);
    x2_LP=x1_LP;
    x1_LP=x;
    y2_LP=y1_LP;
    y1_LP=y_LP;
    return y_LP;
}
double Sample_BP1(float value)
{
    double x = (value);
    // Calculate difference equation
    y_BP1= (a0_BP1*x) + (a1_BP1*x1_BP1) + (a2_BP1*x2_BP1) - (b1_BP1*y1_BP1) - (b2_BP1*y2_BP1);
    x2_BP1=x1_BP1;
    x1_BP1=x;
    y2_BP1=y1_BP1;
    y1_BP1=y_BP1;
    return y_BP1;
}
double Sample_BP2(float value)
{
    double x = (value);
    // Calculate difference equation
    y_BP2= (a0_BP2*x) + (a1_BP2*x1_BP2) + (a2_BP2*x2_BP2) - (b1_BP2*y1_BP2) - (b2_BP2*y2_BP2);
    x2_BP2=x1_BP2;
    x1_BP2=x;
    y2_BP2=y1_BP2;
    y1_BP2=y_BP2;
    return y_BP2;
}
double Sample_BP3(float value)
{
    double x = (value);
    // Calculate difference equation
    y_BP3= (a0_BP3*x) + (a1_BP3*x1_BP3) + (a2_BP3*x2_BP3) - (b1_BP3*y1_BP3) - (b2_BP3*y2_BP3);
    x2_BP3=x1_BP3;
    x1_BP3=x;
    y2_BP3=y1_BP3;
    y1_BP3=y_BP3;
    return y_BP3;
}
double Sample_BP4(float value)
{
    double x = (value);
    // Calculate difference equation
    y_BP4= (a0_BP4*x) + (a1_BP4*x1_BP4) + (a2_BP4*x2_BP4) - (b1_BP4*y1_BP4) - (b2_BP4*y2_BP4);
    x2_BP4=x1_BP4;
    x1_BP4=x;
    y2_BP4=y1_BP4;
    y1_BP4=y_BP4;
    return y_BP4;
}
double Sample_BP5(float value)
{
    double x = (value);
    // Calculate difference equation
    y_BP5= (a0_BP5*x) + (a1_BP5*x1_BP5) + (a2_BP5*x2_BP5) - (b1_BP5*y1_BP5) - (b2_BP5*y2_BP5);
    x2_BP5=x1_BP5;
    x1_BP5=x;
    y2_BP5=y1_BP5;
    y1_BP5=y_BP5;
    return y_BP5;
}
double Sample_HP(float value)
{
    double x = (value);
    // Calculate difference equation
    y_HP= (a0_HP*x) + (a1_HP*x1_HP) + (a2_HP*x2_HP) - (b1_HP*y1_HP) - (b2_HP*y2_HP);
    x2_HP=x1_HP;
    x1_HP=x;
    y2_HP=y1_HP;
    y1_HP=y_HP;
    return y_HP;
}
void Sample_EQ(float *value_L, float *value_R)
{
    double xL=*value_L;
    double xR=*value_R;

    double LPout_L=Sample_LP(xL);
    double BP1out_L=Sample_BP1(xL);
    double BP2out_L=Sample_BP2(xL);
    double BP3out_L=Sample_BP3(xL);
    double BP4out_L=Sample_BP4(xL);
    double BP5out_L=Sample_BP5(xL);
    double HPout_L=Sample_HP(xL);
    double LPout_R=Sample_LP(xR);
    double BP1out_R=Sample_BP1(xR);
    double BP2out_R=Sample_BP2(xR);
    double BP3out_R=Sample_BP3(xR);
    double BP4out_R=Sample_BP4(xR);
    double BP5out_R=Sample_BP5(xR);
    double HPout_R=Sample_HP(xR);

    //multiply by gains and add
    *value_L= (LP_Gain*LPout_L) + (BP1_Gain*BP1out_L) + (BP2_Gain*BP2out_L) + (BP3_Gain*BP3out_L) + (BP4_Gain*BP4out_L) + (BP5_Gain*BP5out_L) + (HP_Gain*HPout_L);
    *value_R= (LP_Gain*LPout_R) + (BP1_Gain*BP1out_R) + (BP2_Gain*BP2out_R) + (BP3_Gain*BP3out_R) + (BP4_Gain*BP4out_R) + (BP5_Gain*BP5out_R) + (HP_Gain*HPout_R);
}
void Sample_Wah(float *value_L, float *value_R)
{
    //calculate the Coifficient from frequency
    // sinusoid
    double theta = dmin_Wah + (dmax_Wah-dmin_Wah)* ((sin(Phase_Wah)+1)/2);
    double thetaR = dmin_Wah + (dmax_Wah-dmin_Wah)* ((sin(Phase_Wah_R)+1)/2);
    //double theta = _pi/8;
    // sawtooth - does nt work well though - sounds like repeating piano hits :)
    //double theta = (Phase_Wah/(2*_pi))*(dmax_Wah-dmin_Wah);
    Phase_Wah+=Inc_Wah;
    if( Phase_Wah >= _pi * 2.f ) Phase_Wah -= _pi * 2.f;
    Phase_Wah_R=Phase_Wah+Stereo_Wah;
    if( Phase_Wah_R >= _pi * 2.f ) Phase_Wah_R -= _pi * 2.f;
    double xL = (*value_L);
    double xR = (*value_R);


    //Left Channel
    //xL=xL*(1-FeedBack_Wah) + FeedBack_Wah*yL_wah;//add the feedback
    // Calculate difference equation
    //if (RunCount_Wah++>2)
        yL_wah=xL-x2L_wah+2*R_Wah*cos(theta)*y1L_wah-R_Wah*R_Wah*y2L_wah;
    x2L_wah=x1L_wah;
    x1L_wah=xL;
    y2L_wah=y1L_wah;
    y1L_wah=yL_wah;
    // Calculate wet and dry contributions
    *value_L= Dry_Wah* (*value_L)+ Wet_Wah*yL_wah;

    //Right Channel
    //xR=xR*(1-FeedBack_Wah)+FeedBack_Wah*yR_wah;//add the feedback
    // Calculate difference equation
    //if (RunCount_Wah>2)
        yR_wah=xR-x2R_wah+2*R_Wah*cos(thetaR)*y1R_wah-R_Wah*R_Wah*y2R_wah;
    x2R_wah=x1R_wah;
    x1R_wah=xR;
    y2R_wah=y1R_wah;
    y1R_wah=yR_wah;
    // Calculate wet and dry contributions
    *value_R= Dry_Wah* (*value_R)+ Wet_Wah*yR_wah;
}
void Sample_Phaser(float *value_L, float *value_R)
{
    //calculate the Coifficient from frequency
    double d = dmin + (dmax-dmin)* ((sin(lfoPhase)+1)/2);
    double dR = dmin + (dmax-dmin)* ((sin(lfoPhaseR)+1)/2);
    lfoPhase+=lfoInc;
    lfoPhaseR=lfoPhase+Stereo_Ph;
    if( lfoPhase >= _pi * 2.f ) lfoPhase -= _pi * 2.f;
    if( lfoPhaseR >= _pi * 2.f ) lfoPhaseR -= _pi * 2.f;

    double C =- (1.0 - d) / (1.0 + d);
    double CR =- (1.0 - dR) / (1.0 + dR);
    //double C =- (1.0 - wp) / (1.0 + wp);
    double xL = (*value_L);
    double xR = (*value_R);


    //Left Channel
    xL=xL*(1-FeedBack_Ph) + FeedBack_Ph*y4L_ph;//add the feedback
    // Calculate difference equation
    y1L_ph=C*(xL-y1L_ph)+x1L_ph;//1st all pass
    x1L_ph=xL;
    y2L_ph=C*(y1L_ph-y2L_ph)+x2L_ph;//2nd all pass
    x2L_ph=y1L_ph;
    y3L_ph=C*(y2L_ph-y3L_ph)+x3L_ph;//3rd all pass
    x3L_ph=y2L_ph;
    y4L_ph=C*(y3L_ph-y4L_ph)+x4L_ph;//4th all pass
    x4L_ph=y3L_ph;
    // Calculate wet and dry contributions
    *value_L= Dry_Ph* (*value_L)+ Wet_Ph*y4L_ph;
    // Update sweep

    //Right Channel
    xR=xR*(1-FeedBack_Ph)+FeedBack_Ph*y4R_ph;//add the feedback
    // Calculate difference equation
    y1R_ph=CR*(xR-y1R_ph)+x1R_ph;//1st all pass
    x1R_ph=xR;
    y2R_ph=CR*(y1R_ph-y2R_ph)+x2R_ph;//2nd all pass
    x2R_ph=y1R_ph;
    y3R_ph=CR*(y2R_ph-y3R_ph)+x3R_ph;//2nd all pass
    x3R_ph=y2R_ph;
    y4R_ph=CR*(y3R_ph-y4R_ph)+x4R_ph;//2nd all pass
    x4R_ph=y3R_ph;
    // Calculate wet and dry contributions
    *value_R= Dry_Ph* (*value_R)+ Wet_Ph*y4R_ph;
    // Update sweep


    /*wp *= currentStep;      // Apply step value
    if(wp > maxWp) {     // Exceed max Wp ?
        currentStep = 1.0 / step;
    }   else if (wp < minWp) {   // Exceed min Wp ?
        currentStep = step;
    }*/
}
void Sample_Comp(float *value_L, float *value_R, int CPoints)
{//implementation of the compressor with moving average ... lets see if it works :)
    double sum=0;
    double rms=0;
    double s=0;

    Comp_Buffer[(SN%CPoints)*2]=*value_L;
    Comp_Buffer[(SN%CPoints)*2+1]=*value_R;


    if (FirstRun_Comp)
    {
        if (RunCount_Comp<CPoints+1)
        {
            RunCount_Comp++;
            return;
            // do nothing - wait until we reach the FPoint-th sample to calculate the average
        }

    // calculate the first sum
        FirstRun_Comp=false;
        for (int i=CPoints-1;i>=0;i--)
        {
            s=0.5*Comp_Buffer[((SN-i)%CPoints)*2]+0.5*Comp_Buffer[((SN-i)%CPoints)*2+1];
            sum+= s*s;
        }
        sum_previous_Comp=sum;
        s=0.5*Comp_Buffer[((SN-CPoints+1)%CPoints)*2]+0.5*Comp_Buffer[((SN-CPoints+1)%CPoints)*2+1];
        x_previous_Comp=s*s;
        return;
    }

    //calculate the average for the y[n] - the output signal
    s=0.5*(*value_L)+0.5*(*value_R);
    sum = sum_previous_Comp + (s*s) - x_previous_Comp;
    rms=sqrt(sum/CPoints);
    sum_previous_Comp=sum;
    s=0.5*Comp_Buffer[((SN-CPoints+1)%CPoints)*2]+0.5*Comp_Buffer[((SN-CPoints+1)%CPoints)*2+1];
    x_previous_Comp=s*s;

    // dynamic selection: attack or release?
    bool attack=(rms > env)? true : false;
    double  theta = (rms > env)? att : rel;
    // smoothing with capacitor, envelope extraction...
    // here be aware of pIV denormal numbers glitch
    env = (1.0 - theta) * rms + theta * env;

    float coef=2000;
    if (useJack) coef = 0.1;

    //The Gate
    double  Ggain = 1.0;
    if (env < GateThresh_Comp*coef)
    {
        Ggain = Ggain - GateFactor_Comp*(GateThresh_Comp*coef - env) / (GateThresh_Comp*coef);
        // result - two hard kneed gated channels...
        *value_L= (*value_L) * Ggain;
        *value_R= (*value_R) * Ggain;
        return;
    }

    //The Compressor (maybe it is called expander...)
    double  Cgain = 1.0;
    if (env < LimiterThresh_Comp*coef)
    {
        Cgain = Cgain +  Compression_Comp*(LimiterThresh_Comp*coef- env) /(LimiterThresh_Comp*coef);
        // result - two hard kneed compressed channels...
        *value_L= (*value_L) * Cgain;
        *value_R= (*value_R) * Cgain;
        return;
    }


    //The limiter
    double  Lgain = 1.0;
    if (env > LimiterThresh_Comp*coef)
    {

        Lgain = Lgain - LimiterFactor_Comp*(env - LimiterThresh_Comp*coef) / (env);
        // result - two hard kneed limited channels...
        *value_L= (*value_L) * Lgain;
        *value_R= (*value_R) * Lgain;
        return;
    }
    return;

}
void Sample_Filter(float *value_L, float *value_R, int FPoints)
{//implementation of the moving average filter
    float sum_L=0;
    float sum_R=0;

    Filter_Buffer[(SN%FPoints)*2]=*value_L;
    Filter_Buffer[(SN%FPoints)*2+1]=*value_R;

    if (FirstRun_Filter)
    {
        if (RunCount_Filter<FPoints+1)
        {
            RunCount_Filter++;
            return;
            // do nothing - wait until we reach the FPoint-th sample to calculate the average
        }

    // calculate the first sum
        FirstRun_Filter=false;
        for (int i=FPoints-1;i>=0;i--)
        {
            sum_L += Filter_Buffer[((SN-i)%FPoints)*2];
            sum_R += Filter_Buffer[((SN-i)%FPoints)*2+1];
        }
        *value_L=sum_L/FPoints;
        *value_R=sum_R/FPoints;
        sum_previous_L=sum_L;
        sum_previous_R=sum_R;
        x_previous_L=Filter_Buffer[((SN-FPoints+1)%FPoints)*2];
        x_previous_R=Filter_Buffer[((SN-FPoints+1)%FPoints)*2+1];
        return;
    }

    //calculate the average for the y[n] - the output signal

    sum_L = sum_previous_L + (*value_L) - x_previous_L;
    sum_R = sum_previous_R + (*value_R) - x_previous_R;
    *value_L=sum_L/FPoints;
    *value_R=sum_R/FPoints;
    sum_previous_L=sum_L;
    sum_previous_R=sum_R;

    x_previous_L=Filter_Buffer[((SN-FPoints+1)%FPoints)*2];
    x_previous_R=Filter_Buffer[((SN-FPoints+1)%FPoints)*2+1];

    return;
}
void Sample_OFilter(float *value_L, float *value_R, int FPoints)
{//implementation of the moving average filter - For Output
    float sum_L=0;
    float sum_R=0;

    Filter_OBuffer[(SN%FPoints)*2]=*value_L;
    Filter_OBuffer[(SN%FPoints)*2+1]=*value_R;

    if (FirstRun_OFilter)
    {
        if (RunCount_OFilter<FPoints+1)
        {
            RunCount_OFilter++;
            return;
            // do nothing - wait until we reach the FPoint-th sample to calculate the average
        }

    // calculate the first sum
        FirstRun_OFilter=false;
        for (int i=FPoints-1;i>=0;i--)
        {
            sum_L += Filter_OBuffer[((SN-i)%FPoints)*2];
            sum_R += Filter_OBuffer[((SN-i)%FPoints)*2+1];
        }
        *value_L=sum_L/FPoints;
        *value_R=sum_R/FPoints;
        sum_previous_LO=sum_L;
        sum_previous_RO=sum_R;
        x_previous_LO=Filter_OBuffer[((SN-FPoints+1)%FPoints)*2];
        x_previous_RO=Filter_OBuffer[((SN-FPoints+1)%FPoints)*2+1];
        return;
    }

    //calculate the average for the y[n] - the output signal

    sum_L = sum_previous_LO + (*value_L) - x_previous_LO;
    sum_R = sum_previous_RO + (*value_R) - x_previous_RO;
    *value_L=sum_L/FPoints;
    *value_R=sum_R/FPoints;
    sum_previous_LO=sum_L;
    sum_previous_RO=sum_R;

    x_previous_LO=Filter_OBuffer[((SN-FPoints+1)%FPoints)*2];
    x_previous_RO=Filter_OBuffer[((SN-FPoints+1)%FPoints)*2+1];

    return;
}

float Sample_Clip(float value,float GainFactor,float MaxGain, float Threshold)
{
    value=Sample_addGain(value,GainFactor,5);
    value=(value<Threshold)?value:Threshold;
    return value;
}
float Sample_Clip2(float value,float GainFactor,float MaxGain, float Threshold, float clipping_Factor)
{
    (clipping_Factor<1)?clipping_Factor:1.5;
    value=Sample_addGain(value,GainFactor,5);
    if (value>Threshold)
    {
        value=(Threshold+ ((value-Threshold)/clipping_Factor) );
    }
    if(value<-Threshold)
    {
        if (asymmetrical_Dist && clipping_Factor>4) clipping_Factor/=2;
        value=(-Threshold+ ((value+Threshold)/clipping_Factor) );
    }
    return value;
}
void Sample_Tremolo(float *L,float *R, float period, float depth)
{
    //monosample= 32000*sin(_pi/22050*(i+phase)*freq);
    //*L = *L * (1-depth*sin(2*_pi*SN/(SamplingRate*period)));
    //*R = *R * (1-depth*sin(2*_pi*SN/(SamplingRate*period)+Stereo_Tr));
    *L = *L * (1-depth*TrOSC_L.newSample(1/period,SamplingRate));
    *R = *R * (1-depth*TrOSC_R.newSample((1/period)+Stereo_Tr, SamplingRate));
}
void Sample_Delay(float *value_L, float *value_R,float Dtime, float FeedBack, float Mix)
{

    // 1.Retrieve Samples from Delay_Buffer
    long size=Dtime*SamplingRate;
    long delay_sampleNum = (SN%size)*2;
    float DB_value_L= Delay_Buffer[delay_sampleNum];
    float DB_value_R= Delay_Buffer[delay_sampleNum+1];

    // 2.multiply * FeedBack and store back to Delay_Buffer
    Delay_Buffer[delay_sampleNum]=Delay_Buffer[delay_sampleNum]*FeedBack + (*value_L);
    Delay_Buffer[delay_sampleNum+1]=Delay_Buffer[delay_sampleNum+1]*FeedBack + (*value_R);

    // 3.Multiply them * Mix, Add to input Samples and send to output
    *value_L=Mix*DB_value_L+(*value_L);
    *value_R=Mix*DB_value_R+(*value_R);
}
void Sample_Vibrato(float *value_L, float *value_R,float Depth, float Speed, float FeedBack, float Wet, float Dry)
{   //also works as a flanger if you mix with the dry signal
    //-pure vibrato if you remove the dry signal completely

    //0.In Vibrato the delay time is variable
    //- changes as a sinusoid (... i think so :)
    //- so we need to calculate it here
    //=>We calculate the sample number in the Delay Buffer that
    //  should be played now
    double Min_Delay=10;
    double sr = SamplingRate;
    double sn=(double)SN;
    double Dtime0 = (double) (Min_Delay/sr);
    //double Dtime = Dtime0 + Depth - Depth * sin(Speed * sn * 2.0d * _pi/ sr);
    double Dtime = Dtime0 + Depth - Depth * VibOSC.newSample(Speed,SamplingRate);

    long size=5*SamplingRate;// our 5 seconds Delay Buffer of 2 channels
    long CurrentBufPointer=(SN%size);
    double DSamples = Dtime*sr;
    long dsi=DSamples;
    //double DSamples = 10 + 22.05 - 22.05 * sin(Speed*sn*2.0d*_pi/sr);
    long DelayBufPointer = (CurrentBufPointer-dsi);
    if (DelayBufPointer>=size) DelayBufPointer-=size;
    if (DelayBufPointer<0) DelayBufPointer+=size;

    //0.5 Perhaps We will need to introduce interpolation for non-integer sample-delay times
    //OK! Lets go for the cubic interpolation
    double hd;
    double med=modf(DSamples,&hd);
    double y0=Vibrato_Buffer[((DelayBufPointer-1)%size)*2];
    double y1=Vibrato_Buffer[((DelayBufPointer)%size)*2];
    double y2=Vibrato_Buffer[((DelayBufPointer+1)%size)*2];
    double y3=Vibrato_Buffer[((DelayBufPointer-2)%size)*2];
    float DB_value_L=cubic_interpolate(y0,y1,y2,y3,med);
    y0=Vibrato_Buffer[((DelayBufPointer-1)%size)*2+1];
    y1=Vibrato_Buffer[((DelayBufPointer)%size)*2+1];
    y2=Vibrato_Buffer[((DelayBufPointer+1)%size)*2+1];
    y0=Vibrato_Buffer[((DelayBufPointer-2)%size)*2+1];
    float DB_value_R=cubic_interpolate(y0,y1,y2,y3,med);

    //float DB_value_L=Vibrato_Buffer[DelayBufPointer*2] + (Vibrato_Buffer[DelayBufPointer*2+2]-Vibrato_Buffer[DelayBufPointer*2])*med;
    //float DB_value_R=Vibrato_Buffer[DelayBufPointer*2+1] + (Vibrato_Buffer[DelayBufPointer*2+1+2]-Vibrato_Buffer[DelayBufPointer*2+1])*med;
    // 1.Retrieved Samples from Delay_Buffer with interpolation
    // - but this doesnt salve any problem - we have the same quality... :(

    // 2.multiply * FeedBack and store back to Delay_Buffer
    Vibrato_Buffer[CurrentBufPointer*2]=DB_value_L*FeedBack + (*value_L);
    Vibrato_Buffer[CurrentBufPointer*2+1]=DB_value_R*FeedBack + (*value_R);

    // 3.Multiply them * Mix, Add to input Samples and send to output
    *value_L=Wet*DB_value_L+Dry*(*value_L);
    *value_R=Wet*DB_value_R+Dry*(*value_R);
}
void Sample_Chorus(float *value_L, float *value_R,float Depth, float Speed, float FeedBack, float Wet, float Dry)
{
    //0.In Vibrato the delay time is variable
    //- changes as a triangular function
    //- so we need to calculate it here
    //=>We calculate the sample number in the Delay Buffer that
    //  should be played now
    float sr = SamplingRate;
    float period=(1/Speed)*sr;
    long lperiod=(long)period;
    long Min_Delay=10;
    float factor=Depth/period;
    float DelaySamples =0;
    float rounding = 10;

    if (SN%lperiod==0) asc_Ch = !asc_Ch;
    //if (DelaySamples>=Depth) asc_Ch = !asc_Ch;
    //if (DelaySamples<=Min_Delay) asc_Ch = !asc_Ch;
    if (asc_Ch)
    {
        if  (SN%lperiod<lperiod/rounding)
        {
            DelaySamples = Min_Delay + factor * ((SN%lperiod)-(lperiod/rounding)) + Depth/5*sin((_pi/2)*(SN%lperiod)/lperiod) ;
        }
        else
        {
            DelaySamples = Min_Delay + factor * (SN%lperiod) ;
        }
    }
    else
    {
        if  (SN%lperiod>lperiod-lperiod/rounding)
        {
            DelaySamples = Depth - factor * ((SN%lperiod)-(lperiod/rounding)) - Depth/5*cos((_pi/2)*(SN%lperiod)/lperiod) ;
        }
        else
        {
            DelaySamples = Depth - factor * (SN%lperiod) ;
        }
    }
    long size=5*SamplingRate;// our 5 seconds Delay Buffer
    long CurrentBufPointer=(SN%size);
    long DelayBufPointer = CurrentBufPointer-DelaySamples;
    if (DelayBufPointer>=size) DelayBufPointer-=size;
    if (DelayBufPointer<0) DelayBufPointer+=size;

    //0.6 An sinusoid envelope filtrer is needed here to wipe out the glitches
    // as well as a second delay buffer running with 90 degrees phase with the first one
    // ... But unfortunately this trick does not work for chorus - only for pitch shifting :(
    //long DelayBufPointer2 = CurrentBufPointer-DelaySamples - lperiod/2;
    //if (DelayBufPointer2>=size) DelayBufPointer2-=size;
    //if (DelayBufPointer2<0) DelayBufPointer2+=size;


    // 1.Retrieve Samples from Delay_Buffer with interpolation
    double hd;
    double med=modf(DelaySamples,&hd);
    //float dbvl=Chorus_Buffer[DelayBufPointer*2];
    //float dbvl1=Chorus_Buffer[DelayBufPointer*2+2];
    float DB_value_L=Chorus_Buffer[DelayBufPointer*2] + (Chorus_Buffer[DelayBufPointer*2+2]-Chorus_Buffer[DelayBufPointer*2])*med;
    float DB_value_R=Chorus_Buffer[DelayBufPointer*2+1] + (Chorus_Buffer[DelayBufPointer*2+1+2]-Chorus_Buffer[DelayBufPointer*2+1])*med;

    //float DB_value_L= (sin(2*_pi*(SN%lperiod)/lperiod))*Chorus_Buffer[DelayBufPointer*2]+(sin((2*_pi*(SN%lperiod)/lperiod)))*Chorus_Buffer[DelayBufPointer2*2];
    //float DB_value_R= (sin(2*_pi*(SN%lperiod)/lperiod))*Chorus_Buffer[DelayBufPointer*2+1]+(sin((2*_pi*(SN%lperiod)/lperiod)))*Chorus_Buffer[DelayBufPointer2*2+1];
    //float DB_value_L= Chorus_Buffer[DelayBufPointer*2];
    //float DB_value_R= Chorus_Buffer[DelayBufPointer*2+1];

    // 2.multiply * FeedBack and store back to Delay_Buffer
    Chorus_Buffer[CurrentBufPointer*2]=DB_value_L*FeedBack + (*value_L);
    Chorus_Buffer[CurrentBufPointer*2+1]=DB_value_R*FeedBack + (*value_R);

    // 3.Multiply them * Mix, Add to input Samples and send to output
    *value_L=Wet*DB_value_L+Dry*(*value_L);
    *value_R=Wet*DB_value_R+Dry*(*value_R);
}
void Sample_Shifter(float *value_L, float *value_R,float Shift, float MaxD, float FeedBack, float Wet, float Dry)
{
    //0.In Pitch Shiftng the delay time is variable
    //- changes linearly - sawtooth function - from 8 to ...(something) samples (... i think so :)
    //- so we need to calculate it here
    //float Min_Delay=5;
    float sr = SamplingRate;
    //long lMaxD= (long) MaxD;
    /*float Dtime0 = (float) (f/sr);
    float f2 = atan((Shift)/(MaxD));
    long l = (long) f2;
    float f3 = (float) ( (SN%l)/ SamplingRate);
    float Dtime = Dtime0 + Shift * f3;
    */
    //lets try calculating the delay in samples instead ...
    long Delay_samples=0;
    if (Shift>0)
    {
        Delay_samples = Shift*(SN%(lMaxD));
    }
    else
    {
        Delay_samples = (-lMaxD*Shift)+ Shift*(SN%(lMaxD));
    }

    //long size=5*SamplingRate;// our 5 seconds Delay Buffer of 2 channels
    long size=lMaxD;// our 5 seconds Delay Buffer of 2 channels
    long CurrentBufPointer=(SN%size);
    long DelayBufPointer = CurrentBufPointer-Delay_samples;
    if (DelayBufPointer>=size) DelayBufPointer-=size;
    if (DelayBufPointer<0) DelayBufPointer+=size;

    //0.6 An envelope filtrer is needed here to wipe out the glitches
    // as well as a second delay buffer running with 180 degrees phase with the first one
    long DelayBufPointer2 = CurrentBufPointer-lMaxD/2;
    if (DelayBufPointer2>=lMaxD) DelayBufPointer2-=lMaxD;
    if (DelayBufPointer2<0) DelayBufPointer2+=lMaxD;

    // 1.Retrieve Samples from the Delay_Buffer and apply sine envelop
    float DB_value_L= (sin(2*_pi*(SN%(lMaxD))/(lMaxD*2)))*Shifter_Buffer[DelayBufPointer*2] ;
    float DB_value_R= (sin(2*_pi*(SN%(lMaxD))/(lMaxD*2)))*Shifter_Buffer[DelayBufPointer*2+1];
    // store the values to the second buffer
    Shifter_B2[CurrentBufPointer*2]=DB_value_L;
    Shifter_B2[CurrentBufPointer*2+1]=DB_value_R;
    // add the second buffer delayed by 90 degr
    DB_value_L+= Shifter_B2[DelayBufPointer2*2];
    DB_value_R+= Shifter_B2[DelayBufPointer2*2+1];


    // 2.multiply * FeedBack and store back to Delay_Buffer
    Shifter_Buffer[CurrentBufPointer*2]=DB_value_L*FeedBack + (*value_L);
    Shifter_Buffer[CurrentBufPointer*2+1]=DB_value_R*FeedBack + (*value_R);

    // 3.Multiply them * Mix, Add to input Samples and send to output
    *value_L=Wet*DB_value_L+Dry*(*value_L);
    *value_R=Wet*DB_value_R+Dry*(*value_R);
}

float square(float f)
{
    long samplePeriod=SamplingRate/f;
    if ((SN%samplePeriod)>(samplePeriod/2)) return 1;
    else return -1;
}
float triangle(float f)
{
    long samplePeriod=SamplingRate/f;
    float t=(SN%samplePeriod);
    if (t>(samplePeriod/2))
    {
        float v=-1+2*t/samplePeriod;
        return v;
    }
    else
    {
        float v=1-2*t/samplePeriod;
        return v;
    }
}
float sawtooth(float f)
{
    long samplePeriod=SamplingRate/f;
    float t=(SN%samplePeriod);
    float v=-1+2*t/samplePeriod;
    return v;
}
void initFFT()
{
    float sr = SamplingRate;
    for (int i=0;i<N_FFT;i++)
    {
        //create the lookup table for the RtFT
        LUT[i]=sin(i*2*_pi/N_FFT)/N_FFT;
        FFTBufferL[i]=0;
        FFTBufferR[i]=0;
    }
    for (int i=0;i<Kmax_FFT;i++)
    {
        AFFTL[i]=0;
        BFFTL[i]=0;
        AFFTR[i]=0;
        BFFTR[i]=0;
        FFTFrequency[i]=(float)i*sr/(float)N_FFT;
    }
    iFFT=0;
}
void Sample_RtFT(float L, float R)
{// Real-time Fourier Transform
    // for economy we FFT only the Left Chanel

    new_valFFTL=L;
    //new_valFFTR=R;
    old_valFFTL=FFTBufferL[iFFT];
    //old_valFFTR=FFTBufferR[iFFT];
    FFTBufferL[iFFT]=new_valFFTL;
    //FFTBufferR[iFFT]=new_valFFTR;
    diffFFTL=new_valFFTL-old_valFFTL;
    //diffFFTR=new_valFFTR-old_valFFTR;
    AFFTL[0]+=diffFFTL*LUT[CosOffset_FFT];
    //AFFTR[0]+=diffFFTR*LUT[CosOffset_FFT];
    for (kFFT=1;kFFT<Kmax_FFT;kFFT++)
    {
        jFFT=(kFFT*iFFT)%N_FFT;
        qFFT=(jFFT+CosOffset_FFT)%N_FFT;
        AFFTL[kFFT]+=diffFFTL*LUT[qFFT];
        //AFFTR[kFFT]+=diffFFTR*LUT[qFFT];
        BFFTL[kFFT]+=diffFFTL*LUT[jFFT];
        //BFFTR[kFFT]+=diffFFTR*LUT[jFFT];
    }
    iFFT++;
    if (iFFT>=N_FFT) iFFT=0;
}
void Sample_LPFSynth(float *value_L, float *value_R)
{
    double xL = (*value_L);
    // Calculate difference equation
    yL_FFT= (a0_FFT*xL)+ (a1_FFT*x1L_FFT)+ (a2_FFT*x2L_FFT)+ (a3_FFT*x3L_FFT)+ (a4_FFT*x4L_FFT)
           -(b1_FFT*y1L_FFT)- (b2_FFT*y2L_FFT)- (b3_FFT*y3L_FFT)- (b4_FFT*y4L_FFT);
    x4L_FFT=x3L_FFT;
    x3L_FFT=x2L_FFT;
    x2L_FFT=x1L_FFT;
    x1L_FFT=xL;
    y4L_FFT=y3L_FFT;
    y3L_FFT=y2L_FFT;
    y2L_FFT=y1L_FFT;
    y1L_FFT=yL_FFT;
    *value_L = yL_FFT;
    *value_R = yL_FFT;
    // for economy we FFT only the Left Chanel
}
void Sample_Envelope(float *value_L, float *value_R)
{}
void Sample_Synthesize(float *value_L, float *value_R)
{
    float sr=SamplingRate;
    float outS=0;
    float LPcoef=1;
    int protectionCount=0;
    for (int i=0; i<Kmax_FFT;i++)
    //for (int j=0; j<36;j++)
    {//look in the notes table... to quantize at notes' pitch so we dont have to look throught the entire frequency range.
        //int i=notes[j];
        float magn=LPcoef*sqrt(AFFTL[i]*AFFTL[i]+BFFTL[i]*BFFTL[i]);
        if (magn>Threshold_Synth)
        {
            protectionCount++;
            outS+=  0.2*SinMag_Synth* (magn-Threshold_Synth)*50* sin(2*_pi*FFTFrequency[i]*SN/sr) +
            /* 0.2*LowOct_Synth*  magn*100* sin(_pi*FFTFrequency[i]*SN/sr) +
            0.2*HiOct_Synth*  magn*100* sin(4*_pi*FFTFrequency[i]*SN/sr) + 0.2*VLOct_Synth*  magn*100* sin(0.5*_pi*FFTFrequency[i]*SN/sr) +*/
                    0.2*SqrMag_Synth*  (magn-Threshold_Synth)*50* square(FFTFrequency[i]) +
                    0.2*TriMag_Synth*MidOct_Synth*  (magn-Threshold_Synth)*50* triangle(FFTFrequency[i]) +
                    0.2*TriMag_Synth*LowOct_Synth*  (magn-Threshold_Synth)*50* triangle(FFTFrequency[i]/2) +
                    0.2*TriMag_Synth*HiOct_Synth*  (magn-Threshold_Synth)*50* triangle(FFTFrequency[i]*2) +
                    0.2*SawMag_Synth*  (magn-Threshold_Synth)*50* sawtooth(FFTFrequency[i]);
            CurrentFreq=FFTFrequency[i];            
        }
        if (protectionCount>9) break;
        //if (i>20) LPcoef+=i/50;
    }
    *value_L=Dry_Synth*(*value_L) + Wet_Synth*(outS);
    *value_R=Dry_Synth*(*value_R) + Wet_Synth*(outS);
}
void Sample_Synthesize2(float *value_L, float *value_R)
{
    float sr=SamplingRate;
    float outS=0;
    int protectionCount=0;
    int k=0;
    if ((SN%100)==0)
    {// we "hear" the FFT results every 400 samples to find the frequencies played
        CurrentFreq=0;
        for (int i=15; i<Kmax_FFT;i++)
        {
            float magn=sqrt(AFFTL[i]*AFFTL[i]+BFFTL[i]*BFFTL[i]);
            if (magn>Threshold_Synth)
            {
                protectionCount++;
                PlayedFreq_Synth[k]=FFTFrequency[i];
                PlayedMagn_Synth[k++]=magn;
                if (protectionCount==1) CurrentFreq=FFTFrequency[i];
            }
            if (protectionCount>Polyphony_Synth) break;
        }
        for(int i=k;i<Polyphony_Synth;i++)
        {
            PlayedFreq_Synth[i]=0;
        }
    }
    for (int i=0;i<Polyphony_Synth;i++)
    {
        if (PlayedFreq_Synth[i]==0) break;
        float magn=PlayedMagn_Synth[i];
        outS+=
                0.2*SinMag_Synth* (magn-Threshold_Synth)*50* sin(2*_pi*PlayedFreq_Synth[i]*SN/sr) +
                //0.2*SinMag_Synth* (magn-Threshold_Synth)*50* oscGT[i].newSample(PlayedFreq_Synth[i]) +
                0.2*SqrMag_Synth*  (magn-Threshold_Synth)*50* square(PlayedFreq_Synth[i]) +
                0.2*TriMag_Synth*MidOct_Synth*  (magn-Threshold_Synth)*50* triangle(PlayedFreq_Synth[i]) +
                0.2*TriMag_Synth*LowOct_Synth*  (magn-Threshold_Synth)*50* triangle(PlayedFreq_Synth[i]/2) +
                0.2*TriMag_Synth*HiOct_Synth*  (magn-Threshold_Synth)*50* triangle(PlayedFreq_Synth[i]*2) +
                0.2*SawMag_Synth*  (magn-Threshold_Synth)*50* sawtooth(PlayedFreq_Synth[i]);
    }

    *value_L=Dry_Synth*(*value_L) + Wet_Synth*(outS);
    *value_R=Dry_Synth*(*value_R) + Wet_Synth*(outS);
}
void Sample_Synth(float *value_L, float *value_R)
{
    float sr=SamplingRate;
    float outSL=0;
    float L=*value_L;
    float R=*value_R;
    float L1=*value_L;
    float R1=*value_R;

    //0. LP Filter - cutoff at 1300Hz
    //- it doesnt really improve anything so we disable it for economy
    //Sample_LPFSynth(&L,&R);
    //1. FFT on the input Buffer
    Sample_RtFT(L,R);
    //3. Synthesize
    Sample_Synthesize2(value_L,value_R);
    //4. Read or Create envelope and apply it -TODO...

    //5. output
    //*value_L=L1;
    //*value_R=R1;
}

float envelop(long t, int i)
{
    float t1= (float) t;
    float ms=(float)t * 1000 /((float)SamplingRate);
    float Att=(SamplingRate/1000)*MIDIAttack;
    float Sus=(SamplingRate/1000)*MIDISustain;
    float Dec=(SamplingRate/1000)*MIDIDecay;
    float Rel=(SamplingRate/1000)*MIDIRelease;

    if ((MIDIDecay>0)&&(t1>=Att+Sus+Dec-(Dec/5))&&(MIDIDecay!=0)&&(MIDIVelosities[i]>0))
    {
        MIDIVelosities[i]=-MIDIVelosities[i];
        MIDITimes[i]=0;
        return MIDIDecayInit[i];
    }
    if ((MIDIDecay==0)&&(MIDISustain==0)&&(t1>=Att)&&(MIDIVelosities[i]>0))
    {
        MIDIVelosities[i]=-MIDIVelosities[i];
        MIDITimes[i]=0;
        return MIDIDecayInit[i];
    }
    if ((MIDIVelosities[i]<0))
    {
        if (t1>Rel)
        {
            MIDIVelosities[i]=0;
            MIDITimes[i]=0;
            MIDIDecayInit[i]=0;
            MIDIReleaseInit[i]==0;
            return 0;
        }
        //Release
        //if (MIDIVelosities[i]>0) MIDIVelosities[i]=-MIDIVelosities[i];
        float y0=MIDIDecayInit[i];
        float f=((y0-(t1*y0)/Rel)/(t1/(Rel)+1));
        MIDIReleaseInit[i]=f;
        return f;
    }
    if (t1<=Att)//attack
    {
        if (MIDIReleaseInit[i]>0)
        {
            float y=MIDIReleaseInit[i];
            //t1=Att*(1+sqrt(1-pow(y,2)));
            //MIDITimes[i]=(long)t1;
            float f=((1-y)/pow(Att,2))*pow(t1,2)+y;
            MIDIDecayInit[i]=f;
            return f;
        }
        //float f=(t1/(t1+(Att*2)));
        float f=sqrt(1-(pow(t1-Att,2)/pow(Att,2)));
        MIDIDecayInit[i]=f;
        //MIDIReleaseInit[i]=f;
        return f;
    }
    if (t1<=Att+Sus)//Sustain
    {
        MIDIDecayInit[i]=1;
        return 1;
    }
    if ((MIDIDecay==0)&&(MIDISustain>0))
    {
        MIDIDecayInit[i]=1;//Sustain until note off
        return 1;
    }
    if (t1<=Att+Sus+Dec-(Dec/5))//Decay
    {
        t1-=(Att+Sus);
        float f=sqrt(1-((pow(t1,2)/(pow(Dec,2)))));
        MIDIDecayInit[i]=f;
        return f;
    }
}
void MIDI_Synth(float *value_L, float *value_R)
{
    int voiceCount=0;
    float sr=SamplingRate;
    float outS=0;
    for (int i=0;i<128;i++)
    {
        if (MIDIVelosities[i]!=0)
        {
            //Pitch Bender - OK!
            //if (CurrentPitchValue<PitchWheelValue) CurrentPitchValue+=0.002;
            //if (CurrentPitchValue>PitchWheelValue) CurrentPitchValue-=0.002;
            float pw = (float) PitchWheelValue; //CurrentPitchValue;
            float pitchB=((pw-64)/64)*PitchBenderRange;
            float fr=MIDIFrequencies[i]*pow(2,pitchB/12);
            //harmonics
            float hf1=2*fr;
            float hf2=3*fr;
            float hf3=5*fr;
            //float fr=MIDIFrequencies[i];
            float v1=(float) MIDIVelosities[i];
            float magn=abs(v1)/127.0f;
            float newnote=0;
            if (SqrMag_MIDISynth>0) newnote+= SqrMag_MIDISynth*  (magn)* square(fr);
            if (TriMag_MIDISynth>0) newnote+=  TriMag_MIDISynth*MidOct_MIDISynth*  (magn)* triangle(fr);
            if (LowOct_MIDISynth>0) newnote+=  TriMag_MIDISynth*LowOct_MIDISynth*  (magn)* triangle(fr/2);
            if (HiOct_MIDISynth>0) newnote+= TriMag_MIDISynth*HiOct_MIDISynth*  (magn)* triangle(fr*2);
            if (SawMag_MIDISynth>0) newnote+= SawMag_MIDISynth*  (magn)* sawtooth(fr);
            //sines
            if (SinMag_MIDISynth>0) newnote+=SinMag_MIDISynth* (magn)* osc[i].newSample(fr,SamplingRate);
            if (HarmB_MIDISynth>0) newnote+=HarmB_MIDISynth* (magn)* osc1[i].newSample(fr/2,SamplingRate);
            if (Harm1_MIDISynth>0) newnote+=Harm1_MIDISynth* (magn)* osc2[i].newSample(hf1,SamplingRate);
            if (Harm2_MIDISynth>0) newnote+=Harm2_MIDISynth* (magn)* osc3[i].newSample(hf2,SamplingRate);
            if (Harm3_MIDISynth>0) newnote+=Harm3_MIDISynth* (magn)* osc4[i].newSample(hf3,SamplingRate);

            //outS+=envelop(MIDITimes[i],i)*newnote;//apply envelop
            float e=MIDIenv[i].newValue();
            outS+=newnote*e;//apply envelop
            if ((MIDIenv[i].state==ENV_STATE_IDLE)&&(e==0))
            {
                //the note finished
                MIDIVelosities[i]=0;
            }
            MIDITimes[i]+=1;//advance time for played note
            voiceCount++;
        }
    }
    float v= (float) VolumeValue;
    *value_L=(*value_L) + (v/127.0f)*(outS);
    *value_R=(*value_R) + (v/127.0f)*(outS);
}


void Sample_Volume(float *L, float *R, long vol)
{
    *L=(*L)*vol/32600;
    *R=(*R)*vol/32600;
    // ??? Volume auto-adjust ??? see about this...
}

void Short_to_Char(char* char1, char* char2, short sh, int LE_or_BE /*Little or Big Endian - 0 or 1*/)
{
    if (LE_or_BE==1){ // if Number in Big Endian Format
        *char2=(sh&255);
        *char1=sh>>8;
    }
    else{ // if Number in Little Endian Format
        *char1=(sh&255);
        *char2=sh>>8;
    }
}
short Char_to_Short(unsigned char char1, unsigned char char2, int LE_or_BE /*Little or Big Endian 0 or 1*/)
{
    short sh;
    if (LE_or_BE==1){ // if Number in Big Endian Format
        sh= (short)char2;
        sh+=(short)char1*256;
    }
    else{ // if Number in Little Endian Format
        sh= (short)char1;
        sh+=(short)char2*256;
    }
}
void Process_Buffer(char *Buffer, int size)
{
    int i_sample_L=0;
    int i_sample_R=0;
    int i =0;

    double sr = SamplingRate;
    /////////////////////////////////////////////////////////////////////
    // Compressor Init
    // attack and release "per sample decay"
    att = (tatt == 0.0) ? (0.0) : exp (-1.0 / (sr * tatt));
    rel = (trel == 0.0) ? (0.0) : exp (-1.0 / (sr * trel));
    /////////////////////////////////////////////////////////////////////
    // Phaser Init
    lfoInc=2*_pi*SweepRate_Ph/sr;
    dmin=BaseFreq_Ph/(sr/2);
    dmax=OffsetFreq_Ph/(sr/2);
    /////////////////////////////////////////////////////////////////////
    // Wah Init
    //2nd method
    Inc_Wah=2*_pi*SweepRate_Wah/sr;
    dmin_Wah=BaseFreq_Wah/(sr/2);
    dmax_Wah=OffsetFreq_Wah/(sr/2);
    /////////////////////////////////////////////////////////////////////


    while (i<size)
    {
        SN=(SN++)%(SamplingRate*10); //counts to 10 seconds samples and then returns to 0
        int j=i;
        char c1L=Buffer[i++];
        char c2L=Buffer[i++];
        char c1R=Buffer[i++];
        char c2R=Buffer[i++];

        i_sample_L=Char_to_Short(c1L,c2L,0);
        i_sample_R=Char_to_Short(c1R,c2R,0);


        // Adjust Input Volume by main faders' indication
        float L1=(float)i_sample_L;
        float R1=(float)i_sample_R;
        Sample_Volume(&L1,&R1,iVolume);
        i_sample_L=(int)L1;
        i_sample_R=(int)R1;
        ////////////////////////////////////////////

        // Show Volume
        long imax = (i_sample_L>i_sample_R)?i_sample_L:i_sample_R;
        if (imax>previusInputSampleMagnitude)
        {
            iascending=true;
        }
        else
        {
            if (iascending) inVolume=previusInputSampleMagnitude;
            iascending=false;
        }
        previusInputSampleMagnitude=imax;
        ////////////////////////////////////////////

        //Noise Filter
        if (Enable_Filter||Enabled_Synth)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Filter(&L,&R,Points_Filter);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }
        if (Enabled_Comp)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Comp(&L,&R,BufferLen_Comp);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//compressor
        if (Enabled_Synth)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Synth(&L,&R);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Synth
        // Add FX
        if (Enabled_Wah)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Wah(&L,&R);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Wah
        if (Enabled_Dist)
        {
            i_sample_L=Sample_Clip2(i_sample_L,GainFactor_Dist,2,Threshold_Dist,ClippingFactor_Dist);
            i_sample_R=Sample_Clip2(i_sample_R,GainFactor_Dist,2,Threshold_Dist,ClippingFactor_Dist);
        }// Clipping ...
        if (Enabled_Tr)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Tremolo(&L, &R, Period_Tr,Depth_Tr);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
            //i_sample_R=Sample_Tremolo(i_sample_R,Period_Tr,Depth_Tr);
        }// Tremolo
        if (Enabled_Ch)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Chorus(&L,&R,Depth_Ch,Speed_Ch,FeedBack_Ch,Wet_Ch,Dry_Ch);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Chorus
        if (Enabled_Vib)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Vibrato(&L,&R,Depth_Vib,Speed_Vib,FeedBack_Vib,Wet_Vib,Dry_Vib);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Vibrato
        if (Enabled_Sh)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Shifter(&L,&R,Shift_Sh,MaxD_Sh,FeedBack_Sh,Wet_Sh,Dry_Sh);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Pitch Shifter
        if (Enabled_Vib||Enabled_Ch||Enabled_Sh|Enabled_Synth)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_OFilter(&L,&R,Points_OFilter);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//output filter
        if (Enabled_Ph)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Phaser(&L,&R);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Phaser
        if (Enabled_D)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Delay(&L,&R,DTime_D,FeedBack_D,Mix_D);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Delay
        if (Enabled_Rv)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_Reverb(&L,&R);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Delay
        if (Enabled_EQ)
        {
            float L=(float)i_sample_L;
            float R=(float)i_sample_R;
            Sample_EQ(&L,&R);
            i_sample_L=(int)L;
            i_sample_R=(int)R;
        }//Delay
        // Adjust Output Volume by main faders' indication
        float L=(float)i_sample_L;
        float R=(float)i_sample_R;
        Sample_Volume(&L,&R,Volume);
        i_sample_L=(int)L;
        i_sample_R=(int)R;
        ////////////////////////////////////////////

        // Show Volume
        long max = (i_sample_L>i_sample_R)?i_sample_L:i_sample_R;
        if (max>previusSampleMagnitude)
        {
            ascending=true;
        }
        else
        {
            if (ascending) outVolume=previusSampleMagnitude;
            ascending=false;
        }
        previusSampleMagnitude=max;
        ////////////////////////////////////////////

        char c1,c2;
        Short_to_Char(&c1,&c2,i_sample_L,0);
        Buffer[j++]=c1;
        Buffer[j++]=c2;
        Short_to_Char(&c1,&c2,i_sample_R,0);
        Buffer[j++]=c1;
        Buffer[j++]=c2;

        First_Run=false;
    }
}

void Process_2Buffers(float *BufferL,float *BufferR, int size)
{//some modifications for Jack - 2 separate buffers, one for each channel: Left and Right
    float L1,R1;
    int i =0;

    double sr = SamplingRate;
    /////////////////////////////////////////////////////////////////////
    // Compressor Init
    // attack and release "per sample decay"
    att = (tatt == 0.0) ? (0.0) : exp (-1.0 / (sr * tatt));
    rel = (trel == 0.0) ? (0.0) : exp (-1.0 / (sr * trel));
    /////////////////////////////////////////////////////////////////////
    // Phaser Init
    lfoInc=2*_pi*SweepRate_Ph/sr;
    dmin=BaseFreq_Ph/(sr/2);
    dmax=OffsetFreq_Ph/(sr/2);
    /////////////////////////////////////////////////////////////////////
    // Wah Init
    Inc_Wah=2*_pi*SweepRate_Wah/sr;
    dmin_Wah=BaseFreq_Wah/(sr/2);
    dmax_Wah=OffsetFreq_Wah/(sr/2);
    /////////////////////////////////////////////////////////////////////


    while (i<size)
    {

        SN=(SN++)%(SamplingRate*10); //counts to 10 seconds samples and then returns to 0

        L1=BufferL[i];
        R1=BufferR[i];

        // Adjust Input Volume by main faders' indication
        Sample_Volume(&L1,&R1,iVolume);
        ////////////////////////////////////////////

        // Show Volume
        long imax = (L1>R1)?L1*32600:R1*32600;
        if (imax>previusInputSampleMagnitude)
        {
            iascending=true;
        }
        else
        {
            if (iascending) inVolume=previusInputSampleMagnitude;
            iascending=false;
        }
        previusInputSampleMagnitude=imax;
        ////////////////////////////////////////////

        //Noise Filter
        if (MIDIExists&&MIDISynthEnabled)
        {
            MIDI_Synth(&L1,&R1);
        }
        if (Enable_Filter/*||Enabled_Synth*/)
        {
            Sample_Filter(&L1,&R1,Points_Filter);
        }
        // Add FX
        if (Enabled_Comp)
        {
            Sample_Comp(&L1,&R1,BufferLen_Comp);
        }//compressor
        if (Enabled_Synth)
        {
            Sample_Synth(&L1,&R1);
        }//Synth
        if (Enabled_Wah)
        {
            Sample_Wah(&L1,&R1);
        }//Wah
        if (Enabled_Dist)
        {
            L1=Sample_Clip2(L1,GainFactor_Dist,2,Threshold_Dist/32600,ClippingFactor_Dist);
            R1=Sample_Clip2(R1,GainFactor_Dist,2,Threshold_Dist/32600,ClippingFactor_Dist);
        }// Clipping ...
        if (Enabled_Tr)
        {
            Sample_Tremolo(&L1,&R1,Period_Tr,Depth_Tr);
            //R1=Sample_Tremolo(R1,Period_Tr,Depth_Tr);
        }// Tremolo
        if (Enabled_Ch)
        {
            Sample_Chorus(&L1,&R1,Depth_Ch,Speed_Ch,FeedBack_Ch,Wet_Ch,Dry_Ch);
        }//Chorus
        if (Enabled_Vib)
        {
            Sample_Vibrato(&L1,&R1,Depth_Vib,Speed_Vib,FeedBack_Vib,Wet_Vib,Dry_Vib);
        }//Vibrato
        if (Enabled_Sh)
        {
            Sample_Shifter(&L1,&R1,Shift_Sh,MaxD_Sh,FeedBack_Sh,Wet_Sh,Dry_Sh);
        }//Pitch Shifter
        if (Enabled_Vib||Enabled_Ch||Enabled_Sh||Enabled_Synth)
        {
            Sample_OFilter(&L1,&R1,Points_OFilter);
        }//Output Noise Filter
        if (Enabled_Ph)
        {
            Sample_Phaser(&L1,&R1);
        }//Phaser
        if (Enabled_D)
        {
            Sample_Delay(&L1,&R1,DTime_D,FeedBack_D,Mix_D);
        }//Delay
        if (Enabled_Rv)
        {
            Sample_Reverb(&L1,&R1);
        }//Delay
        if (Enabled_EQ)
        {
            Sample_EQ(&L1,&R1);
        }//EQ
        if (Enabled_Looper)
        {
            looper.newSample(&L1,&R1,Rec_Buffer,Play_Buffer);
        }//Looper
        // Adjust Output Volume by main faders' indication
        Sample_Volume(&L1,&R1,Volume);
        ////////////////////////////////////////////

        // Show Volume
        long max = (L1>R1)?L1*32600:R1*32600;
        if (max>previusSampleMagnitude)
        {
            ascending=true;
        }
        else
        {
            if (ascending) outVolume=previusSampleMagnitude;
            ascending=false;
        }
        previusSampleMagnitude=max;
        ////////////////////////////////////////////

        BufferL[i]=L1;
        BufferR[i]=R1;
        i++;
        First_Run=false;
    }
}

////////////////////////////////////////////////////////////

// JACK /////////////////////////////////////////////////////////////////

// declare two "jack_port_t" pointers, which will each represent a port
// in the JACK graph (ie: Connections tab in QJackCtl)
jack_port_t* midiinputPort = 0;
jack_port_t* inputPortL = 0;
jack_port_t* outputPortL = 0;
jack_port_t* inputPortR = 0;
jack_port_t* outputPortR = 0;


// this function is the main audio processing loop, JACK calls this function
// every time that it wants "nframes" number of frames to be processed.
// nframes is usually between 64 and 256, but you should always program
// so that you can work with any amount of frames per process() call!
int process(jack_nframes_t nframes, void* )
{
    //midi
    handleMIDI();
    //audio

    // here's a touch tricky, port_get_buffer() will return a pointer to
    // the data that we will use, so cast it to (float*), so that we
    // can use the data as floating point numbers. JACK will always pass
    // floating point samples around, the reason that we have to cast it
    // ourselves is so that it could be changed in the future... don't worry
    // about it too much.
    float* inputBufferL = (float*)jack_port_get_buffer ( inputPortL , nframes);
    float* outputBufferL= (float*)jack_port_get_buffer ( outputPortL, nframes);
    float* inputBufferR = (float*)jack_port_get_buffer ( inputPortR , nframes);
    float* outputBufferR= (float*)jack_port_get_buffer ( outputPortR, nframes);
    // this is the intresting part, we work with each sample of audio data
    // one by one, copying them across. Try multiplying the input by 0.5,
    // it will decrease the volume...
    Process_2Buffers(inputBufferL,inputBufferR, (int)nframes);

    for ( int i = 0; i < (int) nframes; i++)
    {
        // copy data from input to output. Note this is not optimized for speed!
        //outputBufferL[i] = inputBufferL[i];
        outputBufferL[i] = inputBufferL[i];
        outputBufferR[i] = inputBufferR[i];
    }

    return 0;
}

int myJack()
{
    //std::cout << "JACK client tutorial" << std::endl;

    // create a JACK client and activate
    //Deprecated function but more powerfull...
    // ...runs even if jackd is not running and automatically starts jackd.
    // Maybe in later version it will not run at all... :(
    client = jack_client_open ("astyl",JackNullOption,0,0);
    //client = jack_client_new ("astyl");

    // register the process callback
    jack_set_process_callback  (client, process , 0);

    //register the midi in port
    //midiinputPort  = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

    // register two ports, one input one output, both of AUDIO type
    inputPortL = jack_port_register ( client,
                                    "inputL",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsInput,
                                    0 );

    outputPortL = jack_port_register ( client,
                                    "outputL",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput,
                                    0 );
    // register two ports, one input one output, both of AUDIO type
    inputPortR = jack_port_register ( client,
                                    "inputR",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsInput,
                                    0 );

    outputPortR = jack_port_register ( client,
                                    "outputR",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput,
                                    0 );
    jack_nframes_t fr1=(jack_nframes_t)256;
    int ij= jack_set_buffer_size(client,fr1);
    jack_nframes_t fr = jack_get_sample_rate(client);
    SamplingRate=(long)fr;
    if (SamplingRate!=44100)
    {
        delete &looper;
        looper = Looper(SamplingRate, Rec_Buffer, Play_Buffer);
    }

    // activate the client, ie: enable it for processing
    jack_activate(client);

    //connect ports
    const char **ports;
    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL)
    {
                   fprintf(stderr, "Cannot find any physical capture ports\n");
                   exit(1);
    }

    if (int k=(jack_connect (client, ports[0], jack_port_name (inputPortL)) ))
    {
        fprintf (stderr, "cannot connect input ports\n");

    }
    if (jack_connect (client, ports[1], jack_port_name (inputPortR)))
    {
        fprintf (stderr, "cannot connect input ports\n");
    }

    free (ports);

    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL)
    {
        fprintf(stderr, "Cannot find any physical playback ports\n");
        exit(1);
    }

    if (jack_connect (client, jack_port_name (outputPortL), ports[0]))
    {
        fprintf (stderr, "cannot connect output ports\n");
    }
    if (jack_connect (client, jack_port_name (outputPortR), ports[1]))
    {
        fprintf (stderr, "cannot connect output ports\n");
    }
    if (jack_connect (client, jack_port_name (outputPortL), ports[2]))
    {
        fprintf (stderr, "cannot connect output ports\n");
    }
    if (jack_connect (client, jack_port_name (outputPortR), ports[3]))
    {
        fprintf (stderr, "cannot connect output ports\n");
    }

    free (ports);
}

