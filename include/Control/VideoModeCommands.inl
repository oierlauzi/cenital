#include "VideoModeCommands.h"

#include "Generic.h"

#include <set>

namespace Cenital::Control {

constexpr bool test(VideoModeAttributes bitfield, VideoModeAttributes bits) {
	return (bitfield & bits) != VideoModeAttributes::none;
}

inline void setNegotiator(Zuazo::VideoBase& element, Zuazo::DefaultVideoModeNegotiator negotiator) {
	element.setVideoModeNegotiationCallback(std::move(negotiator));
}

inline const Zuazo::DefaultVideoModeNegotiator& getNegotiator(const Zuazo::VideoBase& element) {
	const auto& callback = element.getVideoModeNegotiationCallback();
	const auto* negotiator = callback.target<Zuazo::DefaultVideoModeNegotiator>();

	assert(negotiator);
	return *negotiator;
}

template<typename T, typename FS, typename FG>
inline std::set<T> getVideoModeAttributeSupport(const Zuazo::VideoBase& base,
												FS&& setter,
												FG&& getter )
{
	std::set<T> result;

	//Bypass negotiation for this attribute
	auto restrictions = getNegotiator(base);
	setter(static_cast<Zuazo::VideoMode&>(restrictions), Zuazo::Utils::Any<T>());

	const auto& compatibility = base.getVideoModeCompatibility();
	for(const auto& comp : compatibility) {
		//Intersect the restrictions with the compatibility
		//It is intersected with a 
		const auto supported = comp.intersect(restrictions);
		if(static_cast<bool>(supported)) {
			//This compatibility meets the restrictions
			const auto& attribute = getter(supported);
			switch (attribute.getType()) {
			case Zuazo::Utils::LimitType::any: {
				//Any value
				using EnumTraits = Zuazo::Utils::EnumTraits<T>;
				const auto first = EnumTraits::first();
				const auto last = EnumTraits::last();

				for(auto i = first; i != last; ++i) {
					result.insert(i);
				}
				break;
			}

			case Zuazo::Utils::LimitType::range: {
				//Range
				const auto& range = attribute.getRange();

				for(auto i = range.getMin(); i != range.getMax(); ++i) {
					result.insert(i);
				}
				break;
			}
			
			case Zuazo::Utils::LimitType::discreteRange: {
				//Discrete range
				const auto& discreteRange = attribute.getDiscreteRange();

				for(auto i = discreteRange.getMin(); i <= discreteRange.getMax(); i += discreteRange.getStep()) {
					result.insert(i);
				}
				break;
			}

			case Zuazo::Utils::LimitType::discrete: {
				//Discrete values
				const auto& discrete = attribute.getDiscrete();
				std::copy(
					discrete.cbegin(), discrete.cend(),
					std::inserter(result, result.end())
				);

				break;
			}

			case Zuazo::Utils::LimitType::mustBe: {
				//Single value
				const auto& mustBe = attribute.getMustBe();
				result.insert(mustBe.getValue());
				break;
			}
			
			default:
				//After intersecting with a list and checking that it 
				//is valid, only one of the types listed above is expected.
				assert(false);
				break;
			}

		}
	}

	return result;
}



template<typename T, typename Q, typename FS>
inline void setVideoModeAttribute(	FS&& setter,
									Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	invokeSetter<T, Q>(
		[&setter] (T& el, Q value) {
			auto negotiator = getNegotiator(el);

			//Set the new value in the negotiator
			setter(static_cast<Zuazo::VideoMode&>(negotiator), Zuazo::Utils::MustBe<Q>(std::move(value)));

			//Set the negotiator
			setNegotiator(el, std::move(negotiator));
		},
		controller, base, request, level, response
	);
}

template<typename Q, typename T, typename FG>
inline void getVideoModeAttribute(	FG&& getter,
									Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	invokeGetter<Q, T>(
		[&getter] (T& el) -> Q {
			const auto& videoMode = el.getVideoMode();
			const auto& limit = getter(static_cast<const Zuazo::VideoMode&>(videoMode));
			return limit.hasValue() ? limit.value() : Q();
		},
		controller, base, request, level, response
	);
}

template<typename T, typename Q, typename FS, typename FG>
inline void enumVideoModeAttribute(	FS&& setter,
									FG&& getter,
									Controller&,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		assert(typeid(base) == typeid(T));
		T& element = static_cast<T&>(base);

		//Query attrubute support
		const auto support = getVideoModeAttributeSupport<Q>(
			element,
			std::forward<FS>(setter),
			std::forward<FG>(getter)
		);

		auto& payload = response.getPayload();
		payload.clear();
		payload.reserve(support.size());
		std::transform(
			support.cbegin(), support.cend(),
			std::back_inserter(payload),
			[] (const Q& item) -> std::string {
				return std::string(Zuazo::toString(item));
			}
		);
		response.setType(Message::Type::response);
	}
}





