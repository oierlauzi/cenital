#include <Transitions/Base.h>

namespace Cenital::Transitions {

using namespace Zuazo;

Base::Base(	Instance& instance, 
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
		std::bind(&Base::update, this) ) //FIXME cref(*this) becomes a dangling reference when moved
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


Base::Input& Base::getPrevIn() noexcept {
	return m_prevIn;
}

const Base::Input& Base::getPrevIn() const noexcept {
	return m_prevIn;
}


Base::Input& Base::getPostIn() noexcept {
	return m_postIn;
}

const Base::Input& Base::getPostIn() const noexcept {
	return m_postIn;
}


Base::Layers Base::getLayers() const noexcept {
	return m_layers;
}



void Base::setSize(Math::Vec2f size) {
	if(m_size != size) {
		m_size = size;
		Utils::invokeIf(m_sizeCallback, *this, size);
	}
}

Math::Vec2f Base::getSize() const noexcept {
	return m_size;
}



void Base::setPrevIn(Input& in) noexcept {
	m_prevIn = in;
}

void Base::setPostIn(Input& in) noexcept {
	m_postIn = in;
}


void Base::setLayers(Layers layers) noexcept {
	m_layers = layers;
}



void Base::setSizeCallback(SizeCallback cbk) {
	m_sizeCallback = std::move(cbk);
}

const Base::SizeCallback& Base::getSizeCallback() const noexcept {
	return m_sizeCallback;
}

}