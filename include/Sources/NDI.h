#pragma once

#include <Control/Controller.h>

#include <zuazo/Sources/NDI.h>

namespace Cenital::Sources {

struct NDI :
	Zuazo::Sources::NDI
{
	NDI(Zuazo::Instance& instance, std::string name);

	static void						registerCommands(Control::Controller& controller);
};

}