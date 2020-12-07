#include "plugin.hpp"

struct OutputTrigger {
    rack::engine::Output* output;
    float secToGo = -1.0f;

    void trigger() {
        secToGo = 1.01f;
    }

    void setOutput(rack::engine::Output* newOutput) {
        output = newOutput;
    }

    void process(const Module::ProcessArgs& args) {
        if(output == nullptr || !output->isConnected()) { return; }
        if(secToGo < 0.0f) {
            output->setVoltage(0.0f);
            return;
        }
        output->setVoltage(10.0f);
        secToGo -= args.sampleTime;
    }
};