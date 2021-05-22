#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include "ViewBase.h"
#include "Message.h"

namespace Cenital::Control {

class CLIView 
	: public ViewBase
{
public:
	using Listener = std::function<void(const std::string&)>;

	explicit CLIView(Controller& controller);
	CLIView(const CLIView& other) = default;
	CLIView(CLIView&& other) = default;
	virtual ~CLIView() = default;

	CLIView&							operator=(const CLIView& other) = default;
	CLIView&							operator=(CLIView&& other) = default;

	void								addListener(Listener ls);

	void								parse(std::string_view msg, std::string& ret);
	virtual void						update(const Message& msg) final;

private:
	std::vector<Listener>				m_listeners;

	mutable Message						m_request;
	mutable Message						m_response;
	mutable std::string					m_update;

};

}