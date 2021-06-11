#pragma once

#include "Node.h"
#include "ClassIndex.h"

#include <zuazo/ZuazoBase.h>

#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

namespace Cenital::Control {

class ViewBase;
class Message;

class Controller {
public:
	Controller(Zuazo::ZuazoBase& base);
	Controller(const Controller& other) = default;
	Controller(Controller&& other) = default;
	~Controller() = default;

	Controller&										operator=(const Controller& other) = default;
	Controller&										operator=(Controller&& other) = default;

	Node&											getRootNode() noexcept;
	const Node&										getRootNode() const noexcept;

	ClassIndex&										getClassIndex() noexcept;
	const ClassIndex&								getClassIndex() const noexcept;

	void											process(const Message& request,
															Message& response);

	void											addView(ViewBase& view);
	void											removeView(const ViewBase& view);
	
private:
	Node											m_root;
	ClassIndex										m_classIndex;
	std::vector<std::reference_wrapper<ViewBase>>	m_views;
	std::reference_wrapper<Zuazo::ZuazoBase>		m_baseObject;

	void											broadcast(const Message& msg);

};

}