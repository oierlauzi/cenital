#pragma once


#include <zuazo/Math/Vector.h>
#include <zuazo/Math/BezierLoop.h>

namespace Cenital {

using Shape = Zuazo::Math::CubicBezierLoop<Zuazo::Math::Vec2f>;

void generateStar(Shape& result, size_t count, float radius0, float radius1, float angle);
void generateHeart(Shape& result, float size);
void generateEllipse(Shape& result, Zuazo::Math::Vec2f size);
void generateRectangle(Shape& result, Zuazo::Math::Vec2f size);

}