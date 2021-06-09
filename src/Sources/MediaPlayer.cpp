#include <Sources/MediaPlayer.h>

#include <zuazo/Player.h>
#include <zuazo/Signal/DummyPad.h>

#include <unordered_map>

namespace Cenital::Sources {

using namespace Zuazo;

static void openHelper(ZuazoBase& base, std::unique_lock<Instance>* lock) {
	if(lock) {
		base.asyncOpen(*lock);
	} else {
		base.open();
	}
}

static void closeHelper(ZuazoBase& base, std::unique_lock<Instance>* lock) {
	if(lock) {
		base.asyncClose(*lock);
	} else {
		base.close();
	}
}





/*
 * MediaPlayerImpl
 */

struct MediaPlayerImpl {
	using Output = Signal::DummyPad<Video>;
	using ClipMap = std::unordered_map<std::string_view, std::unique_ptr<MediaPlayer::Clip>>;

	static constexpr auto UPDATE_PRIORITY = Instance::playerPriority;

	std::reference_wrapper<MediaPlayer> owner;

	Output								output;

	ClipMap								clips;
	ClipMap::iterator					currentClip;

	MediaPlayerImpl(MediaPlayer& owner)
		: owner(owner)
		, output(owner, std::string(Signal::makeOutputName<Video>()))
		, clips()
		, currentClip(clips.end())
	{
	}

	~MediaPlayerImpl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<MediaPlayer&>(base);
		output.setLayout(base);		
	}

	void open(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& mp = static_cast<MediaPlayer&>(base);
		assert(&owner.get() == &mp);

		//Open everything
		for(auto& clip : clips) {
			//Open inside a try block, as it may throw if the file is missing
			try {
				openHelper(*(clip.second), lock);
			} catch (...) {
				ZUAZO_BASE_LOG(mp, Severity::error, "Could not open " + clip.second->getName());
			}
		}

		//Enable playing
		mp.enableRegularUpdate(UPDATE_PRIORITY);
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		open(base, &lock);
		assert(lock.owns_lock());
	}

	void close(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& mp = static_cast<MediaPlayer&>(base);
		assert(&owner.get() == &mp);

		//Stop playing
		mp.disablePeriodicUpdate();
		
		//Close everything
		for(auto& clip : clips) {
			closeHelper(*(clip.second), lock);
		}
	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		close(base, &lock);
		assert(lock.owns_lock());
	}

	void update() {
		//Act as a player for the transition
		if(currentClip != clips.cend()) {
			const auto& mp = owner.get();
			const auto& instance = mp.getInstance();
			const auto deltaTime = instance.getDeltaT();

			//Advance the time for the clip
			currentClip->second->advance(deltaTime);
		}
	}

	bool addClip(std::string name, std::string path) {
		bool result;

		const auto& mp = owner.get();
		auto& instance = mp.getInstance();

		//Create the new clip
		auto clip = Utils::makeUnique<MediaPlayer::Clip>(
			instance,
			std::move(name),
			std::move(path)
		);

		//Set the same openess
		if(mp.isOpen()) {
			try {
				assert(!clip->isOpen());
				clip->open();
			} catch(...) {
				ZUAZO_BASE_LOG(mp, Severity::error, "Could not open " + clip->getName());
			}
		}
		
		//Store the name of the current clip, as the
		//iterator may get invalidated when a new element
		//is inserted. References are not invalidated.
		const auto currentClipPtr = getCurrentClip();

		//Try to add it
		std::tie(std::ignore, result) = clips.emplace(clip->getName(), std::move(clip));

		//Set the current clip if it was successfully added.
		if(result) {
			setClip(currentClipPtr ? clips.find(currentClipPtr->getName()) : clips.end());
		}

		return result;
	}

	bool rmClip(std::string_view name) {
		bool result = false;

		const auto ite = clips.find(name);
		if(ite != clips.cend()) {
			//Check if the current clip is the erased one
			if(currentClip == ite) {
				setClip(clips.end());
			} 
			assert(currentClip != ite);

			//Element exists, erase it
			clips.erase(ite);
			
			result = true;
		}

		return result;
	}


