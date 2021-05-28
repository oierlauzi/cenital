#include <Keyer.h>

#include <Control/Generic.h>

using namespace Zuazo;

namespace Cenital {

static void setSize(Zuazo::ZuazoBase& base, 
					const Control::Message& request,
					size_t level,
					Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setSize,
		base, request, level, response
	);
}

static void getSize(Zuazo::ZuazoBase& base, 
					const Control::Message& request,
					size_t level,
					Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getSize,
		base, request, level, response
	);
}


static void setShape(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	const auto& tokens = request.getPayload();

	//Each token will be a contour
	std::vector<Shape> contours;
	std::vector<Shape::value_type> contourPoints;
	bool success = true;
	for(size_t i = level; i < tokens.size() && success; ++i) {
		contourPoints.clear();
		std::string_view token = tokens[i];

		//Parse a list of points
		while(!token.empty() && success) {
			//Determine the limits
			const auto count = token.find(';');

			//Try to parse a vector
			Shape::value_type point;
			success = fromString(token.substr(0, count), point);
			if(success) {
				contourPoints.push_back(point);
			}

			//Advance the read position
			if(count < token.size()) {
				token.remove_prefix(count + 1); //Skip the separator
			} else {
				//Nothing left
				token = std::string_view();
			}
		}

		//In order to be successful, the point count must be multuple of the 
		//degree of the curve (3)
		if(success) {
			assert(tokens.empty()); //All the token should be parsed at this point
			success = (contourPoints.size() % Shape::degree()) == 0;
		}

		//If all went OK, add the new contour to the list
		if(success) {
			//As the multiple of degree constrain is satisfied at this point,
			//It is safe to cast the data to a packed array of arrays
			assert((contourPoints.size() % Shape::degree()) == 0);
			Utils::BufferView<const Shape::segment_data> contourData(
				reinterpret_cast<const Shape::segment_data*>(contourPoints.data()),
				contourPoints.size() / Shape::degree()
			);

			contours.emplace_back(contourData);
		}
	}

	//Check if contours were successfully parse
	if(success) {
		assert(contours.size() == (tokens.size() - level));

		//Write changes to the keyer
		assert(typeid(base) == typeid(Keyer));
		Keyer& keyer = static_cast<Keyer&>(base);
		keyer.setCrop(contours);

		//Write the response
		response.setType(Control::Message::Type::broadcast);
		response.getPayload() = request.getPayload();
	}

}

static void getShape(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == level) {
		assert(typeid(base) == typeid(Keyer));
		const Keyer& keyer = static_cast<const Keyer&>(base);
		const auto& contours = keyer.getCrop();

		auto& payload = response.getPayload();
		payload.clear();

		std::transform(
			contours.cbegin(), contours.cend(),
			std::back_inserter(payload),
			[] (const Shape& shape) -> std::string {
				std::string result;

				for(auto ite = shape.cbegin(); ite != shape.cend(); ++ite) {
					//Add the separator to all elements except the first one
					if(ite != shape.cbegin()) {
						result += ';';
					}

					//Append this point
					result += toString(*ite);
				}

				return result;
			}
		);

		response.setType(Control::Message::Type::response);

	}
}


static void setScalingMode(	Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeSetter<Keyer, ScalingMode>(
		&Keyer::setScalingMode,
		base, request, level, response
	);
}

static void getScalingMode(	Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeGetter<ScalingMode, Keyer>(
		&Keyer::getScalingMode,
		base, request, level, response
	);
}

static void enumScalingMode(Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::enumerate<ScalingMode>(base, request, level, response);
}


static void setScalingFilter(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter<Keyer, ScalingFilter>(
		&Keyer::setScalingFilter,
		base, request, level, response
	);
}

static void getScalingFilter(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter<ScalingFilter, Keyer>(
		&Keyer::getScalingFilter,
		base, request, level, response
	);
}

static void enumScalingFilter(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::enumerate<ScalingFilter>(base, request, level, response);
}


static void setBlendingMode(Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeSetter<Keyer, BlendingMode>(
		&Keyer::setBlendingMode,
		base, request, level, response
	);
}

static void getBlendingMode(Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeGetter<BlendingMode, Keyer>(
		&Keyer::getBlendingMode,
		base, request, level, response
	);
}

static void enumBlendingMode(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::enumerate<BlendingMode>(base, request, level, response);
}


static void setBlendingOpacity(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter<Keyer, float>(
		&Keyer::setOpacity,
		base, request, level, response
	);
}

static void getBlendingOpacity(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter<float, Keyer>(
		&Keyer::getOpacity,
		base, request, level, response
	);
}


static void setTransformPosition(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeSetter<Keyer, const Math::Vec3f&>(
		[] (Keyer& keyer, const Math::Vec3f& pos) -> void {
			auto transform = keyer.getTransform();
			transform.setPosition(pos);
			keyer.setTransform(transform);
		},
		base, request, level, response
	);
}

static void getTransformPosition(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeGetter<const Math::Vec3f&, Keyer>(
		[] (const Keyer& keyer) -> const Math::Vec3f& {
			return keyer.getTransform().getPosition();
		},
		base, request, level, response
	);
}


