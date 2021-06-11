#include <Transitions/Mix.h>

#include <Control/Generic.h>

namespace Cenital::Transitions {

using namespace Zuazo;
using namespace Control;

static void setEffect(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter(
		&Mix::setEffect,
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
		&Mix::getEffect,
		controller, base, request, level, response
	);
}

static void enumEffect(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	enumerate<Mix::Effect>(controller, base, request, level, response);
}




void Mix::registerCommands(Controller& controller) {
	Node configNode;

	configNode.addPath("effect", 	makeAttributeNode(	Transitions::setEffect,
														Transitions::getEffect,
														Transitions::enumEffect ));


	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(Mix),
		ClassIndex::Entry(
			"mix",
			std::move(configNode),
			invokeBaseConstructor<Mix>,
			typeid(Base)
		)	
	);

}

}