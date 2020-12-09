#include "plugin.hpp"
#include "OutputTrigger.h"
#include "FadingLight.h"
#include "EdgeDetector.h"

const auto OUTPUT_STATES = 5;

struct MarkovState : Module {
	enum ParamIds {
		ACTIVATE_BUTTON,
		ADVANCE_BUTTON,
		RESET_BUTTON,
		SIGNAL_ADJUST,
		ENUMS(TRANSITION_BUTTON, OUTPUT_STATES),
		ENUMS(CHANCE_KNOB, OUTPUT_STATES),
		NUM_PARAMS
	};
	enum InputIds {
		ACTIVATE_INPUT,
		ADVANCE_INPUT,
		RESET_INPUT,
		SIGNAL_INPUT,
		ENUMS(TRANSITION_INPUT, OUTPUT_STATES),
		NUM_INPUTS
	};
	enum OutputIds {
		SIGNAL_OUTPUT,
		ENUMS(TRANSITION_OUTPUT, OUTPUT_STATES),
		NUM_OUTPUTS
	};
	enum LightIds {
		ACTIVE_LIGHT,
		ENUMS(TRANSITION_LIGHT, OUTPUT_STATES),
		NUM_LIGHTS
	};

	bool active = false;
	dsp::ClockDivider cvDivider;
	dsp::ClockDivider uiDivider;
	OutputTrigger outTriggers[OUTPUT_STATES];
	FadingLight fadingLights[OUTPUT_STATES];
	EdgeDetector activateTrigger;
	EdgeDetector advanceTrigger;
	EdgeDetector resetTrigger;

	MarkovState() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ACTIVATE_BUTTON, 0.0f, 1.0f, 0.0f);
		configParam(ADVANCE_BUTTON, 0.0f, 1.0f, 0.0f);
		configParam(RESET_BUTTON, 0.0f, 1.0f, 0.0f);
		configParam(SIGNAL_ADJUST, -5.0f, 5.0f, 0.0f);
		for(int i = 0; i < OUTPUT_STATES; i++) {
			configParam(TRANSITION_BUTTON + i, 0.0f, 1.0f, 0.0f);
			configParam(CHANCE_KNOB + i, 0.0f, 10.0f, 1.0f);
			outTriggers[i].setOutput(&outputs[TRANSITION_OUTPUT + i]);
			fadingLights[i].setLight(&lights[TRANSITION_LIGHT + i]);
		}
		activateTrigger.setInput(&inputs[ACTIVATE_INPUT]);
		advanceTrigger.setInput(&inputs[ADVANCE_INPUT]);
		resetTrigger.setInput(&inputs[RESET_INPUT]);
		cvDivider.setDivision(13);
		uiDivider.setDivision(127);
	}

	void process(const ProcessArgs& args) override {
		if(cvDivider.process()) {
			processCV(args);
		}

		processSignal(args);

		if(uiDivider.process()) {
			processUI(args);
		}

		for(int i = 0; i < OUTPUT_STATES; i++) {
			outTriggers[i].process(args);
		}	
	}

	void processSignal(const ProcessArgs& args) {
		auto signal_in = &inputs[SIGNAL_INPUT];
		auto signal_adjust = &params[SIGNAL_ADJUST];
		auto signal_out = &outputs[SIGNAL_OUTPUT];

		// if not active or nothing is connected to the output, there's nothing to do at audio rate.
		if(!(signal_out->isConnected())) { return; }

		// pass the input to the output, offset by the CV.
		const auto knob_val = signal_adjust->getValue();
		if(!signal_in->isConnected()) {
			auto current_output = active ? knob_val : 0.0f;
			signal_out->setVoltage(current_output);
		}
		signal_out->setChannels(signal_in->getChannels());
		for(auto ch = signal_in->getChannels() - 1; ch >= 0; ch--) {
			float current_output = signal_in->getVoltage(ch) + knob_val;
			if(!active) { current_output = 0.0f; }
			signal_out->setVoltage(current_output);
		}
	}

	void processCV(const ProcessArgs& args) {
		// if reset, deactivate
		bool reset_button_smashed = params[RESET_BUTTON].getValue() > 0.1f;
		bool reset_triggered = resetTrigger.process();
		if(reset_button_smashed || reset_triggered) {
			active = false;
		}
		
		// if actived, set it as active so the rest of the stuff works.
		bool activate_button_smashed = params[ACTIVATE_BUTTON].getValue() > 0.1f;
		bool activate_triggered = activateTrigger.process();
		if(activate_button_smashed || activate_triggered) {
			active = true;
		}

		if(active) {
			// if active and advanced, trigger a following state, and deactivate
			bool advance_button_smashed = params[ADVANCE_BUTTON].getValue() > 0.1f;
			bool advance_triggered = advanceTrigger.process();
			if(advance_button_smashed || advance_triggered) {
				uint nextState = selectRandomOutput();
				advance(nextState);
			}

			// if any of the manual advance buttons are hit
			for(int i = 0; i < OUTPUT_STATES; i++) {
				bool tx_button = params[TRANSITION_BUTTON + i].getValue() > 0.1f;
				if(tx_button) { advance(i); }
			}
		}
	}

	uint selectRandomOutput() {
		float chances[5];
		float total = 0.0f;
		for(uint i = 0; i < 5; i++) {
			auto weighted = params[CHANCE_KNOB + i].getValue();
			if(inputs[TRANSITION_INPUT + i].isConnected()) {
				weighted += inputs[TRANSITION_INPUT + i].getVoltage();
			}
			if(weighted < 0.0f) { weighted = 0.0f; }
			chances[i] = weighted + total;
			total = chances[i];
		}
		float selectionWeighted = random::uniform() * total;
		for(uint i = 0; i < 5; i++) {
			if(selectionWeighted < chances[i] && outputs[TRANSITION_OUTPUT + i].isConnected()) { return i; }
		}
		return 0;
	}

	void advance(uint outState) {
		active = false;
		outTriggers[outState].trigger();
		for(int i = 0; i < OUTPUT_STATES; i++) {
			fadingLights[i].reset();
		}
		fadingLights[outState].trigger();
	}

	void processUI(const ProcessArgs& args) {
		// update UI
		auto light = &lights[ACTIVE_LIGHT];
		auto intensity = active ? 1.0f : 0.0f;
		light->setBrightness(intensity);

		// update the transition lights
		for(int i = 0; i < OUTPUT_STATES; i++) {
			fadingLights[i].process(args, 127);
		}

		// TODO: update random counters (woof)
	}
};