template<typename T>
inline void setVideoModeFrameRate(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	setVideoModeAttribute<T, Zuazo::Rate>(
		std::mem_fn(&Zuazo::VideoMode::setFrameRate),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeFrameRate(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	getVideoModeAttribute<Zuazo::Rate, T>(
		std::mem_fn(&Zuazo::VideoMode::getFrameRate),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModeResolution(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	setVideoModeAttribute<T, Zuazo::Resolution>(
		std::mem_fn(&Zuazo::VideoMode::setResolution),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeResolution(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	getVideoModeAttribute<Zuazo::Resolution, T>(
		std::mem_fn(&Zuazo::VideoMode::getResolution),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModePixelAspectRatio(	Controller& controller,
											Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response )
{
	setVideoModeAttribute<T, Zuazo::AspectRatio>(
		std::mem_fn(&Zuazo::VideoMode::setPixelAspectRatio),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModePixelAspectRatio(	Controller& controller,
											Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response )
{
	getVideoModeAttribute<Zuazo::AspectRatio, T>(
		std::mem_fn(&Zuazo::VideoMode::getPixelAspectRatio),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModeColorPrimaries(	Controller& controller,
										Zuazo::ZuazoBase& base, 
										const Message& request,
										size_t level,
										Message& response )
{
	setVideoModeAttribute<T, Zuazo::ColorPrimaries>(
		std::mem_fn(&Zuazo::VideoMode::setColorPrimaries),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeColorPrimaries(	Controller& controller,
										Zuazo::ZuazoBase& base, 
										const Message& request,
										size_t level,
										Message& response )
{
	getVideoModeAttribute<Zuazo::ColorPrimaries, T>(
		std::mem_fn(&Zuazo::VideoMode::getColorPrimaries),
		controller, base, request, level, response
	);
}

template<typename T>
inline void enumVideoModeColorPrimaries(Controller& controller,
										Zuazo::ZuazoBase& base, 
										const Message& request,
										size_t level,
										Message& response )
{
	enumVideoModeAttribute<T, Zuazo::ColorPrimaries>(
		std::mem_fn(&Zuazo::VideoMode::setColorPrimaries),
		std::mem_fn(&Zuazo::VideoMode::getColorPrimaries),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModeColorModel(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	setVideoModeAttribute<T, Zuazo::ColorModel>(
		std::mem_fn(&Zuazo::VideoMode::setColorModel),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeColorModel(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	getVideoModeAttribute<Zuazo::ColorModel, T>(
		std::mem_fn(&Zuazo::VideoMode::getColorModel),
		controller, base, request, level, response
	);
}

template<typename T>
inline void enumVideoModeColorModel(Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	enumVideoModeAttribute<T, Zuazo::ColorModel>(
		std::mem_fn(&Zuazo::VideoMode::setColorModel),
		std::mem_fn(&Zuazo::VideoMode::getColorModel),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModeColorTransferFunction(	Controller& controller,
												Zuazo::ZuazoBase& base, 
												const Message& request,
												size_t level,
												Message& response )
{
	setVideoModeAttribute<T, Zuazo::ColorTransferFunction>(
		std::mem_fn(&Zuazo::VideoMode::setColorTransferFunction),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeColorTransferFunction(	Controller& controller,
												Zuazo::ZuazoBase& base, 
												const Message& request,
												size_t level,
												Message& response )
{
	getVideoModeAttribute<Zuazo::ColorTransferFunction, T>(
		std::mem_fn(&Zuazo::VideoMode::getColorTransferFunction),
		controller, base, request, level, response
	);
}

template<typename T>
inline void enumVideoModeColorTransferFunction(	Controller& controller,
												Zuazo::ZuazoBase& base, 
												const Message& request,
												size_t level,
												Message& response )
{
	enumVideoModeAttribute<T, Zuazo::ColorTransferFunction>(
		std::mem_fn(&Zuazo::VideoMode::setColorTransferFunction),
		std::mem_fn(&Zuazo::VideoMode::getColorTransferFunction),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModeColorSubsampling(	Controller& controller,
											Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response )
{
	setVideoModeAttribute<T, Zuazo::ColorSubsampling>(
		std::mem_fn(&Zuazo::VideoMode::setColorSubsampling),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeColorSubsampling(	Controller& controller,
											Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response )
{
	getVideoModeAttribute<Zuazo::ColorSubsampling, T>(
		std::mem_fn(&Zuazo::VideoMode::getColorSubsampling),
		controller, base, request, level, response
	);
}

template<typename T>
inline void enumVideoModeColorSubsampling(	Controller& controller,
											Zuazo::ZuazoBase& base, 
											const Message& request,
											size_t level,
											Message& response )
{
	enumVideoModeAttribute<T, Zuazo::ColorSubsampling>(
		std::mem_fn(&Zuazo::VideoMode::setColorSubsampling),
		std::mem_fn(&Zuazo::VideoMode::getColorSubsampling),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModeColorRange(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	setVideoModeAttribute<T, Zuazo::ColorRange>(
		std::mem_fn(&Zuazo::VideoMode::setColorRange),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeColorRange(	Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	getVideoModeAttribute<Zuazo::ColorRange, T>(
		std::mem_fn(&Zuazo::VideoMode::getColorRange),
		controller, base, request, level, response
	);
}

template<typename T>
inline void enumVideoModeColorRange(Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	enumVideoModeAttribute<T, Zuazo::ColorRange>(
		std::mem_fn(&Zuazo::VideoMode::setColorRange),
		std::mem_fn(&Zuazo::VideoMode::getColorRange),
		controller, base, request, level, response
	);
}



template<typename T>
inline void setVideoModeColorFormat(Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	setVideoModeAttribute<T, Zuazo::ColorFormat>(
		std::mem_fn(&Zuazo::VideoMode::setColorFormat),
		controller, base, request, level, response
	);
}

template<typename T>
inline void getVideoModeColorFormat(Controller& controller,
									Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	getVideoModeAttribute<Zuazo::ColorFormat, T>(
		std::mem_fn(&Zuazo::VideoMode::getColorFormat),
		controller, base, request, level, response
	);
}

template<typename T>
inline void enumVideoModeColorFormat(	Controller& controller,
										Zuazo::ZuazoBase& base, 
										const Message& request,
										size_t level,
										Message& response )
{
	enumVideoModeAttribute<T, Zuazo::ColorFormat>(
		std::mem_fn(&Zuazo::VideoMode::setColorFormat),
		std::mem_fn(&Zuazo::VideoMode::getColorFormat),
		controller, base, request, level, response
	);
}





template<typename T>
inline void registerVideoModeCommands(	Node& node,
										VideoModeAttributes wr,
										VideoModeAttributes rd,
										const std::string& parentPath )
{
	const auto any = wr | rd;

	if(test(any, VideoModeAttributes::frameRate)) {
		node.addPath(
			parentPath + ":frame-rate",
			makeAttributeNode(
				test(wr, VideoModeAttributes::frameRate) ? setVideoModeFrameRate<T> : nullptr,
				test(rd, VideoModeAttributes::frameRate) ? getVideoModeFrameRate<T> : nullptr
			)
		);
	}
	if(test(any, VideoModeAttributes::resolution)) {
		node.addPath(
			parentPath + ":resolution",
			makeAttributeNode(
				test(wr, VideoModeAttributes::resolution) ? setVideoModeResolution<T> : nullptr,
				test(rd, VideoModeAttributes::resolution) ? getVideoModeResolution<T> : nullptr
			)
		);
	}
	if(test(any, VideoModeAttributes::pixelAspectRatio)) {
		node.addPath(
			parentPath + ":pixel-aspect-ratio",
			makeAttributeNode(
				test(wr, VideoModeAttributes::pixelAspectRatio) ? setVideoModePixelAspectRatio<T> : nullptr,
				test(rd, VideoModeAttributes::pixelAspectRatio) ? getVideoModePixelAspectRatio<T> : nullptr
			)
		);
	}
	if(test(any, VideoModeAttributes::colorPrimaries)) {
		node.addPath(
			parentPath + ":color-primaries",
			makeAttributeNode(
				test(wr, VideoModeAttributes::colorPrimaries) ? setVideoModeColorPrimaries<T> : nullptr,
				test(rd, VideoModeAttributes::colorPrimaries) ? getVideoModeColorPrimaries<T> : nullptr,
				enumVideoModeColorPrimaries<T>
			)
		);
	}
	if(test(any, VideoModeAttributes::colorModel)) {
		node.addPath(
			parentPath + ":color-model",
			makeAttributeNode(
				test(wr, VideoModeAttributes::colorModel) ? setVideoModeColorModel<T> : nullptr,
				test(rd, VideoModeAttributes::colorModel) ? getVideoModeColorModel<T> : nullptr,
				enumVideoModeColorModel<T>
			)
		);
	}
	if(test(any, VideoModeAttributes::colorTransferFunction)) {
		node.addPath(
			parentPath + ":color-transfer-function",
			makeAttributeNode(
				test(wr, VideoModeAttributes::colorTransferFunction) ? setVideoModeColorTransferFunction<T> : nullptr,
				test(rd, VideoModeAttributes::colorTransferFunction) ? getVideoModeColorTransferFunction<T> : nullptr,
				enumVideoModeColorTransferFunction<T>
			)
		);
	}
	if(test(any, VideoModeAttributes::colorSubsampling)) {
		node.addPath(
			parentPath + ":color-subsampling",
			makeAttributeNode(
				test(wr, VideoModeAttributes::colorSubsampling) ? setVideoModeColorSubsampling<T> : nullptr,
				test(rd, VideoModeAttributes::colorSubsampling) ? getVideoModeColorSubsampling<T> : nullptr,
				enumVideoModeColorSubsampling<T>
			)
		);
	}
	if(test(any, VideoModeAttributes::colorRange)) {
		node.addPath(
			parentPath + ":color-range",
			makeAttributeNode(
				test(wr, VideoModeAttributes::colorRange) ? setVideoModeColorRange<T> : nullptr,
				test(rd, VideoModeAttributes::colorRange) ? getVideoModeColorRange<T> : nullptr,
				enumVideoModeColorRange<T>
			)
		);
	}
	if(test(any, VideoModeAttributes::colorFormat)) {
		node.addPath(
			parentPath + ":color-format",
			makeAttributeNode(
				test(wr, VideoModeAttributes::colorFormat) ? setVideoModeColorFormat<T> : nullptr,
				test(rd, VideoModeAttributes::colorFormat) ? getVideoModeColorFormat<T> : nullptr,
				enumVideoModeColorFormat<T>
			)
		);
	}
}

}