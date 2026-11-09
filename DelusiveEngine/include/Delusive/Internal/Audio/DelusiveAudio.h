#pragma once
#include <SDL3/SDL_audio.h>
#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class DelusiveAudio {
public:
	DelusiveAudio() = default;
	~DelusiveAudio();

	DelusiveAudio(const DelusiveAudio&) = delete;
	DelusiveAudio& operator=(const DelusiveAudio&) = delete;

	bool Init();
	void Shutdown();
	bool IsReady() const { return device != 0; }

	//Sounds are .wav files named relative to SOUND_FOLDER, loaded on first use
	bool Load(const std::string& name);
	void Play(const std::string& name, float volume = 1.0f);
	void StopAll();

	void SetMasterVolume(float);
	float GetMasterVolume() const { return masterVolume; }
private:
	struct Sound {
		SDL_AudioSpec spec{};
		std::vector<Uint8> data;
	};
	static constexpr int MAX_VOICES = 16;

	SDL_AudioDeviceID device = 0;
	std::array<SDL_AudioStream*, MAX_VOICES> voices{};
	std::unordered_map<std::string, Sound> sounds;
	//Missing files log once, then stay silent
	std::unordered_set<std::string> missing;
	float masterVolume = 1.0f;

	const Sound* FetchSound(const std::string&);
	SDL_AudioStream* FetchFreeVoice() const;
};
