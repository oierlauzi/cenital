#include "ControllerFunction.h"

#include "StringConversions.h"

#include <functional>

namespace Cenital {

inline bool parse(Controller::TokenArray tokens) {
	return tokens.empty();
}

template<typename First, typename... Args>
inline bool parse(Controller::TokenArray tokens, First& first, Args&... rest) {
	bool result = false;

	if(!tokens.empty()) {
		//Try to parse the rest
		result = parse(
			Controller::TokenArray(std::next(tokens.cbegin()), tokens.cend()),
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
inline bool parse(Controller::TokenArray tokens, std::tuple<Args...>& args) {
	return std::apply(
		[tokens] (auto&... args) -> bool {
			return parse(tokens, args...);
		},
		args
	);
}


template<typename T, typename R, typename... Args>
inline Controller::Result invokeSetter(R (*func)(T&, Args...), Zuazo::ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	//Try to parse the tokens
	std::tuple<Args...> args;
	if(parse(tokens, args)) {
		assert(typeid(base) == typeid(T));
		T& element = static_cast<T&>(base);

		//Elaborate the response
		std::apply(
			func,
			std::tuple_cat(std::forward_as_tuple(element), std::move(args))
		);
		result.setType(Controller::Result::Type::SUCCESS);
	}

	return result;
}

template<typename T, typename R, typename... Args>
inline Controller::Result invokeGetter(R (*func)(T&, Args...), Zuazo::ZuazoBase& base, Controller::TokenArray tokens) {
	Controller::Result result;

	//Try to parse the tokens
	std::tuple<Args...> args;
	if(parse(tokens, args)) {
		assert(typeid(base) == typeid(T));
		T& element = static_cast<T&>(base);

		//Elaborate the response
		const auto ret = std::apply(
			func,
			std::tuple_cat(std::forward_as_tuple(element), std::move(args))
		);
		result.setType(Controller::Result::Type::RESPONSE);
		result.setPayload({ std::string(toString(ret)) });
	}

	return result;
}

}