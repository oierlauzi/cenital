#include <Control/MixerNode.h>

#include <Mixer.h>
#include <Control/Message.h>

namespace Cenital::Control {

using namespace Zuazo;

MixerNode::MixerNode(	std::type_index elementType,
						ConstructCallback constructCallback,
						AimCallback aimCbk )
	: m_constructCallback(std::move(constructCallback))
	, m_aimCallback(std::move(aimCbk))
	, m_type(elementType)
{
}


void MixerNode::setElementType(std::type_index type) {
	m_type = type;
}	

std::type_index MixerNode::getElementType() const noexcept {
	return m_type;
}


void MixerNode::setConstructCallback(ConstructCallback cbk) {
	m_constructCallback = std::move(cbk);
}

const MixerNode::ConstructCallback& MixerNode::getConstructCallback() const noexcept {
	return m_constructCallback;
}

void MixerNode::setAimCallback(AimCallback cbk) {
	m_aimCallback = std::move(cbk);
}

const MixerNode::AimCallback& MixerNode::getAimCallback() const noexcept {
	return m_aimCallback;
}



void MixerNode::operator()(	ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response  ) const
{
	const auto& tokens = request.getPayload();

	if(level < tokens.size()) {
		const auto& action = tokens[level];

		if(action == "help") {
			help(base, request, level + 1, response);
		} else if(action == "ping") {
			Node::ping(base, request, level + 1, response);
		} else if(action == "add") {
			add(base, request, level + 1, response);
		} else if(action == "rm") {
			rm(base, request, level + 1, response);
		} else if(action == "aim") {
			aim(base, request, level + 1, response);
		} else if(action == "ls") {
			ls(base, request, level + 1, response);
		}
	}
}



void MixerNode::help(	Zuazo::ZuazoBase&, 
						const Message& request,
						size_t level,
						Message& response  ) const
{
	const auto& tokens = request.getPayload();

	if(level == tokens.size()) {
		response.setType(Message::Type::RESPONSE);
		response.getPayload() = { "help", "ping", "add", "rm", "aim", "ls" };
	}	
}

void MixerNode::add(ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) const
{
	const auto& tokens = request.getPayload();
	const auto& construct = getConstructCallback();

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
			response.setType(Message::Type::BROADCAST);
			response.getPayload() = tokens;
		}
	}
}

void MixerNode::rm(	ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) const
{
	const auto& tokens = request.getPayload();

	if((level + 1) == tokens.size()) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& name = tokens[level];

		//Try to erase it
		auto element = mixer.eraseElement(name);
		if(element) {
			if(std::type_index(typeid(*element)) == getElementType()) {
				//Success
				response.setType(Message::Type::BROADCAST);
				response.getPayload() = tokens;
			} else {
				//Element type is not coherent. Insert it back
				const auto ret = mixer.addElement(std::move(element));
				assert(ret); Utils::ignore(ret);
			}
		}
	}
}

void MixerNode::aim(ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) const
{
	const auto& tokens = request.getPayload();
	const auto& cbk = getAimCallback();

	if(level < tokens.size() && cbk) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& name = tokens[level];

		//Try to erase it
		auto* element = mixer.getElement(name);
		if(element) {
			if(std::type_index(typeid(*element)) == getElementType()) {
				//Success
				assert(cbk); //Checked earlier
				cbk(*element, request, level + 1, response);
			}
		}
	}
}

void MixerNode::ls(	ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) const
{
	const auto& tokens = request.getPayload();

	if(level == tokens.size()) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		
		const auto elements = mixer.listElements(getElementType());
		std::vector<std::string> payload;
		payload.reserve(elements.size());
		std::transform(
			elements.cbegin(), elements.cend(),
			std::back_inserter(payload),
			[] (const ZuazoBase& base) -> const std::string& {
				return base.getName();
			}
		);

		response.setType(Message::Type::RESPONSE);
		response.getPayload() = std::move(payload);
	}	
}

}