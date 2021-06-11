#pragma once

#include <zuazo/ZuazoBase.h>
#include <zuazo/LayerBase.h>

namespace Cenital::Overlays {

class Base 
	: public Zuazo::ZuazoBase
	, public Zuazo::LayerBase
{
public:
	Base(	Zuazo::Instance& instance, 
			std::string name,
			Zuazo::Utils::BufferView<const PadRef> pads = {},
			MoveCallback moveCbk = {},
			OpenCallback openCbk = {},
			AsyncOpenCallback asyncOpenCbk = {},
			CloseCallback closeCbk = {},
			AsyncCloseCallback asyncCloseCbk = {},
			UpdateCallback updateCbk = {},
			TransformCallback transformCbk = {},
			OpacityCallback opacityCbk = {},
			BlendingModeCallback blendingModeCbk = {},
			RenderingLayerCallback renderModeCbk = {},
			HasChangedCallback hasChangedCbk = {},
			HasAlphaCallback hasAlphaCbk = {},
			DrawCallback drawCbk = {},
			RenderPassCallback renderPassCbk = {} );

};

}