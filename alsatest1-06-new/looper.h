#ifndef LOOPER_H
#define LOOPER_H

#define Max_Looper_Time 600; //(sec)

#define LOOPER_STATE_RECORD     1
#define LOOPER_STATE_PLAY       3
#define LOOPER_STATE_STOP       0

namespace LOOP {class Looper;}
class Looper
{
public:
    int State;
    Looper(long sr, float *RB, float *PB);
    ~Looper();
    void newSample(float *L, float *R , float *Rec_Buffer, float *Play_Buffer);
    int SaveBuffer(char *filename,  float *Rec_Buffer, float *Play_Buffer);
    int SetState(int St);
    void StoreRec(float *Rec_Buffer, float *Play_Buffer);
    void ClearRec(float *Rec_Buffer);
    void ClearAll(float *Rec_Buffer,float *Play_Buffer);
private:
    long SR;
    long LoopSize;
    long t_Looper;
    bool FirstRec;
};

#endif // LOOPER_H
