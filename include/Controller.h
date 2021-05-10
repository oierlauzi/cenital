#pragma once

#include <zuazo/Utils/BufferView.h>
#include <zuazo/ZuazoBase.h>

#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

namespace Cenital {

class ViewBase;

class Controller {
public:
	using TokenArray = Zuazo::Utils::BufferView<const std::string>;

	class Result {
	public:
		enum class Type {
			ERROR,
			SUCCESS,
			RESPONSE
		};

		Result(	Type type = Type::ERROR,
				std::vector<std::string> payload = {} );
		Result(const Result& other) = default;
		Result(Result&& other) = default;
		~Result() = default;

		Result&											operator=(const Result& other) = default;
		Result&											operator=(Result&& other) = default;

		void											setType(Type type) noexcept;
		Type											getType() const noexcept;

		void											setPayload(std::vector<std::string> payload);
		const std::vector<std::string>&					getPayload() const noexcept;

	private:
		Type											m_type;
		std::vector<std::string>						m_payload;

	};

	class Node {
	public:
		using Callback = std::function<Result(Zuazo::ZuazoBase&, TokenArray)>;

		Node() = default;
		Node(std::initializer_list<std::pair<const std::string, Callback>> ilist);
		Node(const Node& other) = default;
		Node(Node&& other) = default;
		~Node() = default;

		Node&											operator=(const Node& other) = default;
		Node&											operator=(Node&& other) = default;

		void 											addPath(std::string token, Callback path);
		void 											removePath(const std::string& token);
		const Callback* 								getPath(const std::string& token) const;

		void											setFallback(Callback cbk);
		const Callback&									getFallback() const noexcept;

		Result 											operator()(Zuazo::ZuazoBase& base, TokenArray tokens) const;

		static Result 									ping(Zuazo::ZuazoBase& base, TokenArray tokens);

		static const std::string PING_PATH;

	private:
		std::unordered_map<std::string, Callback>		m_paths;
		Callback										m_fallback;

	};


	Controller(Zuazo::ZuazoBase& base);
	Controller(const Controller& other) = default;
	Controller(Controller&& other) = default;
	~Controller() = default;

	Controller&										operator=(const Controller& other) = default;
	Controller&										operator=(Controller&& other) = default;

	Node&											getRootNode() noexcept;
	const Node&										getRootNode() const noexcept;

	Result											process(TokenArray tokens);

	void											addView(ViewBase& view);
	void											removeView(const ViewBase& view);
	
private:
	Node											m_root;
	std::vector<std::reference_wrapper<ViewBase>>	m_views;
	std::reference_wrapper<Zuazo::ZuazoBase>		m_baseObject;

};

}