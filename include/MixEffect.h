#pragma once

#include "Controller.h"
#include "TransitionBase.h"
#include "Keyer.h"

#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>
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

	enum class OutputBus {
		NONE = -1,

		PROGRAM,
		PREVIEW,

		COUNT,
	};

	enum class OverlaySlot {
		NONE = -1,

		UPSTREAM,
		DOWNSTREAM,

		COUNT
	};

	enum class OverlayTransition {
		NONE = -1,

		CUT,
		FADE,

		COUNT
	};

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

	Output&									getOutput(OutputBus bus);
	const Output&							getOutput(OutputBus bus) const;
	Output&									getProgramOutput() noexcept;
	const Output&							getProgramOutput() const noexcept;
	Output&									getPreviewOutput() noexcept;
	const Output&							getPreviewOutput() const noexcept;

	void									setBackground(OutputBus bus, size_t idx);
	size_t									getBackground(OutputBus bus) const noexcept;

	void									cut();
	void									playTransition();

	void									setTransitionProgress(float progress);
	float									getTransitionProgress() const noexcept;

	void									setTransitionSlot(OutputBus bus);
	OutputBus								getTransitionSlot() const noexcept;
	std::unique_ptr<TransitionBase>			setTransition(std::unique_ptr<TransitionBase> transition);
	const TransitionBase*					getTransition() const noexcept;


	void									setOverlayCount(OverlaySlot slot, size_t count);
	size_t									getOverlayCount(OverlaySlot slot) const;
	Keyer&									getOverlay(OverlaySlot slot, size_t idx);
	const Keyer&							getOverlay(OverlaySlot slot, size_t idx) const;
	void									setOverlayVisible(OverlaySlot slot, size_t idx, bool visible);
	bool									getOverlayVisible(OverlaySlot slot, size_t idx) const;
	void									setOverlayTransition(OverlaySlot slot, size_t idx, bool transition);
	bool									getOverlayTransition(OverlaySlot slot, size_t idx) const;

	static void 							registerCommands(Controller::Node& node);

};

}