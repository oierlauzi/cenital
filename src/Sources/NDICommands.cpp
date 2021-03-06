#include <Sources/NDI.h>

#include <Mixer.h>
#include <Control/Generic.h>
#include <Control/VideoModeCommands.h>

#include <zuazo/NDI/Finder.h>

namespace Cenital::Sources {

using namespace Zuazo;
using namespace Control;



static Zuazo::NDI::Finder& getSourceFinder() {
	static std::unique_ptr<Zuazo::NDI::Finder> singleton;

	if(!singleton) {
		singleton = Utils::makeUnique<Zuazo::NDI::Finder>();
	}

	assert(singleton);
	return *singleton;
}

static void setSource(	Controller&,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level + 1) {
		const auto& sourceName = tokens[level];

		//List the available sources
		const auto sources = getSourceFinder().getCurrentSources();

		//Check if the source exists
		const auto ite = std::find_if(
			sources.cbegin(), sources.cend(),
			[&sourceName] (const NDI::Source source) -> bool {
				return source.getName() == sourceName;
			}
		);

		if(ite != sources.cend()) {
			//Found it!
			assert(typeid(base) == typeid(NDI));
			auto& ndi = static_cast<NDI&>(base);

			//Set the source
			ndi.setSource(*ite);

			//Elaborate the response
			response.getPayload() = tokens;
			response.setType(Message::Type::broadcast);
		}
	}
}

static void getSource(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<std::string, NDI>(
		[] (const NDI& ndi) -> std::string {
			return ndi.getSource().getName();
		},
		controller, base, request, level, response
	);
}

static void enumSource(	Controller&,
						ZuazoBase&, 
						const Message& request,
						size_t level,
						Message& response )
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		//List the available sources
		const auto sources = getSourceFinder().getCurrentSources();

		auto& payload = response.getPayload();
		payload.clear();
		payload.reserve(sources.size());
		std::transform(
			sources.cbegin(), sources.cend(),
			std::back_inserter(payload),
			[] (const NDI::Source& source) -> std::string {
				return std::string(source.getName());
			}
		);
		response.setType(Message::Type::response);
	}
}

static void unsetSource(Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	invokeSetter<NDI>(
		[] (NDI& ndi) {
			ndi.setSource(NDI::Source());
		},
		controller, base, request, level, response
	);
}




void NDI::registerCommands(Controller& controller) {


	Node configNode({
		{ "source",				makeAttributeNode(	Sources::setSource,
													Sources::getSource,
													Sources::enumSource,
													Sources::unsetSource ) },

	});

	constexpr auto videoModeWr = VideoModeAttributes::none;
	constexpr auto videoModeRd = VideoModeAttributes::all;
	registerVideoModeCommands<NDI>(configNode, videoModeWr, videoModeRd);

	//Register it
	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(NDI),
		ClassIndex::Entry(
			"input-ndi",
			std::move(configNode),
			invokeBaseConstructor<NDI>,
			typeid(ZuazoBase)
		)	
	);
}

}