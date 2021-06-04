#include <Transitions/DVE.h>

#include <Control/Generic.h>
#include <Control/VideoScalingCommands.h>

namespace Cenital::Transitions {

using namespace Zuazo;
using namespace Control;

static void setAngle(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter( 
		&DVE::setAngle,
		base, request, level, response
	);
}

static void getAngle(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter(
		&DVE::getAngle,
		base, request, level, response
	);
}




static void setEffect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter(
		&DVE::setEffect,
		base, request, level, response
	);
}

static void getEffect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter(
		&DVE::getEffect,
		base, request, level, response
	);
}

static void enumEffect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::enumerate<DVE::Effect>(base, request, level, response);
}





void DVE::registerCommands(Control::Node& node) {
	node.addPath("angle",		 	Control::makeAttributeNode( Transitions::setAngle,
																Transitions::getAngle ));
	node.addPath("effect",		 	Control::makeAttributeNode( Transitions::setEffect,
																Transitions::getEffect,
																Transitions::enumEffect ));

	registerVideoScalingFilterAttribute<DVE>(node, true, true);
}

}