#include <MixEffect.h>

#include <Mixer.h>

#include <Control/Node.h>
#include <Control/ElementNode.h>
#include <Control/VideoModeCommands.h>
#include <Control/VideoScalingCommands.h>
#include <Control/Generic.h>

#include <unordered_map>
#include <typeindex>

namespace Cenital {

using namespace Zuazo;
using namespace Control;



static void setInputCount(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter(
		&MixEffect::setInputCount,
		controller, base, request, level, response
	);
}

static void getInputCount(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter(
		&MixEffect::getInputCount,
		controller, base, request, level, response
	);
}



static void setBackground(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response,
							MixEffect::OutputBus bus )
{
	invokeSetter<MixEffect, size_t>(
		std::bind(&MixEffect::setBackground, std::placeholders::_1, bus, std::placeholders::_2),
		[bus] (const MixEffect& mixEffect, size_t index) -> bool {
			return index < mixEffect.getInputCount();
		},
		controller, base, request, level, response
	);
}

static void getBackground(	Controller&,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response,
							MixEffect::OutputBus bus )
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		assert(typeid(base) == typeid(MixEffect));
		const auto& mixEffect = static_cast<const MixEffect&>(base);

		//Get the index of the background for this bus
		const auto index = mixEffect.getBackground(bus);
		
		//Elaborate the response
		response.setType(Message::Type::response);
		if(index < mixEffect.getInputCount()) {
			response.getPayload() = { std::string(toString(index)) };
		} else {
			response.getPayload().clear();
		}
	}
}

static void unsetBackground(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response,
							MixEffect::OutputBus bus )
{
	invokeSetter<MixEffect>(
		std::bind(&MixEffect::setBackground, std::placeholders::_1, bus, std::numeric_limits<size_t>::max()),
		controller, base, request, level, response
	);
}

static void setProgram(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	setBackground(controller, base, request, level, response, MixEffect::OutputBus::program);
}

static void getProgram(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	getBackground(controller, base, request, level, response, MixEffect::OutputBus::program);
}

static void unsetProgram(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	unsetBackground(controller, base, request, level, response, MixEffect::OutputBus::program);
}

static void setPreview(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	setBackground(controller, base, request, level, response, MixEffect::OutputBus::preview);
}

static void getPreview(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	getBackground(controller, base, request, level, response, MixEffect::OutputBus::preview);
}

static void unsetPreview(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	unsetBackground(controller, base, request, level, response, MixEffect::OutputBus::preview);
}


static void cut(Controller& controller,
				ZuazoBase& base,
				const Message& request,
				size_t level,
				Message& response ) 
{
	invokeSetter(
		&MixEffect::cut,
		controller, base, request, level, response
	);
}

static void transition(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter(
		&MixEffect::transition,
		controller, base, request, level, response
	);
}

static void setTransitionBar(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&MixEffect::setTransitionBar,
		controller, base, request, level, response
	);
}

static void getTransitionBar(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter(
		&MixEffect::getTransitionBar,
		controller, base, request, level, response
	);
}

static void setTransitionPreview(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter<MixEffect, bool>( 
		[] (MixEffect& mixEffect, bool ena) {
			mixEffect.setTransitionSlot(ena ? MixEffect::OutputBus::preview : MixEffect::OutputBus::program);
		},
		controller, base, request, level, response
	);
}

static void getTransitionPreview(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter<bool, MixEffect>( 
		[] (const MixEffect& mixEffect) -> bool {
			return mixEffect.getTransitionSlot() == MixEffect::OutputBus::preview;
		},
		controller, base, request, level, response
	);
}

static void setTransitionDuration(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter<MixEffect, Duration>( 
		&MixEffect::setTransitionDuration,
		controller, base, request, level, response
	);
}

static void getTransitionDuration(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter<Duration, MixEffect>( 
		&MixEffect::getTransitionDuration,
		controller, base, request, level, response
	);
}




static void setTransitionEffect(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter<MixEffect, std::string_view>(
		std::bind(&MixEffect::setSelectedTransition, std::placeholders::_1, std::placeholders::_2),
		[] (const MixEffect& mixEffect, std::string_view name) -> bool {
			//List all the available transitions
			const auto transitions = mixEffect.getTransitions();

			//Try to find the requested one
			const auto ite = std::find_if(
				transitions.cbegin(), transitions.cend(),
				[name] (const auto* transition) -> bool {
					bool result = false;

					if(transition) {
						result = transition->getName() == name;
					}

					return result;
				}
			);
			return ite != transitions.cend();
		},
		controller, base, request, level, response
	);
}

