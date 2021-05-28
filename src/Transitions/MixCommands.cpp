#include <Transitions/Mix.h>

#include <Control/Generic.h>

namespace Cenital::Transitions {

static void setEffect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter(
		&Mix::setEffect,
		base, request, level, response
	);
}

static void getEffect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter(
		&Mix::getEffect,
		base, request, level, response
	);
}

static void enumEffect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::enumerate<Mix::Effect>(base, request, level, response);
}




void Mix::registerCommands(Control::Node& node) {
	node.addPath("effect", 	Control::makeAttributeNode( Transitions::setEffect,
														Transitions::getEffect,
														Transitions::enumEffect ));

}

}