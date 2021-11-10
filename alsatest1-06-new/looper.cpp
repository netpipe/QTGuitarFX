#include "looper.h"
#include <stdlib.h>
#include <stdio.h>

Looper::Looper(long sr, float *RB, float *PB)
{
    long max=Max_Looper_Time;
    LoopSize=0;
    t_Looper=0;
    State=0;
    FirstRec=true;
    SR=sr;
    for (int j=0;j<(600*2*44100);j++)
    {
        RB[j]=0;
        RB[j]=0;
    }
}
Looper::~Looper()
{
}


void Looper::newSample(float *value_L, float *value_R, float *Rec_Buffer, float *Play_Buffer)
{
    if (State==LOOPER_STATE_STOP)
    {
        return;
    }
    long t=2*t_Looper;
    long max=Max_Looper_Time;
    if ((State==LOOPER_STATE_RECORD)&&(FirstRec))
    {
        //FirstRec=false;
        // store to Rec Buffer
        Rec_Buffer[t]=(*value_L);
        Rec_Buffer[t+1]=(*value_R);
        //increment the pointer of the end of the buffer - thus when finished we get the Loop Time
        LoopSize++;
        if (LoopSize==max*SR) FirstRec=false;
        //increment the time of the looper
        t_Looper++;
        if (t_Looper>=max*SR) t_Looper=0;
        return;
    }
    if ((State==LOOPER_STATE_RECORD)&&(!FirstRec))
    {
        float RL=Rec_Buffer[t];
        float RR=Rec_Buffer[t+1];
        //float PL=Play_Buffer[t];
        //float PR=Play_Buffer[t+1];
        //Play_Buffer[t]+=L;
        //Play_Buffer[t+1]+=R;
        // store to rec buffer
        Rec_Buffer[t]+=(*value_L);
        Rec_Buffer[t+1]+=(*value_R);
        //return buffers value mixed with input value
        (*value_L)+=RL+Play_Buffer[t];
        (*value_R)+=RL+Play_Buffer[t+1];
        //increment the time of the looper
        t_Looper++;
        if (t_Looper>=LoopSize) t_Looper=0;
        return;
    }
    if (State==LOOPER_STATE_PLAY)
    {
        //get Play Buffer values
        float L=Play_Buffer[t]+Rec_Buffer[t];
        float R=Play_Buffer[t+1]+Rec_Buffer[t+1];
        //return play buffers value mixed with input value
        (*value_L)+= L;
        (*value_R)+= R;
        t_Looper++;
        if (t_Looper>=LoopSize) t_Looper=0;
        return;
    }
}
int Looper::SetState(int St)
{
    if (St==LOOPER_STATE_STOP)
    {
        if ((State==LOOPER_STATE_STOP)) return -1;
        if (LoopSize>0) FirstRec=false;
        State=St;
        t_Looper=0;
        return 0;
    }
    if (St==LOOPER_STATE_RECORD)
    {
        if (State==LOOPER_STATE_RECORD)return -1;
        if (LoopSize>0) FirstRec=false;
        State=St;
        return 0;
    }
    if (St==LOOPER_STATE_PLAY)
    {
        if (LoopSize<=0) return -1;
        if (State==LOOPER_STATE_PLAY) return -1;
        if ((State==LOOPER_STATE_STOP)&&(FirstRec))
        {
            t_Looper=0;
            return -1;
        }
        if ((State==LOOPER_STATE_STOP)&&(!FirstRec))
        {
            if (LoopSize>0) FirstRec=false;
            State=St;
        }
        if (State==LOOPER_STATE_RECORD)
        {
            if (FirstRec) t_Looper=0;
            if (LoopSize>0) FirstRec=false;
            State=St;
        }
        return 0;
    }
    return -1;
}
void Looper::StoreRec(float *Rec_Buffer, float *Play_Buffer)
{
    int i = SetState(LOOPER_STATE_STOP);
    //Copy Recording Buffer to Play Buffer
    long m=Max_Looper_Time;
    long max=m*SR*2;
    for (int j=0;j<(max);j++)
    {
        Play_Buffer[j]+=Rec_Buffer[j];
        Rec_Buffer[j]=0;
    }
}
void Looper::ClearRec(float *Rec_Buffer)
{
    int i = SetState(LOOPER_STATE_STOP);
    //Clear Recording Buffer thus clearing last recording
    long m=Max_Looper_Time;
    long max=m*SR*2;
    for (int j=0;j<(max);j++)
    {
        Rec_Buffer[j]=0;
    }
}
void Looper::ClearAll(float *Rec_Buffer, float *Play_Buffer)
{
    int i = SetState(LOOPER_STATE_STOP);
    //Clear All Buffers
    FirstRec=true;
    t_Looper=0;
    LoopSize=0;
    long m=Max_Looper_Time;
    long max=m*SR*2;
    for (int j=0;j<(max);j++)
    {
        Rec_Buffer[j]=0;
        Play_Buffer[j]=0;
    }
}
int Looper::SaveBuffer(char *filename, float *Rec_Buffer, float *Play_Buffer)
{
    unsigned short val;

    if (LoopSize<=0) return 0;
    int i = SetState(LOOPER_STATE_STOP);
    FILE *f=fopen("Recording.au","w");
    if (!f)
    {
        fprintf(stderr, "Looper: Cannot Open File\n");
        return -1;
    }

    //write header
    fprintf(f,".snd");
    // Data Offset
    fputc(0,f);
    fputc(0,f);
    fputc(0,f);
    fputc(24,f);
    long len= 2*LoopSize;
    // Data Size
    fputc((len & 0xff000000) >> 24, f);
    fputc((len & 0x00ff0000) >> 16, f);
    fputc((len & 0x0000ff00) >>  8, f);
    fputc((len & 0x000000ff), f);
    // Encoding: 16 PCM
    fputc(0,f);
    fputc(0,f);
    fputc(0,f);
    fputc(3,f);
    // Sampling Rate
    fputc((SR & 0xff000000) >> 24, f);
    fputc((SR & 0x00ff0000) >> 16, f);
    fputc((SR & 0x0000ff00) >>  8, f);
    fputc((SR & 0x000000ff), f);
    // Channels
    fputc(0,f);
    fputc(0,f);
    fputc(0,f);
    fputc(1,f);

    // Our samples range from -1 to 1 - floating point
    // Write the samples
    for (int i=0;i<LoopSize;i++)
    {
        //we mix the left and right channels into one...
        val=(unsigned short) (32760*(Rec_Buffer[2*i]+Play_Buffer[2*i]+Rec_Buffer[2*i+1]+Play_Buffer[2*i+1])/2);
        fputc((val & 0x0000ff00) >>  8, f);
        fputc((val & 0x000000ff), f);
    }

    fclose(f);
}
