#include "Shapes.h"

#include <zuazo/Math/Trigonometry.h>

#include <array>
#include <cassert>

namespace Cenital {

void generateStar(Shape& result, size_t count, float radius0, float radius1, float angle) {
	//Start over
	result.clear();

	const float halfStepAngle = M_PI / count;

	for(size_t i = 0; i < count; ++i) {
		//Obtain the position vectors of each of the 2 vertices
		const auto theta0 = angle + halfStepAngle*(i*2 + 0);
		const auto theta1 = angle + halfStepAngle*(i*2 + 1);

		const auto pos0 = radius0*Zuazo::Math::Vec2f(
			Zuazo::Math::cos(theta0),
			Zuazo::Math::sin(theta0)
		);

		const auto pos1 = radius1*Zuazo::Math::Vec2f(
			Zuazo::Math::cos(theta1),
			Zuazo::Math::sin(theta1)
		);

		//Add both lines
		result.lineTo(pos0);
		result.lineTo(pos1);
	}
	assert(result.size() == 2*Shape::degree()*count);
}

void generateHeart(Shape& result, float size) {
	//Heart drawn in inkscape, copied from the svg file
	constexpr std::array<std::array<Shape::value_type, Shape::degree()>, 4> HEART_POINTS = {
		//FIXME make size 1
		Zuazo::Math::Vec2f( 0.0,	 0.0),	//Middle vertex
		Zuazo::Math::Vec2f(-0.3,	-0.6),
		Zuazo::Math::Vec2f(-0.8,	-0.6),
		Zuazo::Math::Vec2f(-0.8,	-0.1),	//Left vertex
		Zuazo::Math::Vec2f(-0.8,	+0.5),
		Zuazo::Math::Vec2f(-0.2,	+1.0),
		Zuazo::Math::Vec2f( 0.0,	+1.0),	//Bottom vertex
		Zuazo::Math::Vec2f(+0.2,	+1.0),
		Zuazo::Math::Vec2f(+0.8,	+0.5),
		Zuazo::Math::Vec2f(+0.8,	-0.1),	//Right vertex
		Zuazo::Math::Vec2f(+0.8,	-0.6),
		Zuazo::Math::Vec2f(+0.3,	-0.6)
	};

	//Resize the heart to the desired size
	auto points = HEART_POINTS;
	for(auto& segmentData : points) {
		for(auto& point : segmentData) {
			point *= size;
		}
	}

	//Add the points
	//FIXME avoid reallocation
	result = Shape(Zuazo::Utils::BufferView<const decltype(points)::value_type>(points)); 
}

void generateEllipse(Shape& result, Zuazo::Math::Vec2f size) {
	//This is not actually a ellipse, but is similar (polynomial approx)
	constexpr std::array<std::array<Shape::value_type, Shape::degree()>, 4> ELLIPSE_POINTS = {
		Zuazo::Math::Vec2f(+0.5,	 0.0 ), //Right vertex
		Zuazo::Math::Vec2f(+0.5,	+0.25),
		Zuazo::Math::Vec2f(+0.25,	+0.5 ),
		Zuazo::Math::Vec2f( 0.0,	+0.5 ),	//Top vertex
		Zuazo::Math::Vec2f(-0.25,	+0.5 ),
		Zuazo::Math::Vec2f(-0.5,	+0.25),
		Zuazo::Math::Vec2f(-0.5,	 0.0 ),	//Left vertex
		Zuazo::Math::Vec2f(-0.5,	-0.25),
		Zuazo::Math::Vec2f(-0.25,	-0.5 ),
		Zuazo::Math::Vec2f( 0.0,	-0.5 ), //Bottom vertex
		Zuazo::Math::Vec2f(+0.25,	-0.5 ),
		Zuazo::Math::Vec2f(+0.5,	-0.25)
	};

	//Resize the heart to the desired size
	auto points = ELLIPSE_POINTS;
	for(auto& segmentData : points) {
		for(auto& point : segmentData) {
			point *= size;
		}
	}

	//Add the points
	//FIXME avoid reallocation
	result = Shape(Zuazo::Utils::BufferView<const decltype(points)::value_type>(points)); 
}

void generateRectangle(Shape& result, Zuazo::Math::Vec2f size) {
	constexpr std::array<Zuazo::Math::Vec2f, 4> RECT_VERTICES = {
		Zuazo::Math::Vec2f(+0.5, -0.5),
		Zuazo::Math::Vec2f(+0.5, +0.5),
		Zuazo::Math::Vec2f(-0.5, +0.5),
		Zuazo::Math::Vec2f(-0.5, -0.5),
	};

	//Add the contour
	result.clear();
	for(const auto& vertex : RECT_VERTICES) {
		result.lineTo(vertex * size);
	}
}

}