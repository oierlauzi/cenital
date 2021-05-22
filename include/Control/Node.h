#pragma once

#include <zuazo/ZuazoBase.h>

#include <functional>

namespace Cenital::Control {

class Message;

class Node {
public:
	using Callback = std::function<void(Zuazo::ZuazoBase&, 
										const Message&,
										size_t,
										Message& )>;

	Node() = default;
	Node(std::initializer_list<std::pair<const std::string, Callback>> ilist);
	Node(const Node& other) = default;
	Node(Node&& other) = default;
	~Node() = default;

	Node&										operator=(const Node& other) = default;
	Node&										operator=(Node&& other) = default;

	void 										addPath(std::string token, Callback path);
	void 										removePath(const std::string& token);
	const Callback* 							getPath(const std::string& token) const;

	void 										operator()(	Zuazo::ZuazoBase& base, 
															const Message& request,
															size_t level,
															Message& response ) const;

	void 										help(	Zuazo::ZuazoBase& base, 
														const Message& request,
														size_t level,
														Message& response ) const;
	static void 								ping(	Zuazo::ZuazoBase& base, 
														const Message& request,
														size_t level,
														Message& response );

private:
	std::unordered_map<std::string, Callback>	m_paths;

};

}