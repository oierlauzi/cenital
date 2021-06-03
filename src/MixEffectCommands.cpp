#include <MixEffect.h>

#include <Mixer.h>
#include <Transitions/Mix.h>
#include <Transitions/DVE.h>

#include <Control/VideoModeCommands.h>
#include <Control/MixerNode.h>
#include <Control/Generic.h>

#include <unordered_map>
#include <typeindex>

namespace Cenital {

using namespace Zuazo;
using namespace Control;

class TransitionEffectConfigNode {
public:
	using Callback = Node::Callback;
	using PathMap = std::unordered_map<std::type_index, Callback>;

	TransitionEffectConfigNode() = default;
	TransitionEffectConfigNode(std::initializer_list<PathMap::value_type> ilist)
		: m_paths(ilist)
	{
	}

	TransitionEffectConfigNode(const TransitionEffectConfigNode& other) = default;
	TransitionEffectConfigNode(TransitionEffectConfigNode&& other) = default;
	~TransitionEffectConfigNode() = default;

	TransitionEffectConfigNode&	operator=(const TransitionEffectConfigNode& other) = default;
	TransitionEffectConfigNode&	operator=(TransitionEffectConfigNode&& other) = default;

	void addPath(std::type_index type, Callback callback) {
		m_paths.emplace(type, std::move(callback));
	}

	void removePath(std::type_index type) {
		m_paths.erase(type);
	}

	const Callback* getPath(std::type_index type) const {
		const auto ite = m_paths.find(type);
		return (ite != m_paths.cend()) ? &(ite->second) : nullptr;
	}



	void operator()(Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) const
	{
		const auto& tokens = request.getPayload();

		if(tokens.size() > level) {
			assert(typeid(base) == typeid(MixEffect));
			auto& mixEffect = static_cast<MixEffect&>(base);
			const auto& transitionName = tokens[level];

			//Try to find the requested transition
			auto* transition = mixEffect.getTransition(transitionName);
			if(transition) {
				//Determine the type of the transion
				const auto* cbk = getPath(typeid(*transition));
				if(cbk) {
					//Success! Invoke the associated callback
					Utils::invokeIf(*cbk, *transition, request, level + 1, response);
				}
			}
		}
	}

private:
	PathMap			m_paths;

};


class OverlayNode {
public:
	using Callback = Node::Callback;

	explicit OverlayNode(MixEffect::OverlaySlot slot, Callback callback = {})
		: m_slot(slot)
		, m_callback(std::move(callback))
	{
	}

	OverlayNode(const OverlayNode& other) = default;
	OverlayNode(OverlayNode&& other) = default;
	~OverlayNode() = default;

	OverlayNode&	operator=(const OverlayNode& other) = default;
	OverlayNode&	operator=(OverlayNode&& other) = default;



	void setSlot(MixEffect::OverlaySlot slot) noexcept {
		m_slot = slot;
	}

	MixEffect::OverlaySlot getSlot() const noexcept {
		return m_slot;
	}


	void setCallback(Callback cbk) {
		m_callback = std::move(cbk);
	}

	const Callback& getCallback() const noexcept {
		return m_callback;
	}


