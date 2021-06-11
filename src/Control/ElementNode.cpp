#include <Control/ElementNode.h>

#include <Control/Controller.h>
#include <Control/Message.h>

namespace Cenital::Control {

using namespace Zuazo;

ElementNode::ElementNode(FindCallback findCbk)
	: m_findCallback(std::move(findCbk))
{
}


void ElementNode::setFindCallback(FindCallback findCbk) {
	m_findCallback = std::move(findCbk);
}

const ElementNode::FindCallback& ElementNode::getFindCallback() const noexcept {
	return m_findCallback;
}


void ElementNode::operator()(	Controller& controller,
								ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) const
{
	const auto& find = getFindCallback();
	const auto& tokens = request.getPayload();
	if((tokens.size() > level) && find) {
		//Try to find the referred element
		const auto& elementName = tokens[level];
		auto* element = find(base, elementName);

		if(element) {
			//Determine the type of the element
			const auto& classIndex = controller.getClassIndex();
			const auto* classEntry = classIndex.find(typeid(*element));

			if(classEntry) {
				//Element is valid and path exists, continue
				Utils::invokeIf(classEntry->getConfigureCallback(), controller, *element, request, level + 1, response);
			}
		}
	}
}
	
}