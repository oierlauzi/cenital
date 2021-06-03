#pragma once

#include "Control/Node.h"

#include <zuazo/ZuazoBase.h>
#include <zuazo/Utils/Pimpl.h>

#include <memory>
#include <typeindex>

namespace Cenital {

struct MixerImpl;
class Mixer 
	: public Zuazo::Utils::Pimpl<MixerImpl>
	, public Zuazo::ZuazoBase
{
public:
	using ConstructCallback = std::function<std::unique_ptr<Zuazo::ZuazoBase>(	Zuazo::Instance&, 
																				Zuazo::Utils::BufferView<const std::string> )>;

	Mixer(	Zuazo::Instance& instance,
			std::string name );
	Mixer(const Mixer& other) = delete;
	Mixer(Mixer&& other) noexcept;
	virtual ~Mixer();

	Mixer&													operator=(const Mixer& other) = delete;
	Mixer&													operator=(Mixer&& other) noexcept;

	bool													addElement(std::unique_ptr<ZuazoBase> element);
	std::unique_ptr<ZuazoBase>								eraseElement(std::string_view name);

	ZuazoBase*												getElement(std::string_view name) noexcept;
	const ZuazoBase*										getElement(std::string_view name) const noexcept;

	bool													connect(std::string_view dstName,
																	std::string_view dstPort,
																	std::string_view srcName,
																	std::string_view srcPort );
	bool													disconnect(	std::string_view dstName,
																		std::string_view dstPort );

	std::vector<std::reference_wrapper<ZuazoBase>>			listElements();
	std::vector<std::reference_wrapper<ZuazoBase>>			listElements(std::type_index type);
	std::vector<std::reference_wrapper<const ZuazoBase>>	listElements() const;
	std::vector<std::reference_wrapper<const ZuazoBase>>	listElements(std::type_index type) const;

	static void 											registerCommands(Control::Node& node);
	static void 											registerClass(	Control::Node& node,
																			std::type_index type,
																			std::string name,
																			ConstructCallback construct,
																			Control::Node::Callback configure );

};
	
}