static void getTransitionEffect(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter<const std::string&, MixEffect>( 
		[] (const MixEffect& mixEffect) -> const std::string& {
			static const std::string noSelection = "";
			const auto* trans = mixEffect.getSelectedTransition();	
			return trans ? trans->getName() : noSelection;
		},
		controller, base, request, level, response
	);
}

static void enumTransitionEffect(	Controller&,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == level) {
		assert(typeid(base) == typeid(MixEffect));
		const auto& mixEffect = static_cast<const MixEffect&>(base);

		const auto transitions = mixEffect.getTransitions();

		std::vector<std::string>& payload = response.getPayload();
		payload.clear();
		payload.reserve(transitions.size());
		std::transform(
			transitions.cbegin(), transitions.cend(),
			std::back_inserter(payload),
			[] (const Transitions::Base* transition) -> std::string {
				return transition ? transition->getName() : "";
			}
		);

		response.setType(Message::Type::response);
	}
}

static void unsetTransitionEffect(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter<MixEffect>( 
		[] (MixEffect& mixEffect) -> void {
			mixEffect.setSelectedTransition("");
		},
		controller, base, request, level, response
	);
}

static ZuazoBase* getTransition(ZuazoBase& base,const std::string& name) {
	assert(typeid(base) == typeid(MixEffect));
	auto& mixEffect = static_cast<MixEffect&>(base);
	return mixEffect.getTransition(name);
}



static ZuazoBase* getOverlay(ZuazoBase& base,MixEffect::OverlaySlot slot, const std::string& indexStr) {
	ZuazoBase* result = nullptr;	

	//Try to parse the index
	size_t index;
	if(fromString(indexStr, index)) {
		assert(typeid(base) == typeid(MixEffect));
		auto& mixEffect = static_cast<MixEffect&>(base);

		//Check if the index is valid
		if(index < mixEffect.getOverlayCount(slot)) {
			result = mixEffect.getOverlay(slot, index);
		}
	}

	return result;
}

static ZuazoBase* getUpstreamOverlay(ZuazoBase& base,const std::string& indexStr) {
	return getOverlay(base, MixEffect::OverlaySlot::upstream, indexStr);
}

static ZuazoBase* getDownstreamOverlay(ZuazoBase& base,const std::string& indexStr) {
	return getOverlay(base, MixEffect::OverlaySlot::downstream, indexStr);
}



static void setOverlayCount(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response,
							MixEffect::OverlaySlot slot ) 
{
	invokeSetter<MixEffect, size_t>(
		std::bind(&MixEffect::setOverlayCount, std::placeholders::_1, slot, std::placeholders::_2),
		controller, base, request, level, response
	);
}

static void getOverlayCount(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response,
							MixEffect::OverlaySlot slot ) 
{
	invokeGetter<size_t, MixEffect>(
		std::bind(&MixEffect::getOverlayCount, std::placeholders::_1, slot),
		controller, base, request, level, response
	);
}