	void operator()(Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) const
	{
		const auto& tokens = request.getPayload();

		if(tokens.size() > level) {
			assert(typeid(base) == typeid(MixEffect));
			auto& mixEffect = static_cast<MixEffect&>(base);

			//Obtain the index of the keyer
			size_t index = mixEffect.getOverlayCount(getSlot());
			fromString(tokens[level], index);
			if(index < mixEffect.getOverlayCount(getSlot())) {
				//Success! this keyer exists
				auto& keyer = mixEffect.getOverlay(getSlot(), index);
				Utils::invokeIf(getCallback(), keyer, request, level + 1, response);
			}
		}
	}

private:
	MixEffect::OverlaySlot	m_slot;
	Callback				m_callback;

};



static void setScalingMode(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MixEffect, ScalingMode>(
		&MixEffect::setScalingMode,
		base, request, level, response
	);
}

static void getScalingMode(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<ScalingMode, MixEffect>(
		&MixEffect::getScalingMode,
		base, request, level, response
	);
}

static void enumScalingMode(Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	enumerate<ScalingMode>(base, request, level, response);
}


static void setScalingFilter(	Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter<MixEffect, ScalingFilter>(
		&MixEffect::setScalingFilter,
		base, request, level, response
	);
}

static void getScalingFilter(	Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter<ScalingFilter, MixEffect>(
		&MixEffect::getScalingFilter,
		base, request, level, response
	);
}

static void enumScalingFilter(	Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	enumerate<ScalingFilter>(base, request, level, response);
}


static void setInputCount(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter(
		&MixEffect::setInputCount,
		base, request, level, response
	);
}

static void getInputCount(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter(
		&MixEffect::getInputCount,
		base, request, level, response
	);
}

static void setProgram(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter<MixEffect, size_t>(
		std::bind(&MixEffect::setBackground, std::placeholders::_1, MixEffect::OutputBus::program, std::placeholders::_2),
		base, request, level, response
	);
}

static void getProgram(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeGetter<size_t, MixEffect>(
		std::bind(&MixEffect::getBackground, std::placeholders::_1, MixEffect::OutputBus::program),
		base, request, level, response
	);
}

static void setPreview(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter<MixEffect, size_t>(
		std::bind(&MixEffect::setBackground, std::placeholders::_1, MixEffect::OutputBus::preview, std::placeholders::_2),
		base, request, level, response
	);
}

static void getPreview(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeGetter<size_t, MixEffect>(
		std::bind(&MixEffect::getBackground, std::placeholders::_1, MixEffect::OutputBus::preview),
		base, request, level, response
	);
}


static void cut(Zuazo::ZuazoBase& base, 
				const Message& request,
				size_t level,
				Message& response ) 
{
	invokeSetter(
		&MixEffect::cut,
		base, request, level, response
	);
}

static void transition(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter(
		&MixEffect::transition,
		base, request, level, response
	);
}

static void setTransitionBar(	Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&MixEffect::setTransitionBar,
		base, request, level, response
	);
}

static void getTransitionBar(	Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter(
		&MixEffect::getTransitionBar,
		base, request, level, response
	);
}

static void setTransitionPreview(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter<MixEffect, bool>( 
		[] (MixEffect& mixEffect, bool ena) {
			mixEffect.setTransitionSlot(ena ? MixEffect::OutputBus::preview : MixEffect::OutputBus::program);
		},
		base, request, level, response
	);
}

static void getTransitionPreview(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter<bool, MixEffect>( 
		[] (const MixEffect& mixEffect) -> bool{
			return mixEffect.getTransitionSlot() == MixEffect::OutputBus::preview;
		},
		base, request, level, response
	);
}



static void setTransitionEffect(Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&MixEffect::setSelectedTransition,
		base, request, level, response
	);
}

static void getTransitionEffect(Zuazo::ZuazoBase& base, 
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
		base, request, level, response
	);
}

