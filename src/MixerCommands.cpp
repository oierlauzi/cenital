#include <Mixer.h>

#include <Control/Node.h>
#include <Control/ElementNode.h>
#include <Control/Message.h>
#include <Control/Generic.h>

#include <zuazo/Video.h>
#include <zuazo/Signal/Input.h>

namespace Cenital {

using namespace Zuazo;
using namespace Control;

static ZuazoBase* getElement(ZuazoBase& base,const std::string& name) {
	assert(typeid(base) == typeid(Mixer));
	auto& mixer = static_cast<Mixer&>(base);
	return mixer.getElement(name);
}

static void addElement(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	const auto& tokens = request.getPayload();
	if(tokens.size() > level) {
		//Find the requested type
		const auto& classIndex = controller.getClassIndex();
		const auto& typeName = tokens[level++];
		const auto* typeEntry = classIndex.find(typeName);

		if(typeEntry) {
			//Check if it can be instantiated in this context
			const auto& construct = typeEntry->getConstructCallback();
			const auto baseClass = typeEntry->getBaseClass();
			if(construct && baseClass == typeid(ZuazoBase)) {
				//Try to construct 
				auto element = construct(
					base.getInstance(),
					Utils::BufferView<const std::string>(
						std::next(tokens.data(), level), 
						tokens.size() - level)
				);

				if(element) {
					//Add it to the mixer
					assert(typeid(base) == typeid(Mixer));
					auto& mixer = static_cast<Mixer&>(base);
					const auto ret = mixer.addElement(std::move(element));

					if(ret) {
						//Successfully added. Elaborate the response
						response.setType(Message::Type::broadcast);
						response.getPayload() = tokens;
					}
				}

				
			}
		}
	}
}

static void rmElement(	Controller&,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == level + 1) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);

		const auto ret = mixer.eraseElement(tokens[level]);
		if(ret) {
			response.setType(Message::Type::broadcast);
			response.getPayload() = request.getPayload();
		}
	}
}

static void enumElements(	Controller&,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == level) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<const Mixer&>(base);

		const auto elements = mixer.listElements();
		
		std::vector<std::string>& payload = response.getPayload();
		payload.clear();
		payload.reserve(elements.size());
		std::transform(
			elements.cbegin(), elements.cend(),
			std::back_inserter(payload),
			[] (const ZuazoBase& el) -> std::string {
				return el.getName();
			}
		);

		response.setType(Message::Type::response);
	}
}



static void setConnection(	Controller&,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == (level + 4)) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& dstName = tokens[level + 0];
		const auto& dstPort = tokens[level + 1];
		const auto& srcName = tokens[level + 2];
		const auto& srcPort = tokens[level + 3];

		const auto ret = mixer.connect(dstName, dstPort, srcName, srcPort);

		if(ret) {
			response.getPayload() = request.getPayload();
			response.setType(Message::Type::broadcast);
		}
	}
}

static void getConnection(	Controller&,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == (level + 2)) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<Mixer&>(base);
		const auto& dstName = tokens[level + 0];
		const auto& dstPort = tokens[level + 1];

		//Obtain the referred elements
		const auto* dst = mixer.getElement(dstName);
		if(dst) {
			//Obtain the referred pad
			const auto* dstPad = dst->getPad<Signal::Input<Video>>(dstPort);
			if(dstPad) {
				//Pad exists, obtain the source
				const auto* dstPadSrc = dstPad->getSource();

				std::vector<std::string>& payload = response.getPayload();
				payload.clear();
				if(dstPadSrc) {
					payload = { 
						dstPadSrc->getLayout().getName(),
						dstPadSrc->getName()
					};
				}

				response.setType(Message::Type::response);
			}	

		}
	}
}

static void unsetConnection(Controller&,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == (level + 2)) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& dstName = tokens[level + 0];
		const auto& dstPort = tokens[level + 1];

		const auto ret = mixer.disconnect(dstName, dstPort);

		if(ret) {
			response.getPayload() = request.getPayload();
			response.setType(Message::Type::broadcast);
		}
	}
}



static void enumConnectionDst(	Controller&,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == (level + 1)) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<const Mixer&>(base);

		const auto* element = mixer.getElement(tokens[level]);
		if(element) {
			const auto pads = element->getPads<Signal::Input<Video>>();

			std::vector<std::string>& payload = response.getPayload();
			payload.clear();
			payload.reserve(pads.size());
			std::transform(
				pads.cbegin(), pads.cend(),
				std::back_inserter(payload),
				[] (const Signal::PadProxy<Signal::Input<Video>>& el) -> std::string {
					return el.getName();
				}
			);

			response.setType(Message::Type::response);
		}
	}
}

static void enumConnectionSrc(	Controller&,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == (level + 1)) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<const Mixer&>(base);

		const auto* element = mixer.getElement(tokens[level]);
		if(element) {
			const auto pads = element->getPads<Signal::Output<Video>>();

			std::vector<std::string>& payload = response.getPayload();
			payload.clear();
			payload.reserve(pads.size());
			std::transform(
				pads.cbegin(), pads.cend(),
				std::back_inserter(payload),
				[] (const Signal::PadProxy<Signal::Output<Video>>& el) -> std::string {
					return el.getName();
				}
			);

			response.setType(Message::Type::response);
		}
	}
}





void Mixer::registerCommands(Controller& controller) {
	auto& rootNode = controller.getRootNode();

	auto addNode = std::bind(
		&Cenital::addElement, 
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3,
		std::placeholders::_4,
		std::placeholders::_5
	);

	auto connectionNode = makeAttributeNode(
		Cenital::setConnection,	//Set
		Cenital::getConnection,	//Get
		{},						//Enum
		Cenital::unsetConnection//Unset
	);

	Node dstNode({
		{ "enum",	Cenital::enumConnectionDst }
	});

	Node srcNode({
		{ "enum",	Cenital::enumConnectionSrc }
	});



	rootNode.addPath("rm", 				Cenital::rmElement);
	rootNode.addPath("add",				std::move(addNode));
	rootNode.addPath("enum",			Cenital::enumElements);
	rootNode.addPath("config",			ElementNode(Cenital::getElement));
	rootNode.addPath("connection", 		std::move(connectionNode));
	rootNode.addPath("connection:dst", 	std::move(dstNode));
	rootNode.addPath("connection:src", 	std::move(srcNode));



	//Register it
	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(Mixer),
		ClassIndex::Entry(
			"mixer",
			{},
			{},
			typeid(void)
		)	
	);
}

}