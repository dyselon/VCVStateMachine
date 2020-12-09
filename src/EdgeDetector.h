#include "plugin.hpp"

struct EdgeDetector {
    rack::engine::Input* input;
    bool active = false;

    void setInput(rack::engine::Input* newInput) {
        input = newInput;
    }

    bool process() {
        if(input == nullptr || !input->isConnected()) { return false; }
        auto curVal = input->getVoltage();
        if(active == false && curVal >= 1.0f) {
            active = true;
            return true;
        }
        if(active == true && curVal <= 0.1f) {
            active = false;
        }
        return false;
    }
};