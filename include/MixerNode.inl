#include "MixerNode.h"

namespace Cenital {

template<typename T>
inline MixerNode<T>::MixerNode(Controller::Node::Callback targetCbk)
	: m_targetCallback(std::move(targetCbk))
{
}



template<typename T>
inline void MixerNode<T>::setTargetCallback(Controller::Node::Callback cbk) {
	m_targetCallback = std::move(cbk);
}


template<typename T>
inline const Controller::Node::Callback& MixerNode<T>::getTargetCallback() const noexcept {
	return m_targetCallback;
}



template<typename T>
inline Controller::Result MixerNode<T>::operator()(	Zuazo::ZuazoBase& base, 
														Controller::TokenArray tokens ) const
{
	Controller::Result result;

	if(tokens.size() > 0) {
		const auto& action = tokens[0];

		//Decide what to do based on the first token
		if(action == "target") {
			targetElement(
				base, 
				Controller::TokenArray(std::next(tokens.begin()), tokens.end())
			);
		} else if(action == "list") {
			listElement(
				base, 
				Controller::TokenArray(std::next(tokens.begin()), tokens.end())
			);
		} else if(action == "add") {
			addElement(
				base, 
				Controller::TokenArray(std::next(tokens.begin()), tokens.end())
			);
		} else if(action == "rm") {
			rmElement(
				base, 
				Controller::TokenArray(std::next(tokens.begin()), tokens.end())
			);
		}
	}

	return result;
}



template<typename T>
inline Controller::Result MixerNode<T>::targetElement(Zuazo::ZuazoBase& base, 
														Controller::TokenArray tokens ) const
{
	Controller::Result result;

	if(tokens.size() > 0 && m_targetCallback) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& name = tokens[0];

		auto* element = dynamic_cast<element_type*>(mixer.getElement(name));
		if(element) {
			result = m_targetCallback(
				*element, 
				Controller::TokenArray(std::next(tokens.begin()), tokens.end())
			);
		}

	}

	return result;
}

template<typename T>
inline Controller::Result MixerNode<T>::listElement(	Zuazo::ZuazoBase& base, 
														Controller::TokenArray tokens )
{
	Controller::Result result;

	if(tokens.size() == 0) {
		assert(typeid(base) == typeid(Mixer));
		const auto& mixer = static_cast<Mixer&>(base);

		const auto elements = mixer.listElements(typeid(element_type));
		std::vector<std::string> payload;
		payload.reserve(elements.size());
		std::transform(
			elements.cbegin(), elements.cend(),
			std::back_inserter(payload),
			[] (const Zuazo::ZuazoBase& base) -> const std::string& {
				return base.getName();
			}
		);

		result.setType(Controller::Result::Type::RESPONSE);
		result.setPayload(std::move(payload));
	}

	return result;
}


template<typename T>
inline Controller::Result MixerNode<T>::addElement(	Zuazo::ZuazoBase& base, 
														Controller::TokenArray tokens )
{
	Controller::Result result;

	if(tokens.size() == 1) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& name = tokens[0];

		//Ensure the name is unique
		if(!mixer.getElement(name)) {
			//At this point, success should be guaranteed
			const auto err = mixer.addElement(
				Zuazo::Utils::makeUnique<element_type>(mixer.getInstance(), name)
			);

			assert(err);
			result.setType(Controller::Result::Type::SUCCESS);
		}
	}

	return result;
}

template<typename T>
inline Controller::Result MixerNode<T>::rmElement(Zuazo::ZuazoBase& base, 
													Controller::TokenArray tokens )
{
	Controller::Result result;

	if(tokens.size() == 1) {
		assert(typeid(base) == typeid(Mixer));
		auto& mixer = static_cast<Mixer&>(base);
		const auto& name = tokens[0];

		//Check if exists
		const auto old = mixer.eraseElement(name);
		if(old) {
			result.setType(Controller::Result::Type::SUCCESS);
		}
	}

	return result;
}

}