#include "VideoScalingCommands.h"

#include "Generic.h"

namespace Cenital::Control {

constexpr bool test(VideoScalingAttributes bitfield, VideoScalingAttributes bits) {
	return (bitfield & bits) != VideoScalingAttributes::none;
}





template<typename T>
inline void setVideoScalingMode(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	invokeSetter<T, Zuazo::ScalingMode>(
		&T::setScalingMode,
		base, request, level, response
	);
}

template<typename T>
inline void getVideoScalingMode(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	invokeGetter<Zuazo::ScalingMode, T>(
		&T::getScalingMode,
		base, request, level, response
	);
}

inline void enumVideoScalingMode(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	enumerate<Zuazo::ScalingMode>(
		base, request, level, response
	);
}


template<typename T>
inline void setVideoScalingFilter(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	invokeSetter<T, Zuazo::ScalingFilter>(
		&T::setScalingFilter,
		base, request, level, response
	);
}

template<typename T>
inline void getVideoScalingFilter(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	invokeGetter<Zuazo::ScalingFilter, T>(
		&T::getScalingFilter,
		base, request, level, response
	);
}

inline void enumVideoScalingFilter(	Zuazo::ZuazoBase& base, 
									const Message& request,
									size_t level,
									Message& response )
{
	enumerate<Zuazo::ScalingFilter>(
		base, request, level, response
	);
}



template<typename T>
inline void registerVideoScalingModeAttribute(	Node& node,
												bool wr,
												bool rd,
												const std::string& parentPath )
{
	if(wr || rd) {
		node.addPath(
			parentPath + ":mode",
			makeAttributeNode(
				wr ? setVideoScalingMode<T> : nullptr,
				rd ? getVideoScalingMode<T> : nullptr,
				enumVideoScalingMode
			)
		);	
	}
}

template<typename T>
inline void registerVideoScalingFilterAttribute(Node& node,
												bool wr,
												bool rd,
												const std::string& parentPath )
{
	if(wr || rd) {
		node.addPath(
			parentPath + ":filter",
			makeAttributeNode(
				wr ? setVideoScalingFilter<T> : nullptr,
				rd ? getVideoScalingFilter<T> : nullptr,
				enumVideoScalingFilter
			)
		);	
	}
}

template<typename T>
inline void registerVideoScalingCommands(	Node& node,
											VideoScalingAttributes wr,
											VideoScalingAttributes rd,
											const std::string& parentPath )
{
	registerVideoScalingModeAttribute<T>(
		node,
		test(wr, VideoScalingAttributes::mode),
		test(rd, VideoScalingAttributes::mode),
		parentPath
	);
	registerVideoScalingFilterAttribute<T>(
		node,
		test(wr, VideoScalingAttributes::filter),
		test(rd, VideoScalingAttributes::filter),
		parentPath
	);
}

}