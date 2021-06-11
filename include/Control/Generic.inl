#include "Generic.h"

#include <zuazo/StringConversions.h>

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
			result = Zuazo::fromString(tokens[0], first);
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
							Controller&,
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
	std::tuple<typename std::decay<Args>::type...> args;
	if(parse(tokens, args)) {
		assert(typeid(base) == typeid(T));
		T& element = static_cast<T&>(base);

		//Elaborate the response
		std::apply(
			std::forward<F>(func),
			std::tuple_cat(std::forward_as_tuple(element), std::move(args))
		);
		response.setType(Message::Type::broadcast);
		response.getPayload() = request.getPayload();
	}
}

template<typename T, typename... Args>
inline void invokeSetter(	MemFnPtr<void, T, Args...> func, 
							Controller& controller,
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<T, Args...>(std::mem_fn(func), controller, base, request, level, response);
}

template<typename T, typename... Args>
inline void invokeSetter(	FnPtr<void, T&, Args...> func, 
							Controller& controller,
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	//Ref is used to force the generic expresion
	invokeSetter<T, Args...>(std::cref(func), controller, base, request, level, response);
}




template<typename R, typename T, typename... Args, typename F>
inline void invokeGetter(	F&& func, 
							Controller&,
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
	std::tuple<typename std::decay<Args>::type...> args;
	if(parse(tokens, args)) {
		assert(typeid(base) == typeid(T));
		T& element = static_cast<T&>(base);

		//Elaborate the response
		const auto ret = std::apply(
			std::forward<F>(func),
			std::tuple_cat(std::forward_as_tuple(element), std::move(args))
		);
		response.setType(Message::Type::response);
		response.getPayload() = { std::string(Zuazo::toString(ret)) };
	}
}

template<typename R, typename T, typename... Args>
inline void invokeGetter(	ConstMemFnPtr<R, T, Args...> func, 
							Controller& controller,
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeGetter<R, T, Args...>(std::mem_fn(func), controller, base, request, level, response);
}

template<typename R, typename T, typename... Args>
inline void invokeGetter(	FnPtr<void, T&, Args...> func, 
							Controller& controller,
							Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	//Ref is used to force the generic expresion
	invokeGetter<R, T, Args...>(std::cref(func), controller, base, request, level, response);
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


template<typename T>
inline void enumerate(	Controller&,
						Zuazo::ZuazoBase&, 
						const Message& request,
						size_t level,
						Message& response )
{
	using EnumTraits = Zuazo::Utils::EnumTraits<T>;
	const auto& tokens = request.getPayload();

	if(tokens.size() == level) {
		response.setType(Message::Type::response);
		
		auto& payload = response.getPayload();
		payload.clear();

		const auto first = EnumTraits::first();
		const auto last = EnumTraits::last();
		payload.reserve(static_cast<size_t>(last) - static_cast<size_t>(first) + 1);
		for(auto i = first; i <= last; ++i) {
			payload.emplace_back(Zuazo::toString(i));
		}
	}
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

}