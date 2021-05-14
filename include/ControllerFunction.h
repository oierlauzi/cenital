#pragma once

#include "Controller.h"

#include <tuple>
#include <functional>

namespace Cenital {

bool parse(Controller::TokenArray tokens);

template<typename First, typename... Args>
bool parse(Controller::TokenArray tokens, First& first, Args&... rest);

template<typename... Args>
bool parse(Controller::TokenArray tokens, std::tuple<Args...>& args);

template<typename T, typename R, typename... Args>
Controller::Result invokeSetter(R (*func)(T&, Args...), Zuazo::ZuazoBase& base, Controller::TokenArray tokens);

template<typename T, typename R, typename... Args>
Controller::Result invokeGetter(R (*func)(T&, Args...), Zuazo::ZuazoBase& base, Controller::TokenArray tokens);

}

#include "ControllerFunction.inl"