struct MarkovStateWidget : ModuleWidget {
	MarkovStateWidget(MarkovState* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MarkovState.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<LEDButton>(mm2px(Vec(9.158, 24)), module, MarkovState::ACTIVATE_BUTTON));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(17.512, 24)), module, MarkovState::ADVANCE_BUTTON));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(25.74, 24)), module, MarkovState::RESET_BUTTON));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(43.524, 34.384)), module, MarkovState::SIGNAL_ADJUST));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.454, 49.472)), module, MarkovState::TRANSITION_BUTTON + 0));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(43.451, 49.472)), module, MarkovState::CHANCE_KNOB + 0));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.454, 64.234)), module, MarkovState::TRANSITION_BUTTON + 1));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(43.451, 64.234)), module, MarkovState::CHANCE_KNOB + 1));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.454, 79.221)), module, MarkovState::TRANSITION_BUTTON + 2));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(43.451, 79.221)), module, MarkovState::CHANCE_KNOB + 2));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.454, 93.982)), module, MarkovState::TRANSITION_BUTTON + 3));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(43.451, 93.982)), module, MarkovState::CHANCE_KNOB + 3));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(8.454, 109.092)), module, MarkovState::TRANSITION_BUTTON + 4));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(43.451, 109.092)), module, MarkovState::CHANCE_KNOB + 4));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.187, 34.384)), module, MarkovState::SIGNAL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.194, 35.052)), module, MarkovState::ACTIVATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.548, 35.078)), module, MarkovState::ADVANCE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.777, 35.078)), module, MarkovState::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.114, 49.472)), module, MarkovState::TRANSITION_INPUT + 0));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.114, 64.234)), module, MarkovState::TRANSITION_INPUT + 1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.114, 79.221)), module, MarkovState::TRANSITION_INPUT + 2));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.114, 93.982)), module, MarkovState::TRANSITION_INPUT + 3));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.114, 109.092)), module, MarkovState::TRANSITION_INPUT + 4));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.951, 34.384)), module, MarkovState::SIGNAL_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.878, 49.472)), module, MarkovState::TRANSITION_OUTPUT + 0));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.878, 64.234)), module, MarkovState::TRANSITION_OUTPUT + 1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.878, 79.221)), module, MarkovState::TRANSITION_OUTPUT + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.878, 93.982)), module, MarkovState::TRANSITION_OUTPUT + 3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.878, 109.092)), module, MarkovState::TRANSITION_OUTPUT + 4));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(9.158, 24)), module, MarkovState::ACTIVE_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.454, 49.472)), module, MarkovState::TRANSITION_LIGHT + 0));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.454, 64.234)), module, MarkovState::TRANSITION_LIGHT + 1));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.454, 79.221)), module, MarkovState::TRANSITION_LIGHT + 2));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.454, 93.982)), module, MarkovState::TRANSITION_LIGHT + 3));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.454, 109.092)), module, MarkovState::TRANSITION_LIGHT + 4));
	}
};


Model* modelMarkovState = createModel<MarkovState, MarkovStateWidget>("MarianConfections-MarkovState");