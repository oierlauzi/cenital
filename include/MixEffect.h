#pragma once

#include "TransitionBase.h"
#include "Keyer.h"
#include "Control/Node.h"

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
	using Input = Zuazo::Signal::PadProxy<Zuazo::Signal::Input<Zuazo::Video>>;
	using Output = Zuazo::Signal::PadProxy<Zuazo::Signal::Output<Zuazo::Video>>;

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
	void									setProgram(size_t idx);
	size_t									getProgram() const noexcept;
	void									setPreview(size_t idx);
	size_t									getPreview() const noexcept;


	void									cut();
	void									transition();

	void									setTransitionBar(float progress);
	float									getTransitionBar() const noexcept;

	void									setTransitionSlot(OutputBus slot);
	OutputBus								getTransitionSlot() const noexcept;

	void									addTransition(std::unique_ptr<TransitionBase> transition);
	std::unique_ptr<TransitionBase>			removeTransition(std::string_view name);
	TransitionBase*							getTransition(std::string_view name);
	const TransitionBase*					getTransition(std::string_view name) const;
	void									selectTransition(std::string_view name);
	TransitionBase*							getSelectedTransition() noexcept;
	const TransitionBase*					getSelectedTransition() const noexcept;



	void									setOverlayCount(OverlaySlot slot, size_t count);
	size_t									getOverlayCount(OverlaySlot slot) const;
	void									setUpstreamOverlayCount(size_t count);
	size_t									getUpstreamOverlayCount() const;
	void									setDownstreamOverlayCount(size_t count);
	size_t									getDownstreamOverlayCount() const;

	Keyer&									getOverlay(OverlaySlot slot, size_t idx);
	const Keyer&							getOverlay(OverlaySlot slot, size_t idx) const;
	Keyer&									getUpstreamOverlay(size_t idx);
	const Keyer&							getUpstreamOverlay(size_t idx) const;
	Keyer&									getDownstreamOverlay(size_t idx);
	const Keyer&							getDownstreamOverlay(size_t idx) const;

	void									setOverlayVisible(OverlaySlot slot, size_t idx, bool visible);
	bool									getOverlayVisible(OverlaySlot slot, size_t idx) const;
	void									setUpstreamOverlayVisible(size_t idx, bool visible);
	bool									getUpstreamOverlayVisible(size_t idx) const;
	void									setDownstreamOverlayVisible(size_t idx, bool visible);
	bool									getDownstreamOverlayVisible(size_t idx) const;

	void									setOverlayTransition(OverlaySlot slot, size_t idx, bool transition);
	bool									getOverlayTransition(OverlaySlot slot, size_t idx) const;
	void									setUpstreamOverlayTransition(size_t idx, bool transition);
	bool									getUpstreamOverlayTransition(size_t idx) const;
	void									setDownstreamOverlayTransition(size_t idx, bool transition);
	bool									getDownstreamOverlayTransition(size_t idx) const;


	static void 							registerCommands(Control::Node& node);

};

}