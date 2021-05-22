#include <MixEffect.h>

#include <Mixer.h>

#include <Control/Node.h>
#include <Control/MixerNode.h>
#include <Control/Generic.h>



namespace Cenital {

using namespace Zuazo;

static void setInputCount(	Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeSetter(
		&MixEffect::setInputCount,
		base, request, level, response
	);
}

static void getInputCount(	Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeGetter(
		&MixEffect::getInputCount,
		base, request, level, response
	);
}

static void setProgram(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter(
		&MixEffect::setProgram,
		base, request, level, response
	);
}

static void getProgram(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter(
		&MixEffect::getProgram,
		base, request, level, response
	);
}

static void setPreview(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter(
		&MixEffect::setPreview,
		base, request, level, response
	);
}

static void getPreview(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter(
		&MixEffect::getPreview,
		base, request, level, response
	);
}


static void cut(Zuazo::ZuazoBase& base, 
				const Control::Message& request,
				size_t level,
				Control::Message& response ) 
{
	Control::invokeSetter(
		&MixEffect::cut,
		base, request, level, response
	);
}

static void transition(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter(
		&MixEffect::transition,
		base, request, level, response
	);
}

static void setTransitionBar(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter(
		&MixEffect::setTransitionBar,
		base, request, level, response
	);
}

static void getTransitionBar(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter(
		&MixEffect::getTransitionBar,
		base, request, level, response
	);
}

static void setPreviewTransitionEnable(	Zuazo::ZuazoBase& base, 
										const Control::Message& request,
										size_t level,
										Control::Message& response ) 
{
	Control::invokeSetter<MixEffect, bool>( 
		[] (MixEffect& mixEffect, bool ena) {
			mixEffect.setTransitionSlot(ena ? MixEffect::OutputBus::PREVIEW : MixEffect::OutputBus::PROGRAM);
		},
		base, request, level, response
	);
}

static void getPreviewTransitionEnable(	Zuazo::ZuazoBase& base, 
										const Control::Message& request,
										size_t level,
										Control::Message& response ) 
{
	Control::invokeGetter<bool, MixEffect>( 
		[] (const MixEffect& mixEffect) -> bool{
			return mixEffect.getTransitionSlot() == MixEffect::OutputBus::PREVIEW;
		},
		base, request, level, response
	);
}

static void setSelectedTransition(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeSetter(
		&MixEffect::selectTransition,
		base, request, level, response
	);
}

static void getSelectedTransition(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeGetter<const std::string&, MixEffect>( 
		[] (const MixEffect& mixEffect) -> const std::string& {
			static const std::string noSelection = "";
			const auto* trans = mixEffect.getSelectedTransition();	
			return trans ? trans->getName() : noSelection;
		},
		base, request, level, response
	);
}





void MixEffect::registerCommands(Control::Node& node) {
	Control::MixerNode mixerNode(
		typeid(MixEffect),
		Control::invokeBaseConstructor<MixEffect>,
		Control::Node({
			{ "input:count",		Control::makeAttributeNode(Cenital::setInputCount, Cenital::getInputCount) },

			{ "pgm",				Control::makeAttributeNode(Cenital::setProgram, Cenital::getProgram) },
			{ "pvw",				Control::makeAttributeNode(Cenital::setPreview, Cenital::getPreview) },

			{ "cut",				Cenital::cut },
			{ "transition", 		Cenital::transition },
			{ "t-bar",				Control::makeAttributeNode(Cenital::setTransitionBar, Cenital::getTransitionBar) },

			{ "transition:pvw",		Control::makeAttributeNode(Cenital::setPreviewTransitionEnable, Cenital::getPreviewTransitionEnable) },
			{ "transition:effect",	Control::makeAttributeNode(Cenital::setSelectedTransition, Cenital::getSelectedTransition) },

		})
	);

	node.addPath("mix-effect", std::move(mixerNode));
}

}