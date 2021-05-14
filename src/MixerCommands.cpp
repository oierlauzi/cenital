#include <Mixer.h>

#include <ControllerFunction.h>

#include <zuazo/Video.h>
#include <zuazo/Signal/Input.h>

namespace Cenital {

using namespace Zuazo;

static Controller::Result listElements(ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	if(tokens.empty()) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<const Mixer&>(base);

		const auto elements = mixer.listElements();
		
		std::vector<std::string> payload;
		payload.reserve(elements.size());
		std::transform(
			elements.cbegin(), elements.cend(),
			std::back_inserter(payload),
			[] (const ZuazoBase& el) -> std::string {
				return el.getName();
			}
		);

		result.setType(Controller::Result::Type::RESPONSE);
		result.setPayload(std::move(payload));
	}

	return result;
}

static Controller::Result listInputs(ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	if(tokens.size() == 1) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<const Mixer&>(base);

		const auto* element = mixer.getElement(tokens[0]);
		if(element) {
			const auto pads = element->getPads<Signal::Input<Video>>();

			std::vector<std::string> payload;
			payload.reserve(pads.size());
			std::transform(
				pads.cbegin(), pads.cend(),
				std::back_inserter(payload),
				[] (const Signal::Layout::PadProxy<Signal::Input<Video>>& el) -> std::string {
					return el.getName();
				}
			);

			result.setType(Controller::Result::Type::RESPONSE);
			result.setPayload(std::move(payload));
		}
	}

	return result;
}

static Controller::Result listOutputs(ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	if(tokens.size() == 1) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<const Mixer&>(base);

		const auto* element = mixer.getElement(tokens[0]);
		if(element) {
			const auto pads = element->getPads<Signal::Output<Video>>();

			std::vector<std::string> payload;
			payload.reserve(pads.size());
			std::transform(
				pads.cbegin(), pads.cend(),
				std::back_inserter(payload),
				[] (const Signal::Layout::PadProxy<Signal::Output<Video>>& el) -> std::string {
					return el.getName();
				}
			);

			result.setType(Controller::Result::Type::RESPONSE);
			result.setPayload(std::move(payload));
		}
	}

	return result;
}

static Controller::Result connect(ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	if(tokens.size() == 4) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& srcName = tokens[0];
		const auto& srcPort = tokens[1];
		const auto& dstName = tokens[2];
		const auto& dstPort = tokens[3];

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
				result.setType(Controller::Result::Type::SUCCESS);
			}
		}
	}

	return result;
}

static Controller::Result disconnect(ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	if(tokens.size() == 2) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& dstName = tokens[0];
		const auto& dstPort = tokens[1];

		//Obtain the referred elements
		auto* dst = mixer.getElement(dstName);
		if(dst) {
			//Obtain the referred pad
			auto* dstPad = dst->getPad<Signal::Input<Video>>(dstPort);
			if(dstPad) {
				//Pad exists, disconnect it
				*dstPad << Signal::noSignal;
				result.setType(Controller::Result::Type::SUCCESS);
			}	
		}
	}


	return result;
}

static Controller::Result getSource(ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	if(tokens.size() == 2) {
		//Some aliases
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<Mixer&>(base);
		const auto& dstName = tokens[0];
		const auto& dstPort = tokens[1];

		//Obtain the referred elements
		const auto* dst = mixer.getElement(dstName);
		if(dst) {
			//Obtain the referred pad
			const auto* dstPad = dst->getPad<Signal::Input<Video>>(dstPort);
			if(dstPad) {
				//Pad exists, obtain the source
				const auto* dstPadSrc = dstPad->getSource();
				std::string payload = dstPadSrc ? dstPadSrc->getName() : "";

				result.setType(Controller::Result::Type::RESPONSE);
				result.setPayload({ std::move(payload) });
			}	

		}
	}


	return result;
}

void Mixer::registerCommands(Controller::Node& node) {
	node.addPath("listElements", 	Cenital::listElements);
	node.addPath("listInputs", 		Cenital::listInputs);
	node.addPath("listOutputs", 	Cenital::listOutputs);
	node.addPath("connect", 		Cenital::connect);
	node.addPath("disconnect", 		Cenital::disconnect);
	node.addPath("getSource", 		Cenital::getSource);
}

}