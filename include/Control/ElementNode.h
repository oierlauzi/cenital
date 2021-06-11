#pragma once

#include "Node.h"

#include <unordered_map>

namespace Cenital::Control {

class ElementNode {
public:
	using FindCallback = std::function<Zuazo::ZuazoBase*(Zuazo::ZuazoBase&, const std::string)>;

	ElementNode(FindCallback findCbk);
	ElementNode(const ElementNode& other) = default;
	ElementNode(ElementNode&& other) = default;
	~ElementNode() = default;

	ElementNode&			operator=(const ElementNode& other) = default;
	ElementNode&			operator=(ElementNode&& other) = default;

	void 					setFindCallback(FindCallback findCbk);
	const FindCallback& 	getFindCallback() const noexcept;

	void 					operator()(	Controller& controller,
										Zuazo::ZuazoBase& base, 
										const Message& request,
										size_t level,
										Message& response ) const;

private:
	FindCallback			m_findCallback;

};

}