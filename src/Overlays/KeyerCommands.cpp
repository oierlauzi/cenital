#include <Overlays/Keyer.h>

#include <Control/Generic.h>
#include <Control/VideoScalingCommands.h>

namespace Cenital::Overlays {

using namespace Zuazo;
using namespace Control;

static void setSize(Controller& controller,
					ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) 
{
	invokeSetter(
		&Keyer::setSize,
		controller, base, request, level, response
	);
}

static void getSize(Controller& controller,
					ZuazoBase& base,
					const Message& request,
					size_t level,
					Message& response ) 
{
	invokeGetter(
		&Keyer::getSize,
		controller, base, request, level, response
	);
}


static void setShape(	Controller&,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
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
		response.setType(Message::Type::broadcast);
		response.getPayload() = request.getPayload();
	}

}

static void getShape(	Controller&,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
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

		response.setType(Message::Type::response);

	}
}



static void setBlendingMode(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<Keyer, BlendingMode>(
		&Keyer::setBlendingMode,
		controller, base, request, level, response
	);
}

static void getBlendingMode(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<BlendingMode, Keyer>(
		&Keyer::getBlendingMode,
		controller, base, request, level, response
	);
}

static void enumBlendingMode(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	enumerate<BlendingMode>(controller, base, request, level, response);
}


static void setBlendingOpacity(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter<Keyer, float>(
		&Keyer::setOpacity,
		controller, base, request, level, response
	);
}

static void getBlendingOpacity(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter<float, Keyer>(
		&Keyer::getOpacity,
		controller, base, request, level, response
	);
}


static void setTransformPosition(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter<Keyer, const Math::Vec3f&>(
		[] (Keyer& keyer, const Math::Vec3f& pos) -> void {
			auto transform = keyer.getTransform();
			transform.setPosition(pos);
			keyer.setTransform(transform);
		},
		controller, base, request, level, response
	);
}

static void getTransformPosition(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter<const Math::Vec3f&, Keyer>(
		[] (const Keyer& keyer) -> const Math::Vec3f& {
			return keyer.getTransform().getPosition();
		},
		controller, base, request, level, response
	);
}


static void setTransformRotation(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter<Keyer, const Math::Quaternionf&>(
		[] (Keyer& keyer, const Math::Quaternionf& rot) -> void {
			auto transform = keyer.getTransform();
			transform.setRotation(rot);
			keyer.setTransform(transform);
		},
		controller, base, request, level, response
	);
}

static void getTransformRotation(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter<const Math::Quaternionf&, Keyer>(
		[] (const Keyer& keyer) -> const Math::Quaternionf& {
			return keyer.getTransform().getRotation();
		},
		controller, base, request, level, response
	);
}


static void setTransformScale(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter<Keyer, const Math::Vec3f&>(
		[] (Keyer& keyer, const Math::Vec3f& scale) -> void {
			auto transform = keyer.getTransform();
			transform.setScale(scale);
			keyer.setTransform(transform);
		},
		controller, base, request, level, response
	);
}

static void getTransformScale(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter<const Math::Vec3f&, Keyer>(
		[] (const Keyer& keyer) -> const Math::Vec3f& {
			return keyer.getTransform().getScale();
		},
		controller, base, request, level, response
	);
}


static void setLumaKeyEnabled(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&Keyer::setLumaKeyEnabled,
		controller, base, request, level, response
	);
}

static void getLumaKeyEnabled(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter(
		&Keyer::getLumaKeyEnabled,
		controller, base, request, level, response
	);
}


static void setLumaKeyInverted(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&Keyer::setLumaKeyInverted,
		controller, base, request, level, response
	);
}

static void getLumaKeyInverted(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter(
		&Keyer::getLumaKeyInverted,
		controller, base, request, level, response
	);
}


static void setLumaKeyMinThreshold(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter(
		&Keyer::setLumaKeyMinThreshold,
		controller, base, request, level, response
	);
}

static void getLumaKeyMinThreshold(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter(
		&Keyer::getLumaKeyMinThreshold,
		controller, base, request, level, response
	);
}



