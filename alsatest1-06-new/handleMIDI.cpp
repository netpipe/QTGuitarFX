#define MOD_WHEEL_DISABLED 0
#define MOD_WHEEL_TREMOLO_DEPTH  1
#define MOD_WHEEL_TREMOLO_SPEED  2
#define MOD_WHEEL_CHORUS_SPEED  3
#define MOD_WHEEL_WAH_SPEED  4
#define MOD_WHEEL_VIBRATO_SPEED  5
#define MOD_WHEEL_VIBRATO_DEPTH  6
#define MOD_WHEEL_PHASER_SPEED  7

///////////////////
// Modulation Wheel
///////////////////
int Mod_Wheel_Connection=0;
int ModulationValue=0;
//////////////////

double const _pi=3.14159265358979323846;

///////////////////
// MIDI Variables
///////////////////
bool MIDIExists=true;
bool MIDISynthEnabled=false;
bool TriggerLooper=false;
long TriggerTime=0;
bool LooperTriggered=false;
bool LooperFlagOn=false;

snd_rawmidi_t* midiin = NULL;
void errormessage(const char *format, ...);
char commandBuffer[100]; //Buffer that holds the midi commands
int commandPointer =0;  // Pointer to the command in the buffer
int bufferPointer=0; //Pointer to the current read position
int VolumeValue=120;
int PitchWheelValue=64;
int MIDIFrequencies[128];
int MIDIVelosities[128];
long MIDITimes[128];
float MIDIDecayInit[128];
float MIDIReleaseInit[128];
Sine_OSC osc[128];// 128 Oscilators
Sine_OSC osc1[128];// 128 Oscilators
Sine_OSC osc2[128];// 128 Oscilators
Sine_OSC osc3[128];// 128 Oscilators
Sine_OSC osc4[128];// 128 Oscilators
env MIDIenv[128];//128 envelop generators

int NormalizeValue(int mv)
{
    if (Mod_Wheel_Connection==0) return 0;
    if (Mod_Wheel_Connection==MOD_WHEEL_TREMOLO_DEPTH) return mv/1.28;
    if (Mod_Wheel_Connection==MOD_WHEEL_TREMOLO_SPEED) return ((25*(128-mv)/128)*1000);
    if (Mod_Wheel_Connection==MOD_WHEEL_CHORUS_SPEED) return ((250*(mv)/128));
    if (Mod_Wheel_Connection==MOD_WHEEL_WAH_SPEED) return ((50*(mv)/128));
    if (Mod_Wheel_Connection==MOD_WHEEL_VIBRATO_SPEED) return ((70*(mv)/128));
    if (Mod_Wheel_Connection==MOD_WHEEL_VIBRATO_DEPTH) return ((300*(mv)/128));
    if (Mod_Wheel_Connection==MOD_WHEEL_PHASER_SPEED) return ((50*(mv)/128));
}

