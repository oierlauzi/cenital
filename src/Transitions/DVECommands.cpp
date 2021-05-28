#include <Transitions/DVE.h>

#include <Control/Generic.h>

using namespace Zuazo;

namespace Cenital::Transitions {

static void setScalingFilter(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter( 
		&DVE::setScalingFilter,
		base, request, level, response
	);
}

static void getScalingFilter(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter(
		&DVE::getScalingFilter,
		base, request, level, response
	);
}

static void enumScalingFilter(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::enumerate<ScalingFilter>(base, request, level, response); 
}


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
	node.addPath("scaling:filter",	Control::makeAttributeNode( Transitions::setScalingFilter,
																Transitions::getScalingFilter,
																Transitions::enumScalingFilter ));
	node.addPath("angle",		 	Control::makeAttributeNode( Transitions::setAngle,
																Transitions::getAngle ));
	node.addPath("effect",		 	Control::makeAttributeNode( Transitions::setEffect,
																Transitions::getEffect,
																Transitions::enumEffect ));

}

}