static void enumTransitionEffect(Zuazo::ZuazoBase& base, 
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

static void unsetTransitionEffect(Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter<MixEffect>( 
		[] (MixEffect& mixEffect) -> void {
			mixEffect.setSelectedTransition("");
		},
		base, request, level, response
	);
}



static void setUpstreamOverlayCount(Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter<MixEffect, size_t>(
		std::bind(&MixEffect::setOverlayCount, std::placeholders::_1, MixEffect::OverlaySlot::upstream, std::placeholders::_2),
		base, request, level, response
	);
}

static void getUpstreamOverlayCount(Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter<size_t, MixEffect>(
		std::bind(&MixEffect::getOverlayCount, std::placeholders::_1, MixEffect::OverlaySlot::upstream),
		base, request, level, response
	);
}


static void setDownstreamOverlayCount(	Zuazo::ZuazoBase& base, 
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeSetter<MixEffect, size_t>(
		std::bind(&MixEffect::setOverlayCount, std::placeholders::_1, MixEffect::OverlaySlot::downstream, std::placeholders::_2),
		base, request, level, response
	);
}

static void getDownstreamOverlayCount(	Zuazo::ZuazoBase& base, 
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeGetter<size_t, MixEffect>(
		std::bind(&MixEffect::getOverlayCount, std::placeholders::_1, MixEffect::OverlaySlot::downstream),
		base, request, level, response
	);
}





void MixEffect::registerCommands(Node& node) {
	//Get transition related nodes
	Node mixTransitionNode;
	Transitions::Mix::registerCommands(mixTransitionNode);
	Node dveTransitionNode;
	Transitions::DVE::registerCommands(dveTransitionNode);
	
	//Configure the transition node
	auto transitionEffectNode = makeAttributeNode(	
		Cenital::setTransitionEffect, 
		Cenital::getTransitionEffect, 
		Cenital::enumTransitionEffect, 
		Cenital::unsetTransitionEffect
	);
	transitionEffectNode.addPath(
		"config",
		TransitionEffectConfigNode({
			{ typeid(Transitions::Mix), std::move(mixTransitionNode) },
			{ typeid(Transitions::DVE), std::move(dveTransitionNode) },
		})
	);


	//Get the keyer related node
	Node keyerNode({
		{ "visible",		{} }, //TODO
		{ "transition",		{} }, //TODO
		{ "key",			{} }, //TODO
		{ "fill",			{} }, //TODO
	});
	Keyer::registerCommands(keyerNode);


	//Configure the overlay nodes
	OverlayNode upstreamOverlayNode(MixEffect::OverlaySlot::upstream, keyerNode);
	OverlayNode downstreamOverlayNode(MixEffect::OverlaySlot::downstream, std::move(keyerNode));

	Node aimNode({
		{ "scaling:mode",		makeAttributeNode(	Cenital::setScalingMode,
													Cenital::getScalingMode,
													Cenital::enumScalingMode) },
		{ "scaling:filter",		makeAttributeNode(	Cenital::setScalingFilter,
													Cenital::getScalingFilter,
													Cenital::enumScalingFilter) },

		{ "input:count",		makeAttributeNode(	Cenital::setInputCount, 
													Cenital::getInputCount) },

		{ "pgm",				makeAttributeNode(	Cenital::setProgram, 
													Cenital::getProgram) },
		{ "pvw",				makeAttributeNode(	Cenital::setPreview, 
													Cenital::getPreview) },

		{ "cut",				Cenital::cut },
		{ "transition", 		Cenital::transition },
		{ "transition:bar",		makeAttributeNode(	Cenital::setTransitionBar, 
													Cenital::getTransitionBar) },

		{ "transition:pvw",		makeAttributeNode(	Cenital::setTransitionPreview, 
													Cenital::getTransitionPreview) },
		{ "transition:effect",	std::move(transitionEffectNode) },

		{ "us-overlay",			std::move(upstreamOverlayNode) },
		{ "us-overlay:count",	makeAttributeNode(	Cenital::setUpstreamOverlayCount, 
													Cenital::getUpstreamOverlayCount) },
		{ "ds-overlay",			std::move(downstreamOverlayNode) },
		{ "ds-overlay:count",	makeAttributeNode(	Cenital::setDownstreamOverlayCount, 
													Cenital::getDownstreamOverlayCount) },

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

	registerVideoModeCommands<MixEffect>(aimNode, videoModeWr, videoModeRd);

	MixerNode mixerNode(
		typeid(MixEffect),
		invokeBaseConstructor<MixEffect>,
		std::move(aimNode)
	);


	node.addPath("mix-effect", std::move(mixerNode));
}

}