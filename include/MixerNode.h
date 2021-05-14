#pragma once

#include "Controller.h"

namespace Cenital {

template<typename T>
class MixerNode {
public:
	static_assert(std::is_convertible<T*, Zuazo::ZuazoBase*>::value, "T must publicly inherit from ZuazoBase");
	using element_type = T;

	explicit MixerNode(Controller::Node::Callback targetCbk = {});
	MixerNode(const MixerNode& other) = default;
	MixerNode(MixerNode&& other) = default;
	~MixerNode() = default;

	MixerNode&							operator=(const MixerNode& other) = default;
	MixerNode&							operator=(MixerNode&& other) = default;

	void								setTargetCallback(Controller::Node::Callback cbk);
	const Controller::Node::Callback&	getTargetCallback() const noexcept;

	Controller::Result 					operator()(	Zuazo::ZuazoBase& base, 
													Controller::TokenArray tokens ) const;
private:
	Controller::Node::Callback			m_targetCallback;

	Controller::Result					targetElement(	Zuazo::ZuazoBase& base, 
														Controller::TokenArray tokens ) const;
	static Controller::Result			listElement(Zuazo::ZuazoBase& base, 
													Controller::TokenArray tokens );
	static Controller::Result			addElement(	Zuazo::ZuazoBase& base, 
													Controller::TokenArray tokens );
	static Controller::Result			rmElement(	Zuazo::ZuazoBase& base, 
													Controller::TokenArray tokens );



};

}

#include "MixerNode.inl"