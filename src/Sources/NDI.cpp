#include <Sources/NDI.h>

namespace Cenital::Sources {

using namespace Zuazo;

NDI::NDI(Zuazo::Instance& instance, std::string name)
	: Zuazo::Sources::NDI(instance, std::move(name), Source())
{
}
	
}