static void setTransformRotation(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeSetter<Keyer, const Math::Quaternionf&>(
		[] (Keyer& keyer, const Math::Quaternionf& rot) -> void {
			auto transform = keyer.getTransform();
			transform.setRotation(rot);
			keyer.setTransform(transform);
		},
		base, request, level, response
	);
}

static void getTransformRotation(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeGetter<const Math::Quaternionf&, Keyer>(
		[] (const Keyer& keyer) -> const Math::Quaternionf& {
			return keyer.getTransform().getRotation();
		},
		base, request, level, response
	);
}


static void setTransformScale(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter<Keyer, const Math::Vec3f&>(
		[] (Keyer& keyer, const Math::Vec3f& scale) -> void {
			auto transform = keyer.getTransform();
			transform.setScale(scale);
			keyer.setTransform(transform);
		},
		base, request, level, response
	);
}

static void getTransformScale(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter<const Math::Vec3f&, Keyer>(
		[] (const Keyer& keyer) -> const Math::Vec3f& {
			return keyer.getTransform().getScale();
		},
		base, request, level, response
	);
}


static void setLumaKeyEnabled(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setLumaKeyEnabled,
		base, request, level, response
	);
}

static void getLumaKeyEnabled(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getLumaKeyEnabled,
		base, request, level, response
	);
}


static void setLumaKeyInverted(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setLumaKeyInverted,
		base, request, level, response
	);
}

static void getLumaKeyInverted(	Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getLumaKeyInverted,
		base, request, level, response
	);
}


static void setLumaKeyMinThreshold(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setLumaKeyMinThreshold,
		base, request, level, response
	);
}

static void getLumaKeyMinThreshold(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getLumaKeyMinThreshold,
		base, request, level, response
	);
}



static void setLumaKeyMaxThreshold(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setLumaKeyMaxThreshold,
		base, request, level, response
	);
}

static void getLumaKeyMaxThreshold(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getLumaKeyMaxThreshold,
		base, request, level, response
	);
}



static void setChromaKeyEnabled(Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeyEnabled,
		base, request, level, response
	);
}

static void getChromaKeyEnabled(Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeyEnabled,
		base, request, level, response
	);
}


static void setChromaKeyHue(Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeyHue,
		base, request, level, response
	);
}

static void getChromaKeyHue(Zuazo::ZuazoBase& base, 
							const Control::Message& request,
							size_t level,
							Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeyHue,
		base, request, level, response
	);
}


static void setChromaKeyHueThreshold(	Zuazo::ZuazoBase& base, 
										const Control::Message& request,
										size_t level,
										Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeyHueThreshold,
		base, request, level, response
	);
}

static void getChromaKeyHueThreshold(	Zuazo::ZuazoBase& base, 
										const Control::Message& request,
										size_t level,
										Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeyHueThreshold,
		base, request, level, response
	);
}


static void setChromaKeyHueSmoothness(	Zuazo::ZuazoBase& base, 
										const Control::Message& request,
										size_t level,
										Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeyHueSmoothness,
		base, request, level, response
	);
}

static void getChromaKeyHueSmoothness(	Zuazo::ZuazoBase& base, 
										const Control::Message& request,
										size_t level,
										Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeyHueSmoothness,
		base, request, level, response
	);
}


static void setChromaKeySaturationThreshold(Zuazo::ZuazoBase& base, 
											const Control::Message& request,
											size_t level,
											Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeySaturationThreshold,
		base, request, level, response
	);
}

static void getChromaKeySaturationThreshold(Zuazo::ZuazoBase& base, 
											const Control::Message& request,
											size_t level,
											Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeySaturationThreshold,
		base, request, level, response
	);
}


static void setChromaKeySaturationSmoothness(	Zuazo::ZuazoBase& base, 
												const Control::Message& request,
												size_t level,
												Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeySaturationSmoothness,
		base, request, level, response
	);
}

static void getChromaKeySaturationSmoothness(	Zuazo::ZuazoBase& base, 
												const Control::Message& request,
												size_t level,
												Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeySaturationSmoothness,
		base, request, level, response
	);
}


static void setChromaKeyValueThreshold(Zuazo::ZuazoBase& base, 
											const Control::Message& request,
											size_t level,
											Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeyValueThreshold,
		base, request, level, response
	);
}

static void getChromaKeyValueThreshold(Zuazo::ZuazoBase& base, 
											const Control::Message& request,
											size_t level,
											Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeyValueThreshold,
		base, request, level, response
	);
}


static void setChromaKeyValueSmoothness(	Zuazo::ZuazoBase& base, 
												const Control::Message& request,
												size_t level,
												Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setChromaKeyValueSmoothness,
		base, request, level, response
	);
}

static void getChromaKeyValueSmoothness(	Zuazo::ZuazoBase& base, 
												const Control::Message& request,
												size_t level,
												Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getChromaKeyValueSmoothness,
		base, request, level, response
	);
}



static void setLinearKeyEnabled(Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setLinearKeyEnabled,
		base, request, level, response
	);
}

