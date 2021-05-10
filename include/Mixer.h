#pragma once

#include "Controller.h"

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

	std::vector<std::reference_wrapper<ZuazoBase>>			listElements();
	std::vector<std::reference_wrapper<ZuazoBase>>			listElements(std::type_index type);
	std::vector<std::reference_wrapper<const ZuazoBase>>	listElements() const;
	std::vector<std::reference_wrapper<const ZuazoBase>>	listElements(std::type_index type) const;

	static void 											registerCommands(Controller::Node& node);

};
	
}