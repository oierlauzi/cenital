#pragma once

#include "Control/Controller.h"
#include "Transitions/Base.h"
#include "Overlays/Base.h"

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
	, public Zuazo::VideoScalerBase
{
	friend MixEffectImpl;
public:
	using Input = Zuazo::Signal::PadProxy<Zuazo::Signal::Input<Zuazo::Video>>;
	using Output = Zuazo::Signal::PadProxy<Zuazo::Signal::Output<Zuazo::Video>>;

	enum class OutputBus {
		none = -1,

		program,
		preview,

		count,
	};

	enum class OverlaySlot {
		none = -1,

		upstream,
		downstream,

		count
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

	void									setBackground(OutputBus bus, size_t idx);
	size_t									getBackground(OutputBus bus) const noexcept;

	void									cut();
	void									transition();

	void									setTransitionBar(float progress);
	float									getTransitionBar() const noexcept;

	void									setTransitionSlot(OutputBus slot);
	OutputBus								getTransitionSlot() const noexcept;

	void									addTransition(std::unique_ptr<Transitions::Base> transition);
	std::unique_ptr<Transitions::Base>		removeTransition(std::string_view name);
	std::vector<Transitions::Base*>			getTransitions();
	std::vector<const Transitions::Base*>	getTransitions() const;
	Transitions::Base*						getTransition(std::string_view name);
	const Transitions::Base*				getTransition(std::string_view name) const;
	void									setSelectedTransition(std::string_view name);
	Transitions::Base*						getSelectedTransition() noexcept;
	const Transitions::Base*				getSelectedTransition() const noexcept;


	void									setOverlayCount(OverlaySlot slot, size_t count);
	size_t									getOverlayCount(OverlaySlot slot) const;

	std::unique_ptr<Overlays::Base>			setOverlay(OverlaySlot slot, size_t idx, std::unique_ptr<Overlays::Base> overlay);
	Overlays::Base*							getOverlay(OverlaySlot slot, size_t idx);
	const Overlays::Base*					getOverlay(OverlaySlot slot, size_t idx) const;
	void									setOverlayVisible(OverlaySlot slot, size_t idx, bool visible);
	bool									getOverlayVisible(OverlaySlot slot, size_t idx) const;
	void									setOverlayTransition(OverlaySlot slot, size_t idx, bool transition);
	bool									getOverlayTransition(OverlaySlot slot, size_t idx) const;
	void									setOverlaySignal(OverlaySlot slot, size_t overlay, std::string_view port, size_t idx);
	size_t									getOverlaySignal(OverlaySlot slot, size_t overlay, std::string_view port) const noexcept;


	static void 							registerCommands(Control::Controller& controller);

};

}