static void setLumaKeyMaxThreshold(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter(
		&Keyer::setLumaKeyMaxThreshold,
		controller, base, request, level, response
	);
}

static void getLumaKeyMaxThreshold(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter(
		&Keyer::getLumaKeyMaxThreshold,
		controller, base, request, level, response
	);
}



static void setChromaKeyEnabled(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeyEnabled,
		controller, base, request, level, response
	);
}

static void getChromaKeyEnabled(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeyEnabled,
		controller, base, request, level, response
	);
}


static void setChromaKeyHue(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeyHue,
		controller, base, request, level, response
	);
}

static void getChromaKeyHue(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeyHue,
		controller, base, request, level, response
	);
}


static void setChromaKeyHueThreshold(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeyHueThreshold,
		controller, base, request, level, response
	);
}

static void getChromaKeyHueThreshold(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeyHueThreshold,
		controller, base, request, level, response
	);
}


static void setChromaKeyHueSmoothness(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeyHueSmoothness,
		controller, base, request, level, response
	);
}

static void getChromaKeyHueSmoothness(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeyHueSmoothness,
		controller, base, request, level, response
	);
}


static void setChromaKeySaturationThreshold(Controller& controller,
											ZuazoBase& base,
											const Message& request,
											size_t level,
											Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeySaturationThreshold,
		controller, base, request, level, response
	);
}

static void getChromaKeySaturationThreshold(Controller& controller,
											ZuazoBase& base,
											const Message& request,
											size_t level,
											Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeySaturationThreshold,
		controller, base, request, level, response
	);
}


static void setChromaKeySaturationSmoothness(	Controller& controller,
												ZuazoBase& base,
												const Message& request,
												size_t level,
												Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeySaturationSmoothness,
		controller, base, request, level, response
	);
}

static void getChromaKeySaturationSmoothness(	Controller& controller,
												ZuazoBase& base,
												const Message& request,
												size_t level,
												Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeySaturationSmoothness,
		controller, base, request, level, response
	);
}


static void setChromaKeyValueThreshold(Controller& controller,
											ZuazoBase& base,
											const Message& request,
											size_t level,
											Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeyValueThreshold,
		controller, base, request, level, response
	);
}

static void getChromaKeyValueThreshold(	Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeyValueThreshold,
		controller, base, request, level, response
	);
}


static void setChromaKeyValueSmoothness(Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeSetter(
		&Keyer::setChromaKeyValueSmoothness,
		controller, base, request, level, response
	);
}

static void getChromaKeyValueSmoothness(Controller& controller,
										ZuazoBase& base,
										const Message& request,
										size_t level,
										Message& response ) 
{
	invokeGetter(
		&Keyer::getChromaKeyValueSmoothness,
		controller, base, request, level, response
	);
}



static void setLinearKeyEnabled(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&Keyer::setLinearKeyEnabled,
		controller, base, request, level, response
	);
}

static void getLinearKeyEnabled(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter(
		&Keyer::getLinearKeyEnabled,
		controller, base, request, level, response
	);
}


static void setLinearKeyInverted(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeSetter(
		&Keyer::setLinearKeyInverted,
		controller, base, request, level, response
	);
}

static void getLinearKeyInverted(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	invokeGetter(
		&Keyer::getLinearKeyInverted,
		controller, base, request, level, response
	);
}

static void setLinearKeyChannel(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter(
		&Keyer::setLinearKeyChannel,
		controller, base, request, level, response
	);
}

static void getLinearKeyChannel(Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeGetter(
		&Keyer::getLinearKeyChannel,
		controller, base, request, level, response
	);
}

static void enumLinearKeyChannel(	Controller& controller,
									ZuazoBase& base,
									const Message& request,
									size_t level,
									Message& response ) 
{
	enumerate<Keyer::LinearKeyChannel>(controller, base, request, level, response);
}





