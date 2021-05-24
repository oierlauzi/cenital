#include "Generic.h"

#include "../StringConversions.h"

#include <functional>

namespace Cenital::Control {

inline bool parse(Zuazo::Utils::BufferView<const std::string> tokens) {
	return tokens.empty();
}

template<typename First, typename... Args>
inline bool parse(	Zuazo::Utils::BufferView<const std::string> tokens,
					First& first, Args&... rest) 
{
	bool result = false;

	if(!tokens.empty()) {
		//Try to parse the rest
		result = parse(
			Zuazo::Utils::BufferView<const std::string>(
				std::next(tokens.cbegin()),
				tokens.cend()
			),
			rest...
		);

		//Try to parse the first argument if the
		//previous step succeeded
		if(result) {
			result = fromString(tokens[0], first);
		}
	}

	return result;
}

template<typename... Args>
inline bool parse(	Zuazo::Utils::BufferView<const std::string> tokens,
					std::tuple<Args...>& args ) 
{
	return std::apply(
		[tokens] (auto&... args) -> bool {
			return parse(tokens, args...);
		},
		args
	);
}



template<typename T, typename... Args, typename F>
inline void invokeSetter(	F&& func, 
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	//Get the tokens from the request
	const auto& payload = request.getPayload();
	const Zuazo::Utils::BufferView<const std::string> tokens(
		std::next(payload.data(), level),
		payload.data() + payload.size()
	);

	//Try to parse the tokens
	std::tuple<Args...> args;
	if(parse(tokens, args)) {
		assert(typeid(base) == typeid(T));
		T& element = static_cast<T&>(base);

		//Elaborate the response
		std::apply(
			std::forward<F>(func),
			std::tuple_cat(std::forward_as_tuple(element), std::move(args))
		);
		response.setType(Message::Type::BROADCAST);
		response.getPayload() = request.getPayload();
	}
}

template<typename T, typename... Args>
inline void invokeSetter(	MemFnPtr<void, T, Args...> func, 
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<T, Args...>(std::mem_fn(func), base, request, level, response);
}

template<typename T, typename... Args>
inline void invokeSetter(	FnPtr<void, T&, Args...> func, 
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	//Ref is used to force the generic expresion
	invokeSetter<T, Args...>(std::cref(func), base, request, level, response);
}




template<typename R, typename T, typename... Args, typename F>
inline void invokeGetter(	F&& func, 
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	//Get the tokens from the request
	const auto& payload = request.getPayload();
	const Zuazo::Utils::BufferView<const std::string> tokens(
		std::next(payload.data(), level),
		payload.data() + payload.size()
	);

	//Try to parse the tokens
	std::tuple<Args...> args;
	if(parse(tokens, args)) {
		assert(typeid(base) == typeid(T));
		T& element = static_cast<T&>(base);

		//Elaborate the response
		const auto ret = std::apply(
			std::forward<F>(func),
			std::tuple_cat(std::forward_as_tuple(element), std::move(args))
		);
		response.setType(Message::Type::RESPONSE);
		response.getPayload() = { std::string(toString(ret)) };
	}
}

template<typename R, typename T, typename... Args>
inline void invokeGetter(	ConstMemFnPtr<R, T, Args...> func, 
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeGetter<R, T, Args...>(std::mem_fn(func), base, request, level, response);
}

template<typename R, typename T, typename... Args>
inline void invokeGetter(	FnPtr<void, T&, Args...> func, 
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	//Ref is used to force the generic expresion
	invokeGetter<R, T, Args...>(std::cref(func), base, request, level, response);
}



template<typename T, typename... Args>
inline std::unique_ptr<Zuazo::ZuazoBase> 
invokeBaseConstructor(	Zuazo::Instance& instance, 
						Zuazo::Utils::BufferView<const std::string> tokens )
{
	std::unique_ptr<Zuazo::ZuazoBase> result;

	//Try to parse the tokens
	std::tuple<std::string, Args...> args;
	if(parse(tokens, args)) {
		//Invoke the constructor
		result = std::apply(
			Zuazo::Utils::makeUnique<T, Zuazo::Instance&, std::string, Args...>,
			std::tuple_cat(std::forward_as_tuple(instance), std::move(args))
		);
	}


	return result;
}


inline Node makeAttributeNode(	Node::Callback setter,
								Node::Callback getter,
								Node::Callback lister,
								Node::Callback unsetter )
{
	Node result;

	if(setter) {
		result.addPath("set", std::move(setter));
	}

	if(getter) {
		result.addPath("get", std::move(getter));
	}

	if(lister) {
		result.addPath("enum", std::move(lister));
	}

	if(unsetter) {
		result.addPath("unset", std::move(unsetter));
	}

	return result;
}

inline Node makeCollectionNode(	Node::Callback adder,
								Node::Callback remover,
								Node::Callback lister )
{
	Node result;

	if(adder) {
		result.addPath("add", std::move(adder));
	}

	if(remover) {
		result.addPath("rm", std::move(remover));
	}

	if(lister) {
		result.addPath("ls", std::move(lister));
	}

	return result;	
}

}