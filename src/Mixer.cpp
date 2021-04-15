#include <Mixer.h>

namespace Cenital {

using namespace Zuazo;

/*
 * MixerImpl
 */

struct MixerImpl {

};



/*
 * Mixer
 */

Mixer::Mixer(	Zuazo::Instance& instance,
				std::string name )
	: Utils::Pimpl<MixerImpl>({})
	, ZuazoBase(
		instance,
		std::move(name)
	)
{
}

Mixer::Mixer(Mixer&& other) noexcept = default;

Mixer::~Mixer() = default;

Mixer& Mixer::operator=(Mixer&& other) noexcept = default;
	
}