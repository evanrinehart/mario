#include <stdio.h>
#include <math.h>

/* audio processing unit */


// 0 to 1, repeats,
float phase = 0.0;
// phase increment between samples, 1.0 represents oscillation equal to sample rate
float dt = 220.0 / 44100.0;
// simply silences the square wave if 1
int enable = 0;

float polyblep(float dt, float t){
    if(t < dt){
        t /= dt;
        return t+t - t*t - 1.0;
    }
    else if(1.0 - dt < t){
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }
    else
        return 0.0;
}

float squareWave(float dt, float t){
    float value = t < 0.5 ? 1.0 : -1.0;
    value += polyblep(dt, t);
    value -= polyblep(dt, fmod(t + 0.5, 1.0));
    return value;
}

// makes timer signal every (N+1) / 179.MHz
void setFrequency(float f){
    dt = f / 44100.0;
}

void setEnable(int en){
    if(enable == en) return;
    enable = en;
    //if(en == 0) phase = 0.0;
}






/* programmable timer
this device counts down from N+1 to zero then resets.
N is configured in the control register for the relevant channel
since it's driven by the CPU clock, the frequency is 1.79MHz / (N+1)
it is used to drive the waveform generators, and so their frequency.
*/

/* length counter
a 60Hz counter which counts down from some value to zero.
used to stop waveform generators when a note ends.
can be disabled.
*/

/* 4-bit DAC
4 input bits determine an output voltage, which goes to the mixers.
The inputs can come directly from a generator (triangle) or be gated
by an envelope generator (square, noise). The output is one of 16 values,
but depending on signal amplitude we will have varying DC bias which
needs to be dynamically removed before output.
*/

/* volume / envelope decay unit
has 2 modes. mode 1: outputs a constant 4-bit volume level
mode 2: a 4-bit counter decreases at a configurable rate, decreasing
the volume level each time until zero. Can be configured to loop back
to 1111 or remain at zero.
*/

/* sweep unit
a counter which, if enabled, increases or decreases and updates some
frequency register to make a sweep effect. It stops when the connected
length counter reaches zero or sweep increases to max (carry detected).
if sweep increases to max the channel is silenced.
*/

/* sequencer
outputs a sequence of bits as it clocks, the sequence depends on one
of 4 options. This is used to gate the envelope signal in the pulse
wave generators.
*/

/*

output = snd0 + snd1 // actually a more complex formula
snd0 = square0 + square1
square0 = gate(sequencer0, v1)
v1 = gate(sweep0silence, envelope0)
envelope = depends on mode, either constant volume or current level
sequencer0 = dutyCycle[x][i]   i increments when timer0 goes from 0 to N+1
timer0 = a counter with period (N + 1) / 1.79MHz



triangle equations
output = sequencer(v1, {0,1,2,3,...F,F,E,D,C,...0}) :: V
v1 = gate(v2, lenc) :: E
v2 = gate(v3, linc) :: E
v3 = timer(clock1, N+1) :: E
lenc = counter(L) :: V
linc = counter(L) :: V

events - occur at a point in time, or periodically
values/voltages - hold a value over a period of time and can be sampled

clock1 - the 1.79MHz CPU clock, an event
apuClk - 

*/


struct TriangleGenerator {
    unsigned char counter; // 5 bits
    unsigned char output; // 4 bits
    // input1: length counter OR linear counter == 0
    // input2: programmable timer terminal count
};

struct TriangleGenerator tri = {0,0};

// happens when programmer timer reaches 0
// unless length counter OR linear counter are zero
void clockTriangleGenerator(struct TriangleGenerator *g){
    g->counter = (g->counter + 1) & 0x1f;
    g->output = ((g->counter >> 4) ? 0xf : 0x0) ^ (g->counter & 0xf);
}

void test(){
    for(int i = 0; i < 50; i++){
        printf("counter = %02x, output = %02x\n", tri.counter, tri.output);
        clockTriangleGenerator(&tri);
    }
}




// generate numSamples more samples worth of output
// each sample is 1/44100 seconds of time
// events might occur between samples
// in 1/44100 seconds, the CPU cycles 40.4595 times
void synth(float *out, int numSamples){

    if(enable){
        for(int i = 0; i < numSamples; i++){
            out[i] = 0.1 * squareWave(dt, phase);
            phase += dt;
            if(phase > 1.0) phase -= 1.0;
        }
    }
    else {
        for(int i = 0; i < numSamples; i++){
            out[i] = 0.0;
        }
    }

}

