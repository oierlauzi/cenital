#include <Mixer.h>

#include <Control/Node.h>
#include <Control/Message.h>

#include <zuazo/Video.h>
#include <zuazo/Signal/Input.h>

namespace Cenital {

using namespace Zuazo;

static void listElements(	Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
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

		response.setType(Control::Message::Type::RESPONSE);
	}
}

static void listInputs(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
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
				[] (const Signal::Layout::PadProxy<Signal::Input<Video>>& el) -> std::string {
					return el.getName();
				}
			);

			response.setType(Control::Message::Type::RESPONSE);
		}
	}
}

static void listOutputs(Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
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
				[] (const Signal::Layout::PadProxy<Signal::Output<Video>>& el) -> std::string {
					return el.getName();
				}
			);

			response.setType(Control::Message::Type::RESPONSE);
		}
	}
}

static void connect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == (level + 4)) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& srcName = tokens[level + 0];
		const auto& srcPort = tokens[level + 1];
		const auto& dstName = tokens[level + 2];
		const auto& dstPort = tokens[level + 3];

		//Obtain the referred elements
		auto* src = mixer.getElement(srcName);
		auto* dst = mixer.getElement(dstName);
		if(src && dst) {
			//Obtain the referred pads. 
			auto* srcPad = src->getPad<Signal::Output<Video>>(srcPort);
			auto* dstPad = dst->getPad<Signal::Input<Video>>(dstPort);
			if(srcPad && dstPad) {
				//Pads exist, connect them
				*dstPad << *srcPad;
				response.getPayload() = request.getPayload();
				response.setType(Control::Message::Type::BROADCAST);
			}
		}
	}
}

static void disconnect(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == (level + 2)) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& dstName = tokens[level + 0];
		const auto& dstPort = tokens[level + 1];

		//Obtain the referred elements
		auto* dst = mixer.getElement(dstName);
		if(dst) {
			//Obtain the referred pad
			auto* dstPad = dst->getPad<Signal::Input<Video>>(dstPort);
			if(dstPad) {
				//Pad exists, disconnect it
				*dstPad << Signal::noSignal;
				response.getPayload() = request.getPayload();
				response.setType(Control::Message::Type::BROADCAST);
			}	
		}
	}
}

static void getSource(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
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
					payload.emplace_back(dstPadSrc->getName());
				}

				response.setType(Control::Message::Type::RESPONSE);
			}	

		}
	}
}

void Mixer::registerCommands(Control::Node& node) {
	Control::Node inputNode ({
		{ "list",	Cenital::listInputs },
		{ "source",	Cenital::getSource }
	});

	Control::Node outputNode ({
		{ "list",	Cenital::listOutputs }
	});

	node.addPath("list", 			Cenital::listElements);
	node.addPath("connect", 		Cenital::connect);
	node.addPath("disconnect", 		Cenital::disconnect);
	node.addPath("input", 			std::move(inputNode));
	node.addPath("output", 			std::move(outputNode));
}

}