#pragma once

#include "miniaudio.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <filesystem>
#include <mutex>
#include <queue>

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

	bool initMicrophone();
	float getMicLoudness();
private:
	AudioManager()
	{
		if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
		{
			throw std::runtime_error("Failed to initialize audio engine!");
		}

		ma_engine_listener_set_world_up(&engine, 0, 0.0f, 1.0f, 0.0f);

		// Don't understand why it needs to be created like this instead of just std::thread(fin_snd_collector_func)
		// Nor do I understand how it works and can't find an explanation of this anywhere
		// But it does work :-)
		fin_snd_collector_thread = std::thread(&AudioManager::fin_snd_collector_func, this);
	};

	~AudioManager()
	{
		if (ma_device_is_started(&microphone))
		{
			ma_device_uninit(&microphone);
		}

		stopAll();
		fin_snd_collector_finish = true;
		cv_fin_snd_collector_sleep.notify_one();
		fin_snd_collector_thread.join();
		ma_engine_uninit(&engine);
	};

	ma_engine engine{};

	std::unordered_map<std::string, std::unique_ptr<ma_sound, void(*)(ma_sound*)>> sound_bank;

	std::unordered_set<ma_sound*> active_sounds;
	std::mutex mut_active_sounds;
	std::queue<ma_sound*> finished_sounds;
	std::mutex mut_finished_sounds;

	// This is necessary because calling sound_uninit() from within the callback breaks the sound engine and the miniaudio
	// documentation warns against this
	std::condition_variable cv_fin_snd_collector_sleep;
	std::mutex mut_fin_snd_collector_sleep;
	std::atomic_bool fin_snd_collector_finish;
	std::thread fin_snd_collector_thread;
	void fin_snd_collector_func();

	ma_sound active_bgm{};

	static void sound_end_callback(void* pUserData, ma_sound* pSound);
	static void bgm_end_callback(void* pUserData, ma_sound* pSound);

	ma_device microphone{};
	std::atomic<float> mic_loudness;
	static void mic_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};