///////////////////
// Midi Commands
///////////////////
void NoteOnOff()
{
    int i=(unsigned char)commandBuffer[1];
    if (i>124)  return;
    int j=(unsigned char)commandBuffer[2];
    if (j==0)
    {
        // Hnadle Note Off for Release
        MIDIenv[i].ResetState(ENV_STATE_RELEASE);
    }
    else
    {   //Note on
        MIDIVelosities[i]=j;
        MIDIenv[i].ResetState(ENV_STATE_ATTACK);
    }
}
void NoteOnOff1()
{
    int i=(unsigned char)commandBuffer[1];
    if (i>124)  return;
    int j=(unsigned char)commandBuffer[2];
    if (j==0)
    {
        // Hnadle Note Off for Release
        if (MIDIVelosities[i]<0) return;
        MIDITimes[i]=0;
        MIDIVelosities[i]=-MIDIVelosities[i];
    }
    else
    {   //Note on
        MIDIVelosities[i]=j;
        MIDITimes[i]=0;
        MIDIDecayInit[i]=0;
    }
}
void ModWheelVolChange()
{
    if ((unsigned char)commandBuffer[1] == 1)
    {
    // modulation wheel change
        ModulationValue=(unsigned char)commandBuffer[2];
        ModulationValue= NormalizeValue(ModulationValue);
    }

    if ((unsigned char)commandBuffer[1] == 7)
    //volume change
        VolumeValue=(unsigned char)commandBuffer[2];

}
void PitchWheelChange()
{
    if ((unsigned char)commandBuffer[1] == 0)
    // modulation wheel change
        PitchWheelValue=(unsigned char)commandBuffer[2];

}
///////////////////
// MIDI Init
///////////////////
void initMIDI()
{
    int status;
    int mode = SND_RAWMIDI_NONBLOCK;
    const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
    if ((status = snd_rawmidi_open(&midiin, NULL, portname, mode)) < 0) {
       errormessage("Problem opening MIDI input: %s", snd_strerror(status));
       MIDIExists=false;
    }
    // Init MIDI Oscilators
    //for (int i=0; i<128;i++)
    //{
    //    osc[i]=Sine_OSC(sr);
    //}
    // Initialize the Midi Frequency table
    float bf=27.5;
    MIDIFrequencies[9]=bf;
    for (int i=8;i>-1;i--)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i+1]/pow(2,(1.0f/12.0f));
    }
    for (int i=10;i<=20;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=55;
    MIDIFrequencies[21]=bf;
    for (int i=22;i<=32;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=110;
    MIDIFrequencies[33]=bf;
    for (int i=34;i<=44;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=220;
    MIDIFrequencies[45]=bf;
    for (int i=46;i<=56;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=440;
    MIDIFrequencies[57]=bf;
    for (int i=58;i<=68;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=880;
    MIDIFrequencies[69]=bf;
    for (int i=70;i<=80;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=1760;
    MIDIFrequencies[81]=bf;
    for (int i=82;i<=92;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=3520;
    MIDIFrequencies[93]=bf;
    for (int i=94;i<=104;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=7040;
    MIDIFrequencies[105]=bf;
    for (int i=106;i<=116;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
    bf=14080;
    MIDIFrequencies[117]=bf;
    for (int i=118;i<128;i++)
    {
        MIDIFrequencies[i]=MIDIFrequencies[i-1]*pow(2,(1.0f/12.0f));
    }
}
void closeMIDI()
{
    midiin  = NULL;    // snd_rawmidi_close() does not clear invalid pointer,
    if (!MIDIExists) return;
    snd_rawmidi_close(midiin);
}
void errormessage(const char *format, ...) {
   va_list ap;
   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);
   putc('\n', stderr);
}
///////////////////
// MIDI handling
///////////////////
void handleMIDI()
{
    if (!MIDIExists) return;
    if (LooperTriggered)
    {
        //increment timer
        TriggerTime++;
        if (TriggerTime>=90)
        {
            TriggerTime=0;
            LooperTriggered=false;
        }
    }

    char buffer[1];
    int status=0;
    //int count=0;
    while (status != -EAGAIN)
    {
       status = snd_rawmidi_read(midiin, buffer, 1);
       if ((status < 0) && (status != -EBUSY) && (status != -EAGAIN)) {
          errormessage("Problem reading MIDI input: %s",snd_strerror(status));
       } else if (status >= 0) {

           //MIDI Triggered Looper
          if (TriggerLooper)
          {
            if (!LooperTriggered)
            {
                LooperTriggered=true;
                TriggerTime=0;
                LooperFlagOn=true;
            }
            return;
          }
            ////////////////////////

          commandBuffer[bufferPointer++]=buffer[0];
          if (bufferPointer==3)
          {
              if ((unsigned char)commandBuffer[0] == 0x90)
              {  // note on or note off
                  NoteOnOff();
                  bufferPointer=0;
              } else if ((unsigned char)commandBuffer[0] == 0xb0)
              {  // modulation wheel or volume change
                  ModWheelVolChange();
                  bufferPointer=0;
              } else if ((unsigned char)commandBuffer[0] == 0xe0)
              {  // pitch wheel change
                  PitchWheelChange();
                  bufferPointer=0;
              } else
              {
                  if (((unsigned char)commandBuffer[1] == 0xe0)||((unsigned char)commandBuffer[1] == 0xb0)||((unsigned char)commandBuffer[1] == 0x90))
                  {
                      commandBuffer[0]=commandBuffer[1];
                      commandBuffer[1]=commandBuffer[2];
                      bufferPointer=2;
                  } else if (((unsigned char)commandBuffer[2] == 0xe0)||((unsigned char)commandBuffer[2] == 0xb0)||((unsigned char)commandBuffer[2] == 0x90))
                  {
                      commandBuffer[0]=commandBuffer[1];
                      bufferPointer=1;
                  }
              }
          }

       }
    }

}
/////////////////
// MIDI Synth
/////////////////
// Variables
/////////////////
int MIDIPolyphony=30;
// Waveshapes
float LowOct_MIDISynth=0.3;
float HiOct_MIDISynth=0.3;
float MidOct_MIDISynth=0.5;
float VLOct_MIDISynth=0.2;
float SinMag_MIDISynth=0.8;
float SqrMag_MIDISynth=0.05;
float TriMag_MIDISynth=0.5;
float SawMag_MIDISynth=0.05;
float HarmB_MIDISynth=0.02;
float Harm1_MIDISynth=0.05;
float Harm2_MIDISynth=0.015;
float Harm3_MIDISynth=0.005;
float Harm4_MIDISynth=0.000;
float Harm5_MIDISynth=0.000;
int PitchBenderRange=2;
float CurrentPitchValue=64.0f;
///////////////
// Envelop
///////////////
float MIDIAttack=50;
float MIDISustain=10000;
float MIDIDecay=300;
float MIDIRelease=700;

