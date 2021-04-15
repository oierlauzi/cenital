#pragma once

#include <zuazo/ZuazoBase.h>
#include <zuazo/Utils/Pimpl.h>

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

	Mixer&			operator=(const Mixer& other) = delete;
	Mixer&			operator=(Mixer&& other) noexcept;

	//TODO
};
	
}