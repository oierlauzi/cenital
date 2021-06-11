#pragma once

#include "Node.h"

#include <zuazo/ZuazoBase.h>

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <string_view>

namespace Cenital::Control {

class ClassIndex {
public:
	class Entry {
	public:
		using ConfigureCallback = Node::Callback;
		using ConstructCallback = std::function<std::unique_ptr<Zuazo::ZuazoBase>(	Zuazo::Instance&, 
																					Zuazo::Utils::BufferView<const std::string> )>;

		Entry(	std::string name, 
				ConfigureCallback configureCbk,
				ConstructCallback constructCbk,
				std::type_index baseClass );
		Entry(const Entry& other) = default;
		Entry(Entry&& other) = default;
		~Entry() = default;

		Entry&						operator=(const Entry& other) = default;
		Entry&						operator=(Entry&& other) = default;

		const std::string&			getName() const noexcept;
		const ConfigureCallback&	getConfigureCallback() const noexcept;
		const ConstructCallback&	getConstructCallback() const noexcept;
		std::type_index				getBaseClass() const noexcept;

	private:
		std::string 				m_name;
		ConfigureCallback			m_configureCallback;
		ConstructCallback			m_constructCallback;
		std::type_index				m_baseClass;
	
	};

	using ClassMap = std::unordered_map<std::type_index, Entry>;

	ClassIndex() = default;
	ClassIndex(std::initializer_list<ClassMap::value_type> ilist);
	ClassIndex(const ClassIndex& other) = default;
	ClassIndex(ClassIndex&& other) = default;
	~ClassIndex() = default;

	ClassIndex&		operator=(const ClassIndex& other) = default;
	ClassIndex&		operator=(ClassIndex&& other) = default;

	bool			registerClass(std::type_index type, Entry data);
	Entry*			find(std::type_index type);
	const Entry*	find(std::type_index type) const;
	Entry*			find(std::string_view name);
	const Entry*	find(std::string_view name) const;

private:
	ClassMap		m_classes;
};

}