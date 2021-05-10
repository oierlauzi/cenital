#include <TransitionBase.h>

namespace Cenital {

using namespace Zuazo;

TransitionBase::TransitionBase(	Instance& instance, 
								std::string name,
								Input& prevIn,
								Input& postIn,
								Layers layers,
								MoveCallback moveCbk,
								OpenCallback openCbk,
								AsyncOpenCallback asyncOpenCbk,
								CloseCallback closeCbk,
								AsyncCloseCallback asyncCloseCbk,
								UpdateCallback updateCbk,
								SizeCallback sizeCbk )
	: ZuazoBase(
		instance, 
		std::move(name), 
		{}, 
		std::move(moveCbk), 
		std::move(openCbk), 
		std::move(asyncOpenCbk),
		std::move(closeCbk), 
		std::move(asyncCloseCbk),
		std::move(updateCbk) )
	, ClipBase(
		std::chrono::seconds(1), 
		{}, 
		std::bind(&TransitionBase::update, this) ) //FIXME cref(*this) becomes a dangling reference when moved
	, m_prevIn(prevIn)
	, m_postIn(postIn)
	, m_layers(layers)
	, m_renderer(nullptr)
	, m_size()
	, m_sizeCallback(std::move(sizeCbk))
{
	//Register the pads
	registerPad(prevIn);
	registerPad(postIn);
}


TransitionBase::Input& TransitionBase::getPrevIn() noexcept {
	return m_prevIn;
}

const TransitionBase::Input& TransitionBase::getPrevIn() const noexcept {
	return m_prevIn;
}


TransitionBase::Input& TransitionBase::getPostIn() noexcept {
	return m_postIn;
}

const TransitionBase::Input& TransitionBase::getPostIn() const noexcept {
	return m_postIn;
}


TransitionBase::Layers TransitionBase::getLayers() const noexcept {
	return m_layers;
}



void TransitionBase::setSize(Math::Vec2f size) {
	if(m_size != size) {
		m_size = size;
		Utils::invokeIf(m_sizeCallback, *this, size);
	}
}

Math::Vec2f TransitionBase::getSize() const noexcept {
	return m_size;
}



void TransitionBase::setPrevIn(Input& in) noexcept {
	m_prevIn = in;
}

void TransitionBase::setPostIn(Input& in) noexcept {
	m_postIn = in;
}


void TransitionBase::setLayers(Layers layers) noexcept {
	m_layers = layers;
}



void TransitionBase::setSizeCallback(SizeCallback cbk) {
	m_sizeCallback = std::move(cbk);
}

const TransitionBase::SizeCallback& TransitionBase::getSizeCallback() const noexcept {
	return m_sizeCallback;
}

}