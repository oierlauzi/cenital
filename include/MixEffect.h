#pragma once

#include "TransitionBase.h"

#include <zuazo/ZuazoBase.h>
#include <zuazo/RendererBase.h>
#include <zuazo/Signal/Input.h>
#include <zuazo/Signal/Output.h>

#include <memory>

namespace Cenital {

struct MixEffectImpl;
class MixEffect 
	: private Zuazo::Utils::Pimpl<MixEffectImpl>
	, public Zuazo::ZuazoBase
	, public Zuazo::VideoBase
{
	friend MixEffectImpl;
public:
	using Input = Zuazo::Signal::Layout::PadProxy<Zuazo::Signal::Input<Zuazo::Video>>;
	using Output = Zuazo::Signal::Layout::PadProxy<Zuazo::Signal::Output<Zuazo::Video>>;

	static constexpr auto NO_SIGNAL = ~size_t(0);

	MixEffect(	Zuazo::Instance& instance,
				std::string name );

	MixEffect(const MixEffect& other) = delete;
	MixEffect(MixEffect&& other);
	virtual ~MixEffect();

	MixEffect&								operator=(const MixEffect& other) = delete;
	MixEffect&								operator=(MixEffect&& other);

	void									setInputCount(size_t count);
	size_t									getInputCount() const noexcept;
	Input&									getInput(size_t idx);
	const Input&							getInput(size_t idx) const;

	Output&									getProgramOutput() noexcept;
	const Output&							getProgramOutput() const noexcept;
	Output&									getPreviewOutput() noexcept;
	const Output&							getPreviewOutput() const noexcept;

	std::unique_ptr<TransitionBase>			setTransition(std::unique_ptr<TransitionBase> transition);
	const TransitionBase*					getTransition() const noexcept;

	void									setUpstreamOverlayCount(size_t count);
	size_t									getUpstreamOverlayCount() const noexcept;

	void									setDownstreamOverlayCount(size_t count);
	size_t									getDownstreamOverlayCount() const noexcept;

	void									setProgram(size_t idx);
	size_t									getProgram() const noexcept;

	void									setPreview(size_t idx);
	size_t									getPreview() const noexcept;

	void									cut();

};
	
}