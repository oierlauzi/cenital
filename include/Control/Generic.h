#pragma once

#include "Message.h"
#include "Node.h"

#include <tuple>

namespace Cenital::Control {

template<typename R, typename T, typename... Args>
using MemFnPtr = R(T::*)(Args...);

template<typename R, typename T, typename... Args>
using ConstMemFnPtr = R(T::*)(Args...) const;

struct DefaultValidator {
	template<typename... Args>
	constexpr bool operator()(Args&&...) const noexcept{
		return true;
	}
};



bool parse(Zuazo::Utils::BufferView<const std::string> tokens);

template<typename First, typename... Args>
bool parse(	Zuazo::Utils::BufferView<const std::string> tokens,
			First& first, Args&... rest );

template<typename... Args>
bool parse(	Zuazo::Utils::BufferView<const std::string> tokens,
			std::tuple<Args...>& args);


template<typename T, typename... Args, typename F, typename V>
void invokeSetter(	F&& func,
					V&& validate,
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );

template<typename T, typename... Args, typename F>
void invokeSetter(	F&& func,
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );

template<typename T, typename... Args, typename V>
void invokeSetter(	MemFnPtr<void, T, Args...> func, 
					V&& validate,
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );

template<typename T, typename... Args>
void invokeSetter(	MemFnPtr<void, T, Args...> func,
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );


template<typename R, typename T, typename... Args, typename F, typename V>
void invokeGetter(	F&& func, 
					V&& validate,
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );

template<typename R, typename T, typename... Args, typename F>
void invokeGetter(	F&& func, 
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );

template<typename R, typename T, typename... Args, typename V>
void invokeGetter(	ConstMemFnPtr<R, T, Args...> func, 
					V&& validate,
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );

template<typename R, typename T, typename... Args>
void invokeGetter(	ConstMemFnPtr<R, T, Args...> func, 
					Controller& controller,
					Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response );



template<typename T, typename... Args>
std::unique_ptr<Zuazo::ZuazoBase> invokeBaseConstructor(Zuazo::Instance& instance, 
														Zuazo::Utils::BufferView<const std::string> args );


template<typename T>
void enumerate(	Controller& controller,
				Zuazo::ZuazoBase& base, 
				const Message& request,
				size_t level,
				Message& response );


Node makeAttributeNode(	Node::Callback setter,
						Node::Callback getter,
						Node::Callback lister = {},
						Node::Callback unsetter = {} );

}

#include "Generic.inl"