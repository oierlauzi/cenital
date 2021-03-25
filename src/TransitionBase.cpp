#include <TransitionBase.h>

namespace Cenital {

TransitionBase::TransitionBase(	Input& prevIn,
								Input& postIn,
								Layers layers,
								RefreshCallback refreshCbk,
								RendererCallback rendererCbk,
								SizeCallback sizeCbk )
	: ClipBase(std::chrono::seconds(1), {}, std::move(refreshCbk))
	, m_prevIn(prevIn)
	, m_postIn(postIn)
	, m_layers(layers)
	, m_renderer(nullptr)
	, m_size()
	, m_rendererCallback(std::move(rendererCbk))
	, m_sizeCallback(std::move(sizeCbk))

{
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



void TransitionBase::setRenderer(const Zuazo::RendererBase* renderer) {
	if(m_renderer != renderer) {
		m_renderer = renderer;
		Zuazo::Utils::invokeIf(m_rendererCallback, *this, m_renderer);
	}
}

const Zuazo::RendererBase* TransitionBase::getRenderer() const noexcept {
	return m_renderer;
}


void TransitionBase::setSize(Zuazo::Math::Vec2f size) {
	if(m_size != size) {
		m_size = size;
		Zuazo::Utils::invokeIf(m_sizeCallback, *this, size);
	}
}

Zuazo::Math::Vec2f TransitionBase::getSize() const noexcept {
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



void TransitionBase::setRendererCallback(RendererCallback cbk) {
	m_rendererCallback = std::move(cbk);
}

const TransitionBase::RendererCallback& TransitionBase::getRendererCallback() const noexcept {
	return m_rendererCallback;
}


void TransitionBase::setSizeCallback(SizeCallback cbk) {
	m_sizeCallback = std::move(cbk);
}

const TransitionBase::SizeCallback& TransitionBase::getSizeCallback() const noexcept {
	return m_sizeCallback;
}

}