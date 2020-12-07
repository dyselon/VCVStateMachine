#include "plugin.hpp"

struct FadingLight {
    rack::engine::Light* light;
    float secToGo = -1.0f;
    float curBrightness = 0.0f;
    constexpr static const float pulseLength = 0.5f;
    constexpr static float minBrightness = 0.3f;

    void trigger() {
        secToGo = pulseLength;
    }

    void reset() {
        secToGo = -1.0f;
        curBrightness = 0.0f;
    }

    void setLight(rack::engine::Light* newLight) {
        light = newLight;
    }

    void process(const Module::ProcessArgs& args, uint frameDivision = 1) {
        if(light == nullptr) { return; }
        if(secToGo < 0.0f) { light->setBrightness(0.0f); }
        auto newBrightness = ((secToGo / pulseLength) * (1.0f - minBrightness)) + minBrightness; // lerp the brightness
        light->setBrightness(newBrightness);
        secToGo -= args.sampleTime * frameDivision;
        if(secToGo < 0.0f) { secToGo = 0.0f; } // should probably just have a separate "reset" variable rather than this <0 thing.
    }
};