	std::vector<std::reference_wrapper<MediaPlayer::Clip>> getClips() {
		std::vector<std::reference_wrapper<MediaPlayer::Clip>> result;
		result.reserve(clips.size());
		
		std::transform(
			clips.begin(), clips.end(),
			std::back_inserter(result),
			[] (ClipMap::value_type& value) -> MediaPlayer::Clip& {
				return *(value.second);
			}
		);

		return result;
	}

	std::vector<std::reference_wrapper<const MediaPlayer::Clip>> getClips() const {
		std::vector<std::reference_wrapper<const MediaPlayer::Clip>> result;
		result.reserve(clips.size());
		
		std::transform(
			clips.begin(), clips.end(),
			std::back_inserter(result),
			[] (const ClipMap::value_type& value) -> const MediaPlayer::Clip& {
				return *(value.second);
			}
		);

		return result;
	}

	MediaPlayer::Clip* getClip(std::string_view name) {
		const auto ite = clips.find(name);
		return ite != clips.cend() ? ite->second.get() : nullptr;
	}

	const MediaPlayer::Clip* getClip(std::string_view name) const {
		const auto ite = clips.find(name);
		return ite != clips.cend() ? ite->second.get() : nullptr;
	}



	
	bool setCurrentClip(std::string_view path) {
		const auto ite = clips.find(path);
		setClip(ite);
		return ite != clips.cend();
	}

	MediaPlayer::Clip* getCurrentClip() const noexcept {
		return (currentClip != clips.cend()) ? currentClip->second.get() : nullptr;
	}


private:
	void setClip(ClipMap::iterator ite) {
		if(currentClip != ite) {
			currentClip = ite;

			//Configure the new clip
			if(currentClip != clips.cend()) {
				//The new clip is not valid
				output.getInput() << *(currentClip->second);
			} else {
				//The new clip is not valid
				output.getInput() << Signal::noSignal;
			}
		}
	}

};





/*
 * MediaPlayer
 */

MediaPlayer::MediaPlayer(	Zuazo::Instance& instance,
							std::string name )
	: Zuazo::Utils::Pimpl<MediaPlayerImpl>({}, *this)
	, Zuazo::ZuazoBase(
		instance,
		std::move(name),
		{},
		std::bind(&MediaPlayerImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&MediaPlayerImpl::open, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&MediaPlayerImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MediaPlayerImpl::close, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&MediaPlayerImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MediaPlayerImpl::update, std::ref(**this)) )
	, Zuazo::Signal::SourceLayout<Zuazo::Video>((*this)->output)
{
	//Register the output pad
	registerPad(getOutput());
}

MediaPlayer::~MediaPlayer() = default;



bool MediaPlayer::addClip(std::string name, std::string path) {
	return (*this)->addClip(std::move(name), std::move(path));
}

bool MediaPlayer::addClip(std::string path) {
	return addClip(path, path);
}

bool MediaPlayer::rmClip(std::string_view name) {
	return (*this)->rmClip(name);
}

std::vector<std::reference_wrapper<MediaPlayer::Clip>> MediaPlayer::getClips() {
	return (*this)->getClips();
}

std::vector<std::reference_wrapper<const MediaPlayer::Clip>> MediaPlayer::getClips() const {
	return (*this)->getClips();
}

MediaPlayer::Clip* MediaPlayer::getClip(std::string_view name) {
	return (*this)->getClip(name);
}

const MediaPlayer::Clip* MediaPlayer::getClip(std::string_view name) const {
	return (*this)->getClip(name);
}



bool MediaPlayer::setCurrentClip(std::string_view name) {
	return (*this)->setCurrentClip(name);
}

MediaPlayer::Clip* MediaPlayer::getCurrentClip() noexcept {
	return (*this)->getCurrentClip();
}

const MediaPlayer::Clip* MediaPlayer::getCurrentClip() const noexcept {
	return (*this)->getCurrentClip();
}


}