void Keyer::registerCommands(Controller& controller) {
	Node configNode;

	configNode.addPath("size",					makeAttributeNode(	Overlays::setSize,
																	Overlays::getSize) );
	configNode.addPath("shape",					makeAttributeNode(	Overlays::setShape,
																	Overlays::getShape) );

	configNode.addPath("blending:mode",			makeAttributeNode(	Overlays::setBlendingMode,
																	Overlays::getBlendingMode,
																	Overlays::enumBlendingMode) );
	configNode.addPath("blending:opacity",		makeAttributeNode(	Overlays::setBlendingOpacity,
																	Overlays::getBlendingOpacity) );

	configNode.addPath("transform:pos",			makeAttributeNode(	Overlays::setTransformPosition,
																	Overlays::getTransformPosition) );
	configNode.addPath("transform:rot",			makeAttributeNode(	Overlays::setTransformRotation,
																	Overlays::getTransformRotation) );
	configNode.addPath("transform:scale",		makeAttributeNode(	Overlays::setTransformScale,
																	Overlays::getTransformScale) );

	configNode.addPath("luma-key:ena",			makeAttributeNode(	Overlays::setLumaKeyEnabled,
																	Overlays::getLumaKeyEnabled) );
	configNode.addPath("luma-key:inv",			makeAttributeNode(	Overlays::setLumaKeyInverted,
																	Overlays::getLumaKeyInverted) );
	configNode.addPath("luma-key:min",			makeAttributeNode(	Overlays::setLumaKeyMinThreshold,
																	Overlays::getLumaKeyMinThreshold) );
	configNode.addPath("luma-key:max",			makeAttributeNode(	Overlays::setLumaKeyMaxThreshold,
																	Overlays::getLumaKeyMaxThreshold) );

	configNode.addPath("chroma-key:ena",		makeAttributeNode(	Overlays::setChromaKeyEnabled,
																	Overlays::getChromaKeyEnabled) );
	configNode.addPath("chroma-key:hue:center",	makeAttributeNode(	Overlays::setChromaKeyHue,
																	Overlays::getChromaKeyHue) );
	configNode.addPath("chroma-key:hue:span",	makeAttributeNode(	Overlays::setChromaKeyHueThreshold,
																	Overlays::getChromaKeyHueThreshold) );
	configNode.addPath("chroma-key:hue:smooth",	makeAttributeNode(	Overlays::setChromaKeyHueSmoothness,
																	Overlays::getChromaKeyHueSmoothness) );
	configNode.addPath("chroma-key:sat:min",	makeAttributeNode(	Overlays::setChromaKeySaturationThreshold,
																	Overlays::getChromaKeySaturationThreshold) );
	configNode.addPath("chroma-key:sat:smooth",	makeAttributeNode(	Overlays::setChromaKeySaturationSmoothness,
																	Overlays::getChromaKeySaturationSmoothness) );
	configNode.addPath("chroma-key:val:min",	makeAttributeNode(	Overlays::setChromaKeyValueThreshold,
																	Overlays::getChromaKeyValueThreshold) );
	configNode.addPath("chroma-key:val:smooth",	makeAttributeNode(	Overlays::setChromaKeyValueSmoothness,
																	Overlays::getChromaKeyValueSmoothness) );

	configNode.addPath("linear-key:ena",		makeAttributeNode(	Overlays::setLinearKeyEnabled,
																	Overlays::getLinearKeyEnabled) );
	configNode.addPath("linear-key:inv",		makeAttributeNode(	Overlays::setLinearKeyInverted,
																	Overlays::getLinearKeyInverted) );
	configNode.addPath("linear-key:ch",			makeAttributeNode(	Overlays::setLinearKeyChannel,
																	Overlays::getLinearKeyChannel,
																	Overlays::enumLinearKeyChannel) );

	constexpr auto videoScalingWr = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	constexpr auto videoScalingRd = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	registerVideoScalingCommands<Keyer>(configNode, videoScalingWr, videoScalingRd);

	//Register it
	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(Keyer),
		ClassIndex::Entry(
			"keyer",
			std::move(configNode),
			invokeBaseConstructor<Keyer, Math::Vec2f>,
			typeid(Base)
		)	
	);
}

}