static void setUpstreamOverlayCount(Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	setOverlayCount(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void getUpstreamOverlayCount(Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	getOverlayCount(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void setDownstreamOverlayCount(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	setOverlayCount(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}

static void getDownstreamOverlayCount(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	getOverlayCount(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}


static void setOverlayEnabled(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response,
								MixEffect::OverlaySlot slot ) 
{
	invokeSetter<MixEffect, size_t, bool>(
		std::bind(&MixEffect::setOverlayVisible, std::placeholders::_1, slot, std::placeholders::_2, std::placeholders::_3),
		[slot] (const MixEffect& mixEffect, size_t index, bool) -> bool {
			return index < mixEffect.getOverlayCount(slot);
		},
		controller, base, request, level, response
	);
}

static void getOverlayEnabled(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response,
								MixEffect::OverlaySlot slot ) 
{
	invokeGetter<bool, MixEffect, size_t>(
		std::bind(&MixEffect::getOverlayVisible, std::placeholders::_1, slot, std::placeholders::_2),
		[slot] (const MixEffect& mixEffect, size_t index) -> bool {
			return index < mixEffect.getOverlayCount(slot);
		},
		controller, base, request, level, response
	);
}

static void setUpstreamOverlayEnabled(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response) 
{
	setOverlayEnabled(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void getUpstreamOverlayEnabled(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	getOverlayEnabled(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void setDownstreamOverlayEnabled(Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response) 
{
	setOverlayEnabled(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}

static void getDownstreamOverlayEnabled(Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	getOverlayEnabled(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}


static void setOverlayTransition(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response,
									MixEffect::OverlaySlot slot ) 
{
	invokeSetter<MixEffect, size_t, bool>(
		std::bind(&MixEffect::setOverlayTransition, std::placeholders::_1, slot, std::placeholders::_2, std::placeholders::_3),
		[slot] (const MixEffect& mixEffect, size_t index, bool) -> bool {
			return index < mixEffect.getOverlayCount(slot);
		},
		controller, base, request, level, response
	);
}

static void getOverlayTransition(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response,
									MixEffect::OverlaySlot slot ) 
{
	invokeGetter<bool, MixEffect, size_t>(
		std::bind(&MixEffect::getOverlayTransition, std::placeholders::_1, slot, std::placeholders::_2),
		[slot] (const MixEffect& mixEffect, size_t index) -> bool {
			return index < mixEffect.getOverlayCount(slot);
		},
		controller, base, request, level, response
	);
}

static void setUpstreamOverlayTransition(	Controller& controller,
											ZuazoBase& base,
											const Message& request,
											size_t level,
											Message& response) 
{
	setOverlayTransition(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void getUpstreamOverlayTransition(	Controller& controller,
											ZuazoBase& base,
											const Message& request,
											size_t level,
											Message& response ) 
{
	getOverlayTransition(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void setDownstreamOverlayTransition(	Controller& controller,
											ZuazoBase& base,
											const Message& request,
											size_t level,
											Message& response) 
{
	setOverlayTransition(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}

static void getDownstreamOverlayTransition(	Controller& controller,
											ZuazoBase& base,
											const Message& request,
											size_t level,
											Message& response ) 
{
	getOverlayTransition(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}


static void setOverlayFeed(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response,
							MixEffect::OverlaySlot slot ) 
{
	invokeSetter<MixEffect, size_t, std::string_view, size_t>(
		std::bind(&MixEffect::setOverlaySignal, std::placeholders::_1, slot, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
		[slot] (const MixEffect& mixEffect, size_t index, std::string_view portName, size_t signal) -> bool {
			bool result = false;

			//Check if the overlay and its port exists
			if(index < mixEffect.getOverlayCount(slot)) {
				const auto* overlay = mixEffect.getOverlay(slot, index);
				if(overlay) {
					const auto* port = overlay->getPad<Signal::Input<Video>>(portName);
					result = port != nullptr;
				}
			}

			//Check if the signal is valid
			result = result && (signal < mixEffect.getInputCount());

			return result;
		},
		controller, base, request, level, response
	);
}

static void getOverlayFeed(	Controller&,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response,
							MixEffect::OverlaySlot slot ) 
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level + 2) {
		const auto& overlayIndexStr = tokens[level+0];
		const auto& portName = tokens[level+1];

		//Try to parse the overlay index
		size_t overlayIndex;
		if(fromString(overlayIndexStr, overlayIndex)) {
			//Check validity of the overlay index
			assert(typeid(base) == typeid(MixEffect));
			const auto& mixEffect = static_cast<const MixEffect&>(base);

			if(overlayIndex < mixEffect.getOverlayCount(slot)) {
				const auto* overlay = mixEffect.getOverlay(slot, overlayIndex);

				if(overlay) {
					//Obtain the input port
					const auto* port = overlay->getPad<Signal::Input<Video>>(portName);

					if(port) {
						//Query the signal index
						const auto signalIndex = mixEffect.getOverlaySignal(slot, overlayIndex, portName);

						//Elaborate the response
						response.setType(Message::Type::response);
						if(signalIndex < mixEffect.getInputCount()) {
							response.getPayload() = { std::string(toString(signalIndex)) };
						} else {
							response.getPayload().clear();
						}
					}
				}
			}
		}
	}
}

static void unsetOverlayFeed(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response,
								MixEffect::OverlaySlot slot ) 
{
	invokeSetter<MixEffect, size_t, std::string_view>(
		std::bind(&MixEffect::setOverlaySignal, std::placeholders::_1, slot, std::placeholders::_2, std::placeholders::_3, std::numeric_limits<size_t>::max()),
		[slot] (const MixEffect& mixEffect, size_t index, std::string_view portName) -> bool {
			bool result = false;

			//Check if the overlay and its port exists
			if(index < mixEffect.getOverlayCount(slot)) {
				const auto* overlay = mixEffect.getOverlay(slot, index);
				if(overlay) {
					const auto* port = overlay->getPad<Signal::Input<Video>>(portName);
					result = port != nullptr;
				}
			}

			return result;
		},
		controller, base, request, level, response
	);
}

static void setUpstreamOverlayFeed(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	setOverlayFeed(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void getUpstreamOverlayFeed(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	getOverlayFeed(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void unsetUpstreamOverlayFeed(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	unsetOverlayFeed(controller, base, request, level, response, MixEffect::OverlaySlot::upstream);
}

static void setDownstreamOverlayFeed(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	setOverlayFeed(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}

static void getDownstreamOverlayFeed(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	getOverlayFeed(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}

static void unsetDownstreamOverlayFeed(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	unsetOverlayFeed(controller, base, request, level, response, MixEffect::OverlaySlot::downstream);
}





void MixEffect::registerCommands(Control::Controller& controller) {
	//Configure the transition node
	auto transitionEffectNode = makeAttributeNode(	
		Cenital::setTransitionEffect, 
		Cenital::getTransitionEffect, 
		Cenital::enumTransitionEffect, 
		Cenital::unsetTransitionEffect
	);
	transitionEffectNode.addPath(
		"config", ElementNode(Cenital::getTransition)
	);

	//Configure the overlay nodes
	Node upstreamOverlayNode({
		{ "config",					ElementNode(Cenital::getUpstreamOverlay) }
	});
	Node downstreamOverlayNode({
		{ "config",					ElementNode(Cenital::getDownstreamOverlay) }
	});

	Node configNode({
		{ "input:count",			makeAttributeNode(	Cenital::setInputCount, 
														Cenital::getInputCount) },

		{ "pgm",					makeAttributeNode(	Cenital::setProgram, 
														Cenital::getProgram,
														{},
														Cenital::unsetProgram) },
		{ "pvw",					makeAttributeNode(	Cenital::setPreview, 
														Cenital::getPreview,
														{},
														Cenital::unsetPreview) },

		{ "cut",					Cenital::cut },
		{ "transition", 			Cenital::transition },
		{ "transition:bar",			makeAttributeNode(	Cenital::setTransitionBar, 
														Cenital::getTransitionBar) },

		{ "transition:pvw",			makeAttributeNode(	Cenital::setTransitionPreview, 
														Cenital::getTransitionPreview) },
		{ "transition:duration",	makeAttributeNode(	Cenital::setTransitionDuration, 
														Cenital::getTransitionDuration) },
		{ "transition:effect",		std::move(transitionEffectNode) },

		{ "us-overlay",				std::move(upstreamOverlayNode) },
		{ "ds-overlay",				std::move(downstreamOverlayNode) },
		{ "us-overlay:count",		makeAttributeNode(	Cenital::setUpstreamOverlayCount, 
														Cenital::getUpstreamOverlayCount) },
		{ "ds-overlay:count",		makeAttributeNode(	Cenital::setDownstreamOverlayCount, 
														Cenital::getDownstreamOverlayCount) },
		{ "us-overlay:ena",			makeAttributeNode(	Cenital::setUpstreamOverlayEnabled, 
														Cenital::getUpstreamOverlayEnabled ) },
		{ "ds-overlay:ena",			makeAttributeNode(	Cenital::setDownstreamOverlayEnabled, 
														Cenital::getDownstreamOverlayEnabled ) },
		{ "us-overlay:transition",	makeAttributeNode(	Cenital::setUpstreamOverlayTransition, 
														Cenital::getUpstreamOverlayTransition ) },
		{ "ds-overlay:transition",	makeAttributeNode(	Cenital::setDownstreamOverlayTransition, 
														Cenital::getDownstreamOverlayTransition ) },
		{ "us-overlay:feed",		makeAttributeNode(	Cenital::setUpstreamOverlayFeed, 
														Cenital::getUpstreamOverlayFeed,
														{},
														Cenital::unsetUpstreamOverlayFeed ) },
		{ "ds-overlay:feed",		makeAttributeNode(	Cenital::setDownstreamOverlayFeed, 
														Cenital::getDownstreamOverlayFeed,
														{},
														Cenital::unsetDownstreamOverlayFeed ) },

	});

	constexpr auto videoModeWr = 
		VideoModeAttributes::resolution |
		VideoModeAttributes::pixelAspectRatio |
		VideoModeAttributes::colorPrimaries |
		VideoModeAttributes::colorTransferFunction |
		VideoModeAttributes::colorFormat ;
	constexpr auto videoModeRd = 
		VideoModeAttributes::resolution |
		VideoModeAttributes::pixelAspectRatio |
		VideoModeAttributes::colorPrimaries |
		VideoModeAttributes::colorTransferFunction |
		VideoModeAttributes::colorFormat ;
	registerVideoModeCommands<MixEffect>(configNode, videoModeWr, videoModeRd);

	constexpr auto videoScalingWr = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	constexpr auto videoScalingRd = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	registerVideoScalingCommands<MixEffect>(configNode, videoScalingWr, videoScalingRd);

	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(MixEffect),
		ClassIndex::Entry(
			"mix-effect",
			std::move(configNode),
			invokeBaseConstructor<MixEffect>,
			typeid(ZuazoBase)
		)	
	);
}

}