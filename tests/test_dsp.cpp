#include "../Source/AcidDSP.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
    acid::Voice v;
    v.prepare(44100);
    v.noteOn(48, true, false);
    double energy = 0;
    for(int i=0;i<44100;++i) {
        float x=v.render(.3f,.75f,.7f,.35f,.75f,0,false,.18f);
        assert(std::isfinite(x));
        energy += x*x;
    }
    assert(energy > .001);
    v.noteOn(55, false, true);
    for(int i=0;i<10000;++i) assert(std::isfinite(v.render(.3f,.9f,.7f,.35f,.75f,0,true,.2f)));
    v.noteOff();
    std::cout << "DSP passed; first-note energy " << energy << "\n";
}
