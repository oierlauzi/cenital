#pragma once

#include "Node.h"

#include <zuazo/ZuazoBase.h>

#include <string>
#include <functional>
#include <typeindex>

namespace Cenital::Control {

class Message;

class MixerNode {
public:
	using ConstructCallback = std::function<std::unique_ptr<Zuazo::ZuazoBase>(	Zuazo::Instance&, 
																				Zuazo::Utils::BufferView<const std::string> )>;
	using AimCallback = Node::Callback;

	MixerNode(	std::type_index elementType,
				ConstructCallback constructCallback,
				AimCallback aimCbk );
	MixerNode(const MixerNode& other) = default;
	MixerNode(MixerNode&& other) = default;
	~MixerNode() = default;

	MixerNode&							operator=(const MixerNode& other) = default;
	MixerNode&							operator=(MixerNode&& other) = default;

	void								setElementType(std::type_index type);
	std::type_index						getElementType() const noexcept;

	void								setConstructCallback(ConstructCallback cbk);
	const ConstructCallback&			getConstructCallback() const noexcept;

	void								setAimCallback(AimCallback cbk);
	const AimCallback&					getAimCallback() const noexcept;

	void								operator()(	Zuazo::ZuazoBase& base, 
													const Message& request,
													size_t level,
													Message& response ) const;
private:
	ConstructCallback					m_constructCallback;
	AimCallback							m_aimCallback;
	std::type_index						m_type;


	void								help(	Zuazo::ZuazoBase& base, 
												const Message& request,
												size_t level,
												Message& response  ) const;
	void								add(Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response ) const;
	void								rm(	Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response  ) const;
	void								aim(Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response  ) const;
	void								ls(	Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response  ) const;

};

}