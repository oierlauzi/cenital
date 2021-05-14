#include <MixEffect.h>

#include <Mixer.h>
#include <MixerNode.h>

#include <ControllerFunction.h>



namespace Cenital {

using namespace Zuazo;

static Controller::Result setInputCount(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&, size_t) = [] (MixEffect& mixEffect, size_t cnt) -> void {
		mixEffect.setInputCount(cnt);
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result getInputCount(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr size_t (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> size_t {
		return mixEffect.getInputCount();
	};
	return Cenital::invokeGetter(fn, base, tokens);
}


static Controller::Result setProgram(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&, size_t) = [] (MixEffect& mixEffect, size_t idx) -> void {
		mixEffect.setProgram(idx);
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result getProgram(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr size_t (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> size_t {
		return mixEffect.getProgram();
	};
	return Cenital::invokeGetter(fn, base, tokens);
}

static Controller::Result setPreview(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&, size_t) = [] (MixEffect& mixEffect, size_t idx) -> void {
		mixEffect.setPreview(idx);
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result getPreview(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr size_t (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> size_t {
		return mixEffect.getPreview();
	};
	return Cenital::invokeGetter(fn, base, tokens);
}


static Controller::Result cut(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> void {
		mixEffect.cut();
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result transition(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> void {
		mixEffect.transition();
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result setTransitionBar(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&, float) = [] (MixEffect& mixEffect, float t) -> void {
		mixEffect.setTransitionBar(t);
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result getTransitionBar(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr float (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> float {
		return mixEffect.getTransitionBar();
	};
	return Cenital::invokeGetter(fn, base, tokens);
}

static Controller::Result setPreviewTransitionEnable(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&, bool) = [] (MixEffect& mixEffect, bool preview) -> void {
		mixEffect.setTransitionSlot(preview ? MixEffect::OutputBus::PREVIEW : MixEffect::OutputBus::PROGRAM);
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result getPreviewTransitionEnable(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr bool (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> bool {
		return mixEffect.getTransitionSlot() == MixEffect::OutputBus::PREVIEW;
	};
	return Cenital::invokeGetter(fn, base, tokens);
}

static Controller::Result setSelectedTransition(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr void (*fn)(MixEffect&, std::string_view) = [] (MixEffect& mixEffect, std::string_view name) -> void {
		mixEffect.selectTransition(name);
	};
	return Cenital::invokeSetter(fn, base, tokens);
}

static Controller::Result getSelectedTransition(ZuazoBase& base, Controller::TokenArray tokens) {
	constexpr std::string_view (*fn)(MixEffect&) = [] (MixEffect& mixEffect) -> std::string_view {
		constexpr std::string_view noSelection = "";

		const auto* trans = mixEffect.getSelectedTransition();	
		return trans ? trans->getName() : noSelection;
	};
	return Cenital::invokeGetter(fn, base, tokens);
}




void MixEffect::registerCommands(Controller::Node& node) {
	Controller::Node meNode = {
		{ "setInputCount",				Cenital::setInputCount },
		{ "getInputCount",				Cenital::getInputCount },

		{ "setProgram",					Cenital::setProgram },
		{ "getProgram", 				Cenital::getProgram },
		{ "setPreview",					Cenital::setPreview },
		{ "getPreview", 				Cenital::getPreview },

		{ "cut",						Cenital::cut },
		{ "transition", 				Cenital::transition },
		{ "setTBar",					Cenital::setTransitionBar },
		{ "getTBar", 					Cenital::getTransitionBar },
		{ "setPreviewTransitionEnable",	Cenital::setPreviewTransitionEnable },
		{ "getPreviewTransitionEnable", Cenital::getPreviewTransitionEnable },
		{ "setSelectedTransition",		Cenital::setSelectedTransition },
		{ "getSelectedTransition", 		Cenital::getSelectedTransition },
	};

	node.addPath("mixEffect", MixerNode<MixEffect>(std::move(meNode)));
}

}