#include "plugin.hpp"

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
		NUM_LIGHTS
	};

	bool active = false;

	MarkovState() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ACTIVATE_BUTTON, 0.0f, 1.0f, 0.0f);
		configParam(ADVANCE_BUTTON, 0.0f, 1.0f, 0.0f);
		configParam(RESET_BUTTON, 0.0f, 1.0f, 0.0f);
		configParam(SIGNAL_ADJUST, -5.0f, 5.0f, 0.0f);
		for(int i = 0; i < OUTPUT_STATES; i++) {
			configParam(TRANSITION_BUTTON + i, 0.0f, 1.0f, 0.0f);
			configParam(CHANCE_KNOB + i, 1.0f, 100.0f, 1.0f);
		}
	}

	void process(const ProcessArgs& args) override {
		auto signal_out = &outputs[SIGNAL_OUTPUT];
		signal_out->setVoltage(5.0f);
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

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(9.158, 28.224)), module, MarkovState::ACTIVE_LIGHT));
	}
};


Model* modelMarkovState = createModel<MarkovState, MarkovStateWidget>("MarkovState");