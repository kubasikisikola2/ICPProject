#pragma once

#include "miniaudio.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <filesystem>
#include <mutex>

class AudioManager {
public:
	static AudioManager& getInstance()
	{
		static AudioManager instance;
		return instance;
	}

	AudioManager(AudioManager const&) = delete;
	void operator=(AudioManager const&) = delete;

	bool load(const std::string& name, const std::filesystem::path& filename);

	void setListenerPosition(float x, float y, float z, float xDir, float yDir, float zDir);

	bool play3D(const std::string& name, float soundX, float soundY, float soundZ);
	bool play2D(const std::string& name);
	bool playBGM(const std::string& name);

	void stopBGM();
	void stopAll();
private:
	AudioManager()
	{
		if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
		{
			throw std::runtime_error("Failed to initialize audio engine!");
		}

		ma_engine_listener_set_world_up(&engine, 0, 0.0f, 1.0f, 0.0f);
	};

	~AudioManager()
	{
		stopAll();
		ma_engine_uninit(&engine);
	};

	ma_engine engine{};

	std::unordered_map<std::string, std::unique_ptr<ma_sound, void(*)(ma_sound*)>> sound_bank;

	std::unordered_set<ma_sound*> active_sounds;
	std::mutex mut_active_sounds;
	ma_sound active_bgm{};
	std::mutex mut_active_bgm;

	static void sound_end_callback(void* pUserData, ma_sound* pSound);
	static void bgm_end_callback(void* pUserData, ma_sound* pSound);
};