#include <Overlays/Base.h>

namespace Cenital::Overlays {

using namespace Zuazo;

Base::Base(	Zuazo::Instance& instance, 
			std::string name,
			Zuazo::Utils::BufferView<const PadRef> pads,
			MoveCallback moveCbk,
			OpenCallback openCbk,
			AsyncOpenCallback asyncOpenCbk,
			CloseCallback closeCbk,
			AsyncCloseCallback asyncCloseCbk,
			UpdateCallback updateCbk,
			TransformCallback transformCbk,
			OpacityCallback opacityCbk,
			BlendingModeCallback blendingModeCbk,
			RenderingLayerCallback renderModeCbk,
			HasChangedCallback hasChangedCbk,
			HasAlphaCallback hasAlphaCbk,
			DrawCallback drawCbk,
			RenderPassCallback renderPassCbk )
	: ZuazoBase(
		instance,
		std::move(name),
		pads,
		std::move(moveCbk),
		std::move(openCbk),
		std::move(asyncOpenCbk),
		std::move(closeCbk),
		std::move(asyncCloseCbk),
		std::move(updateCbk) )
	, LayerBase(
		std::move(transformCbk),
		std::move(opacityCbk),
		std::move(blendingModeCbk),
		std::move(renderModeCbk),
		std::move(hasChangedCbk),
		std::move(hasAlphaCbk),
		std::move(drawCbk),
		std::move(renderPassCbk) )
{

}

}