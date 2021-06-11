#include <Control/ClassIndex.h>

#include <algorithm>

namespace Cenital::Control {

using namespace Zuazo;

ClassIndex::Entry::Entry(	std::string name, 
							ConfigureCallback configureCbk,
							ConstructCallback constructCbk,
							std::type_index baseClass )
	: m_name(std::move(name))
	, m_configureCallback(std::move(configureCbk))
	, m_constructCallback(std::move(constructCbk))
	, m_baseClass(baseClass)
{
}


const std::string& ClassIndex::Entry::getName() const noexcept {
	return m_name;
}

const ClassIndex::Entry::ConfigureCallback&	ClassIndex::Entry::getConfigureCallback() const noexcept {
	return m_configureCallback;
}

const ClassIndex::Entry::ConstructCallback&	ClassIndex::Entry::getConstructCallback() const noexcept {
	return m_constructCallback;
}

std::type_index ClassIndex::Entry::getBaseClass() const noexcept {
	return m_baseClass;
}

	



ClassIndex::ClassIndex(std::initializer_list<ClassMap::value_type> ilist)
	: m_classes(ilist)
{
}


bool ClassIndex::registerClass(std::type_index type, Entry data) {
	bool result;
	std::tie(std::ignore, result) = m_classes.emplace(type, std::move(data));
	return result;
}

ClassIndex::Entry* ClassIndex::find(std::type_index type) {
	const auto ite = m_classes.find(type);
	return (ite != m_classes.end()) ? &ite->second : nullptr;
}

const ClassIndex::Entry* ClassIndex::find(std::type_index type) const {
	const auto ite = m_classes.find(type);
	return (ite != m_classes.end()) ? &ite->second : nullptr;
}

ClassIndex::Entry* ClassIndex::find(std::string_view name) {
	const auto ite = std::find_if(
		m_classes.begin(), m_classes.end(),
		[name] (const ClassMap::value_type& value) -> bool {
			return value.second.getName() == name;
		}
	);
	return (ite != m_classes.end()) ? &ite->second : nullptr;
}

const ClassIndex::Entry* ClassIndex::find(std::string_view name) const {
	const auto ite = std::find_if(
		m_classes.begin(), m_classes.end(),
		[name] (const ClassMap::value_type& value) -> bool {
			return value.second.getName() == name;
		}
	);
	return (ite != m_classes.end()) ? &ite->second : nullptr;
}

}