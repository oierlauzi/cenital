#pragma once

#include <zuazo/Video.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/ClipBase.h>
#include <zuazo/RendererBase.h>
#include <zuazo/Signal/Input.h>
#include <zuazo/Utils/BufferView.h>

#include <utility>
#include <functional>

namespace Cenital {

class TransitionBase 
	: public Zuazo::ZuazoBase
	, public Zuazo::ClipBase
{
public:
	using Input = Zuazo::Signal::PadProxy<Zuazo::Signal::Input<Zuazo::Video>>;
	using Layers = Zuazo::Utils::BufferView<const Zuazo::RendererBase::LayerRef>;
	using SizeCallback = std::function<void(TransitionBase&, Zuazo::Math::Vec2f)>;

	TransitionBase(	Zuazo::Instance& instance, 
					std::string name,
					Input& prevIn,
					Input& postIn,
					Layers layers,
					MoveCallback moveCbk = {},
					OpenCallback openCbk = {},
					AsyncOpenCallback asyncOpenCbk = {},
					CloseCallback closeCbk = {},
					AsyncCloseCallback asyncCloseCbk = {},
					UpdateCallback updateCbk = {},
					SizeCallback sizeCbk = {} );
	TransitionBase(const TransitionBase& other) = default;
	TransitionBase(TransitionBase&& other) = default;
	~TransitionBase() = default;

	TransitionBase&					operator=(const TransitionBase& other) = default;
	TransitionBase&					operator=(TransitionBase&& other) = default;

	using ClipBase::setDuration;
	using ClipBase::setTimeStep;

	Input&							getPrevIn() noexcept;
	const Input&					getPrevIn() const noexcept;
	Input&							getPostIn() noexcept;
	const Input&					getPostIn() const noexcept;

	Layers							getLayers() const noexcept;

	void							setSize(Zuazo::Math::Vec2f size);
	Zuazo::Math::Vec2f				getSize() const noexcept;

protected:
	void							setPrevIn(Input& in) noexcept;
	void							setPostIn(Input& in) noexcept;

	void							setLayers(Layers layers) noexcept;

	void							setSizeCallback(SizeCallback cbk);
	const SizeCallback&				getSizeCallback() const noexcept;

private:
	std::reference_wrapper<Input>	m_prevIn;
	std::reference_wrapper<Input>	m_postIn;
	Layers							m_layers;
	const Zuazo::RendererBase*		m_renderer;
	Zuazo::Math::Vec2f				m_size;

	SizeCallback					m_sizeCallback;

};

}