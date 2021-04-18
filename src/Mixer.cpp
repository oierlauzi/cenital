#include <Mixer.h>

#include <unordered_map>
#include <cassert>

namespace Cenital {

using namespace Zuazo;

/*
 * MixerImpl
 */

struct MixerImpl {
	using ElementMap = std::unordered_map<std::string_view, std::unique_ptr<ZuazoBase>>;

	ElementMap						elements;

	MixerImpl() = default;
	~MixerImpl() = default;

	void open() {
		std::for_each(
			elements.begin(), elements.end(),
			[] (ElementMap::value_type& element) {
				assert(element.second);
				element.second->open();
			}
		);
	}

	void asyncOpen(std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());

		std::for_each(
			elements.begin(), elements.end(),
			[&lock] (ElementMap::value_type& element) {
				assert(element.second);
				element.second->asyncOpen(lock);
			}
		);

		assert(lock.owns_lock());
	}

	void close() {
		std::for_each(
			elements.begin(), elements.end(),
			[] (ElementMap::value_type& element) {
				assert(element.second);
				element.second->close();
			}
		);
	}

	void asyncClose(std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());

		std::for_each(
			elements.begin(), elements.end(),
			[&lock] (ElementMap::value_type& element) {
				assert(element.second);
				element.second->asyncClose(lock);
			}
		);

		assert(lock.owns_lock());
	}

	bool addElement(Mixer& mixer, std::unique_ptr<ZuazoBase> element) {
		bool result;

		//Only add if non null
		if(element) {
			//Insert the new element if possible
			ElementMap::iterator ite;
			std::tie(ite, result) = elements.emplace(
				element->getName(),
				std::move(element)
			);

			//Set a matching state
			assert(ite->second);
			if(mixer.isOpen() && !ite->second->isOpen()) {
				ite->second->open();
			} else if(!mixer.isOpen() && ite->second->isOpen()) {
				ite->second->close();
			}
		} else {
			result = false;
		}

		return result;
	}

	std::unique_ptr<ZuazoBase> eraseElement(std::string_view name) {
		std::unique_ptr<ZuazoBase> result;

		const auto ite = elements.find(name);
		if(ite != elements.cend()) {
			//Element exists. Erase it
			result = std::move(ite->second);
			assert(result);
			elements.erase(ite);
		}

		return result;
	}


	ZuazoBase* getElement(std::string_view name) const noexcept {
		ZuazoBase* result;

		const auto ite = elements.find(name);
		if(ite != elements.cend()) {
			result = ite->second.get();
			assert(result); //If found, it should be non-null
		} else {
			result = nullptr;
		}

		return result;
	}

	std::vector<std::reference_wrapper<ZuazoBase>> listElements() const {
		std::vector<std::reference_wrapper<ZuazoBase>> result;

		for(const auto& element : elements) {
			assert(element.second);
			result.emplace_back(*element.second);
		}

		return result;
	}

	std::vector<std::reference_wrapper<ZuazoBase>> listElements(std::type_index type) const {
		std::vector<std::reference_wrapper<ZuazoBase>> result;

		for(const auto& element : elements) {
			assert(element.second);
			if(std::type_index(typeid(*element.second)) == type) {
				result.emplace_back(*element.second);
			}
		}

		return result;
	}

};



/*
 * Mixer
 */

Mixer::Mixer(	Zuazo::Instance& instance,
				std::string name )
	: Utils::Pimpl<MixerImpl>({})
	, ZuazoBase(
		instance,
		std::move(name),
		{},
		{},
		std::bind(&MixerImpl::open, std::ref(**this)),
		std::bind(&MixerImpl::asyncOpen, std::ref(**this), std::placeholders::_2),
		std::bind(&MixerImpl::close, std::ref(**this)),
		std::bind(&MixerImpl::asyncClose, std::ref(**this), std::placeholders::_2)
	)
{
}

Mixer::Mixer(Mixer&& other) noexcept = default;

Mixer::~Mixer() = default;

Mixer& Mixer::operator=(Mixer&& other) noexcept = default;
	


bool Mixer::addElement(std::unique_ptr<ZuazoBase> element) {
	return (*this)->addElement(*this, std::move(element));
}

std::unique_ptr<ZuazoBase> Mixer::eraseElement(std::string_view name) {
	return (*this)->eraseElement(name);
}


ZuazoBase* Mixer::getElement(std::string_view name) noexcept {
	return (*this)->getElement(name);
}

const ZuazoBase* Mixer::getElement(std::string_view name) const noexcept {
	return (*this)->getElement(name);
}


std::vector<std::reference_wrapper<ZuazoBase>> Mixer::listElements() {
	return (*this)->listElements();
}

std::vector<std::reference_wrapper<ZuazoBase>> Mixer::listElements(std::type_index type) {
	return (*this)->listElements(type); 
}

std::vector<std::reference_wrapper<const ZuazoBase>> Mixer::listElements() const {
	return reinterpret_cast<std::vector<std::reference_wrapper<const ZuazoBase>>&&>((*this)->listElements());
}

std::vector<std::reference_wrapper<const ZuazoBase>> Mixer::listElements(std::type_index type) const {
	return reinterpret_cast<std::vector<std::reference_wrapper<const ZuazoBase>>&&>((*this)->listElements(type));
}


}