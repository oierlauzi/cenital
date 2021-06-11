#include <Transitions/DVE.h>

#include <Control/Generic.h>
#include <Control/VideoScalingCommands.h>

namespace Cenital::Transitions {

using namespace Zuazo;
using namespace Control;

static void setAngle(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter( 
		&DVE::setAngle,
		controller, base, request, level, response
	);
}

static void getAngle(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeGetter(
		&DVE::getAngle,
		controller, base, request, level, response
	);
}




static void setEffect(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter(
		&DVE::setEffect,
		controller, base, request, level, response
	);
}

static void getEffect(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeGetter(
		&DVE::getEffect,
		controller, base, request, level, response
	);
}

static void enumEffect(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	enumerate<DVE::Effect>(controller, base, request, level, response);
}





void DVE::registerCommands(Controller& controller) {
	Node configNode;

	configNode.addPath("angle",		 	makeAttributeNode( 	Transitions::setAngle,
															Transitions::getAngle ));
	configNode.addPath("effect",		makeAttributeNode(	Transitions::setEffect,
															Transitions::getEffect,
															Transitions::enumEffect ));

	registerVideoScalingFilterAttribute<DVE>(configNode, true, true);

	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(DVE),
		ClassIndex::Entry(
			"dve",
			std::move(configNode),
			invokeBaseConstructor<DVE>,
			typeid(Base)
		)	
	);

}

}