static void getLinearKeyEnabled(Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getLinearKeyEnabled,
		base, request, level, response
	);
}


static void setLinearKeyInverted(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setLinearKeyInverted,
		base, request, level, response
	);
}

static void getLinearKeyInverted(	Zuazo::ZuazoBase& base, 
									const Control::Message& request,
									size_t level,
									Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getLinearKeyInverted,
		base, request, level, response
	);
}

static void setLinearKeyChannel(Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeSetter(
		&Keyer::setLinearKeyChannel,
		base, request, level, response
	);
}

static void getLinearKeyChannel(Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::invokeGetter(
		&Keyer::getLinearKeyChannel,
		base, request, level, response
	);
}

static void enumLinearKeyChannel(Zuazo::ZuazoBase& base, 
								const Control::Message& request,
								size_t level,
								Control::Message& response ) 
{
	Control::enumerate<Keyer::LinearKeyChannel>(base, request, level, response);
}





void Keyer::registerCommands(Control::Node& node) {
	node.addPath("size",					Control::makeAttributeNode(	Cenital::setSize,
																		Cenital::getSize) );
	node.addPath("shape",					Control::makeAttributeNode(	Cenital::setShape,
																		Cenital::getShape) );

	node.addPath("scaling:mode",			Control::makeAttributeNode(	Cenital::setScalingMode,
																		Cenital::getScalingMode,
																		Cenital::enumScalingMode) );
	node.addPath("scaling:filter",			Control::makeAttributeNode(	Cenital::setScalingFilter,
																		Cenital::getScalingFilter,
																		Cenital::enumScalingFilter) );

	node.addPath("blending:mode",			Control::makeAttributeNode(	Cenital::setBlendingMode,
																		Cenital::getBlendingMode,
																		Cenital::enumBlendingMode) );
	node.addPath("blending:opacity",		Control::makeAttributeNode(	Cenital::setBlendingOpacity,
																		Cenital::getBlendingOpacity) );

	node.addPath("transform:pos",			Control::makeAttributeNode(	Cenital::setTransformPosition,
																		Cenital::getTransformPosition) );
	node.addPath("transform:rot",			Control::makeAttributeNode(	Cenital::setTransformRotation,
																		Cenital::getTransformRotation) );
	node.addPath("transform:scale",			Control::makeAttributeNode(	Cenital::setTransformScale,
																		Cenital::getTransformScale) );

	node.addPath("luma-key:ena",			Control::makeAttributeNode(	Cenital::setLumaKeyEnabled,
																		Cenital::getLumaKeyEnabled) );
	node.addPath("luma-key:inv",			Control::makeAttributeNode(	Cenital::setLumaKeyInverted,
																		Cenital::getLumaKeyInverted) );
	node.addPath("luma-key:min",			Control::makeAttributeNode(	Cenital::setLumaKeyMinThreshold,
																		Cenital::getLumaKeyMinThreshold) );
	node.addPath("luma-key:max",			Control::makeAttributeNode(	Cenital::setLumaKeyMaxThreshold,
																		Cenital::getLumaKeyMaxThreshold) );

	node.addPath("chroma-key:ena",			Control::makeAttributeNode(	Cenital::setChromaKeyEnabled,
																		Cenital::getChromaKeyEnabled) );
	node.addPath("chroma-key:hue:center",	Control::makeAttributeNode(	Cenital::setChromaKeyHue,
																		Cenital::getChromaKeyHue) );
	node.addPath("chroma-key:hue:span",		Control::makeAttributeNode(	Cenital::setChromaKeyHueThreshold,
																		Cenital::getChromaKeyHueThreshold) );
	node.addPath("chroma-key:hue:smooth",	Control::makeAttributeNode(	Cenital::setChromaKeyHueSmoothness,
																		Cenital::getChromaKeyHueSmoothness) );
	node.addPath("chroma-key:sat:min",		Control::makeAttributeNode(	Cenital::setChromaKeySaturationThreshold,
																		Cenital::getChromaKeySaturationThreshold) );
	node.addPath("chroma-key:sat:smooth",	Control::makeAttributeNode(	Cenital::setChromaKeySaturationSmoothness,
																		Cenital::getChromaKeySaturationSmoothness) );
	node.addPath("chroma-key:val:min",		Control::makeAttributeNode(	Cenital::setChromaKeyValueThreshold,
																		Cenital::getChromaKeyValueThreshold) );
	node.addPath("chroma-key:val:smooth",	Control::makeAttributeNode(	Cenital::setChromaKeyValueSmoothness,
																		Cenital::getChromaKeyValueSmoothness) );

	node.addPath("linear-key:ena",			Control::makeAttributeNode(	Cenital::setLinearKeyEnabled,
																		Cenital::getLinearKeyEnabled) );
	node.addPath("linear-key:inv",			Control::makeAttributeNode(	Cenital::setLinearKeyInverted,
																		Cenital::getLinearKeyInverted) );
	node.addPath("linear-key:ch",			Control::makeAttributeNode(	Cenital::setLinearKeyChannel,
																		Cenital::getLinearKeyChannel,
																		Cenital::enumLinearKeyChannel) );

}

}