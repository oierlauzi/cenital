#include <Mixer.h>

#include <Control/Node.h>
#include <Control/Message.h>
#include <Control/Generic.h>

#include <zuazo/Video.h>
#include <zuazo/Signal/Input.h>

namespace Cenital {

using namespace Zuazo;
using namespace Control;

class ConfigNode {
public:
	using Callback = Node::Callback;
	using PathMap = std::unordered_map<std::type_index, Callback>;

	ConfigNode() = default;
	ConfigNode(std::initializer_list<PathMap::value_type> ilist)
		: m_paths(ilist)
	{
	}

	ConfigNode(const ConfigNode& other) = default;
	ConfigNode(ConfigNode&& other) = default;
	~ConfigNode() = default;

	ConfigNode&	operator=(const ConfigNode& other) = default;
	ConfigNode&	operator=(ConfigNode&& other) = default;

	void addPath(std::type_index type, Callback callback) {
		m_paths.emplace(type, std::move(callback));
	}

	void removePath(std::type_index type) {
		m_paths.erase(type);
	}

	Callback* getPath(std::type_index type) {
		const auto ite = m_paths.find(type);
		return (ite != m_paths.cend()) ? &(ite->second) : nullptr;
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
			assert(typeid(base) == typeid(Mixer));
			auto& mixer = static_cast<Mixer&>(base);
			const auto& elementName = tokens[level];

			//Try to find the requested element
			auto* element = mixer.getElement(elementName);
			if(element) {
				//Determine the type of the element
				const auto* cbk = getPath(typeid(*element));
				if(cbk) {
					//Success! Invoke the associated callback
					Utils::invokeIf(*cbk, *element, request, level + 1, response);
				}
			}
		}
	}

private:
	PathMap			m_paths;

};



static void addElement(	const Mixer::ConstructCallback& construct,
						Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	const auto& tokens = request.getPayload();	
	if(construct) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);

		//Try to construct 
		auto element = construct(
			base.getInstance(),
			Utils::BufferView<const std::string>(
				std::next(tokens.data(), level), 
				tokens.size() - level )
		);
		const auto ret = mixer.addElement(std::move(element));

		if(ret) {
			//Successfully added
			response.setType(Message::Type::broadcast);
			response.getPayload() = tokens;
		}
	}
}

static void rmElement(	Zuazo::ZuazoBase& base, 
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

static void enumElements(	Zuazo::ZuazoBase& base, 
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

static void enumElementsOfType(	std::type_index type,
								Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == level) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<const Mixer&>(base);

		const auto elements = mixer.listElements(type);
		
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



static void setConnection(	Zuazo::ZuazoBase& base, 
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

static void getConnection(	Zuazo::ZuazoBase& base, 
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

static void unsetConnection(Zuazo::ZuazoBase& base, 
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



static void enumConnectionDst(	Zuazo::ZuazoBase& base, 
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

static void enumConnectionSrc(	Zuazo::ZuazoBase& base, 
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





void Mixer::registerCommands(Node& node) {
	Node enumNode({
		{ "all",	Cenital::enumElements }
	});


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



	node.addPath("rm", 				Cenital::rmElement);
	node.addPath("add",				Node());
	node.addPath("enum",			std::move(enumNode));
	node.addPath("config",			ConfigNode());
	node.addPath("connection", 		std::move(connectionNode));
	node.addPath("connection:dst", 	std::move(dstNode));
	node.addPath("connection:src", 	std::move(srcNode));
}

void Mixer::registerClass(	Node& node,
							std::type_index type,
							std::string name,
							ConstructCallback construct,
							Node::Callback configure )
{
	//Get all the nodes
	auto& addNode = *(node.getPath("add")->target<Node>());
	auto& enumNode = *(node.getPath("enum")->target<Node>());
	auto& configNode = *(node.getPath("config")->target<ConfigNode>());

	//Configure the add node
	addNode.addPath(
		name,
		[construct = std::move(construct)] (Zuazo::ZuazoBase& base, 
											const Message& request, 
											size_t level, 
											Message& response) 
		{
			Cenital::addElement(construct, base, request, level, response);
		}
	);

	//Configure the enum node
	enumNode.addPath(
		std::move(name),
		[type] (Zuazo::ZuazoBase& base, 
				const Message& request, 
				size_t level, 
				Message& response ) 
		{
			Cenital::enumElementsOfType(type, base, request, level, response);
		}
	);

	//Configure the config node
	configNode.addPath(type, std::move(configure));

}

}