#include <Delusive/Internal/Audio/DelusiveAudio.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>

DelusiveAudio::~DelusiveAudio() {
	Shutdown();
}

bool DelusiveAudio::Init() {
	//No audio device is not fatal, the game just runs silent
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		std::cerr << "[DelusiveAudio] Audio init failed: " << SDL_GetError() << std::endl;
		return false;
	}

	device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	if (!device) {
		std::cerr << "[DelusiveAudio] Failed to open playback device: " << SDL_GetError() << std::endl;
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	SDL_AudioSpec deviceSpec{};
	SDL_GetAudioDeviceFormat(device, &deviceSpec, nullptr);
	for (auto& voice : voices) {
		voice = SDL_CreateAudioStream(&deviceSpec, &deviceSpec);
	}

	//Bound streams are mixed by the device
	if (!SDL_BindAudioStreams(device, voices.data(), MAX_VOICES)) {
		std::cerr << "[DelusiveAudio] Failed to bind voices: " << SDL_GetError() << std::endl;
		Shutdown();
		return false;
	}

	SetMasterVolume(masterVolume);
	return true;
}

void DelusiveAudio::Shutdown() {
	if (!device) return;

	for (auto& voice : voices) {
		SDL_DestroyAudioStream(voice);
		voice = nullptr;
	}
	SDL_CloseAudioDevice(device);
	device = 0;
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool DelusiveAudio::Load(const std::string& name) {
	return FetchSound(name) != nullptr;
}

void DelusiveAudio::Play(const std::string& name, float volume) {
	if (!device || name.empty()) return;

	const Sound* sound = FetchSound(name);
	if (!sound) return;

	SDL_AudioStream* voice = FetchFreeVoice();
	SDL_ClearAudioStream(voice);
	SDL_SetAudioStreamFormat(voice, &sound->spec, nullptr);
	SDL_SetAudioStreamGain(voice, std::max(volume, 0.0f));
	SDL_PutAudioStreamData(voice, sound->data.data(), static_cast<int>(sound->data.size()));
}

void DelusiveAudio::StopAll() {
	for (auto* voice : voices) {
		if (voice) SDL_ClearAudioStream(voice);
	}
}

void DelusiveAudio::SetMasterVolume(float volume) {
	masterVolume = std::max(volume, 0.0f);
	if (device) SDL_SetAudioDeviceGain(device, masterVolume);
}

const DelusiveAudio::Sound* DelusiveAudio::FetchSound(const std::string& name) {
	auto it = sounds.find(name);
	if (it != sounds.end()) return &it->second;
	if (missing.count(name)) return nullptr;

	const std::string path = SOUND_FOLDER + name;
	Sound sound;
	Uint8* buffer = nullptr;
	Uint32 length = 0;
	if (!SDL_LoadWAV(path.c_str(), &sound.spec, &buffer, &length)) {
		std::cerr << "[DelusiveAudio] Failed to load " << path << ": " << SDL_GetError() << std::endl;
		missing.insert(name);
		return nullptr;
	}

	sound.data.assign(buffer, buffer + length);
	SDL_free(buffer);
	return &sounds.emplace(name, std::move(sound)).first->second;
}

SDL_AudioStream* DelusiveAudio::FetchFreeVoice() const {
	//All voices busy: cut the one closest to finishing
	SDL_AudioStream* best = voices.front();
	int leastQueued = SDL_GetAudioStreamQueued(best);
	for (auto* voice : voices) {
		const int queued = SDL_GetAudioStreamQueued(voice);
		if (queued == 0 && SDL_GetAudioStreamAvailable(voice) == 0) return voice;
		if (queued < leastQueued) {
			best = voice;
			leastQueued = queued;
		}
	}
	return best;
}
