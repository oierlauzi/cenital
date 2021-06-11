#pragma once

#include <zuazo/ZuazoBase.h>

#include <functional>

namespace Cenital::Control {

class Controller;
class Message;

class Node {
public:
	using Callback = std::function<void(Controller&,
										Zuazo::ZuazoBase&, 
										const Message&,
										size_t,
										Message& )>;
	using PathMap = std::unordered_map<std::string, Callback>;

	Node() = default;
	Node(std::initializer_list<PathMap::value_type> ilist);
	Node(const Node& other) = default;
	Node(Node&& other) = default;
	~Node() = default;

	Node&				operator=(const Node& other) = default;
	Node&				operator=(Node&& other) = default;

	bool 				addPath(std::string token, Callback path);
	bool 				removePath(const std::string& token);
	Callback* 			getPath(const std::string& token);
	const Callback* 	getPath(const std::string& token) const;

	void 				operator()(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response ) const;

	void				help(	Controller& controller,
								Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response ) const;
	static void			name(	Controller& controller,
								Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response );
	static void			type(	Controller& controller,
								Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response );
	static void 		ping(	Controller& controller,
								Zuazo::ZuazoBase& base, 
								const Message& request,
								size_t level,
								Message& response );

private:
	PathMap				m_paths;

};

}