#include "AudioManager.hpp"

#include "Config.hpp"

#include <iostream>

bool AudioManager::load(const std::string& name, const std::filesystem::path& filename)
{
	// create a new sound with a custom deleter
	std::unique_ptr<ma_sound, void(*)(ma_sound*)> new_snd(new ma_sound, [](ma_sound* pSnd) { ma_sound_uninit(pSnd); delete pSnd; });

	if (ma_sound_init_from_file(&engine, filename.string().c_str(), MA_SOUND_FLAG_ASYNC, nullptr, nullptr, new_snd.get()) != MA_SUCCESS) {
		std::cerr << "Failed to load sound: " << name << std::endl;
		return false;
	}

	ma_sound_set_min_distance(new_snd.get(), AUDIO_MIN_DISTANCE);
	ma_sound_set_max_distance(new_snd.get(), AUDIO_MAX_DISTANCE);
	ma_sound_set_volume(new_snd.get(), AUDIO_DEF_VOLUME);

	sound_bank.emplace(name, std::move(new_snd));
	return true;
}

void AudioManager::setListenerPosition(float x, float y, float z, float xDir, float yDir, float zDir)
{
	ma_engine_listener_set_position(&engine, 0, x, y, z);
	ma_engine_listener_set_direction(&engine, 0, xDir, yDir, zDir);
}

bool AudioManager::play3D(const std::string& name, float soundX, float soundY, float soundZ)
{
	ma_sound* original = sound_bank.at(name).get();
	ma_sound* copy_snd = new ma_sound;

	if (ma_sound_init_copy(&engine, original, MA_SOUND_FLAG_ASYNC, nullptr, copy_snd) != MA_SUCCESS) {
		std::cerr << "Failed to copy sound: " << name << std::endl;
		delete copy_snd;
		return false;
	}

	copy_snd->endCallback = sound_end_callback; // set callback for auto-deletion
	copy_snd->pEndCallbackUserData = this; // pointer to AudioManager instance

	ma_sound_seek_to_pcm_frame(copy_snd, 0); // seek to the beginning
	ma_sound_set_position(copy_snd, soundX, soundY, soundZ);

	if (ma_sound_start(copy_snd) != MA_SUCCESS) {
		std::cerr << "Failed to play sound: " << name << std::endl;
		ma_sound_uninit(copy_snd);
		delete copy_snd;
		return false;
	}

	{
		std::lock_guard<std::mutex> lg(mut_active_sounds);
		active_sounds.insert(copy_snd);
	}
	return true;
}

bool AudioManager::play2D(const std::string& name)
{
	ma_sound* original = sound_bank.at(name).get();
	ma_sound* copy_snd = new ma_sound;

	if (ma_sound_init_copy(&engine, original, MA_SOUND_FLAG_ASYNC, nullptr, copy_snd) != MA_SUCCESS) {
		std::cerr << "Failed to copy sound: " << name << std::endl;
		delete copy_snd;
		return false;
	}

	copy_snd->endCallback = sound_end_callback; // set callback for auto-deletion
	copy_snd->pEndCallbackUserData = this; // pointer to AudioManager instance

	ma_sound_seek_to_pcm_frame(copy_snd, 0); // seek to the beginning
	ma_sound_set_spatialization_enabled(copy_snd, false); // disable spatialization

	if (ma_sound_start(copy_snd) != MA_SUCCESS) {
		std::cerr << "Failed to play sound: " << name << std::endl;
		ma_sound_uninit(copy_snd);
		delete copy_snd;
		return false;
	}

	{
		std::lock_guard<std::mutex> lg(mut_active_sounds);
		active_sounds.insert(copy_snd);
	}
	return true;
}

bool AudioManager::playBGM(const std::string& name)
{
	if (ma_sound_is_playing(&active_bgm))
	{
		ma_sound_stop(&active_bgm);
		ma_sound_uninit(&active_bgm);
	}
	ma_sound* original = sound_bank.at(name).get();

	if (ma_sound_init_copy(&engine, original, MA_SOUND_FLAG_ASYNC, nullptr, &active_bgm) != MA_SUCCESS) {
		std::cerr << "Failed to copy sound: " << name << std::endl;
		return false;
	}

	active_bgm.endCallback = bgm_end_callback; // set callback for auto-deletion
	active_bgm.pEndCallbackUserData = this; // pointer to AudioManager instance

	ma_sound_seek_to_pcm_frame(&active_bgm, 0); // seek to the beginning
	ma_sound_set_spatialization_enabled(&active_bgm, false); // disable spatialization
	ma_sound_set_looping(&active_bgm, true); // loop BGM

	if (ma_sound_start(&active_bgm) != MA_SUCCESS) {
		std::cerr << "Failed to play sound: " << name << std::endl;
		ma_sound_uninit(&active_bgm);
		return false;
	}

	return true;
}

void AudioManager::stopBGM()
{
	ma_sound_stop(&active_bgm);
	ma_sound_uninit(&active_bgm);
}

void AudioManager::stopAll()
{
	stopBGM();

	std::lock_guard<std::mutex> lg(mut_active_sounds);
	for (auto snd : active_sounds)
	{
		ma_sound_stop(snd);
		ma_sound_uninit(snd);
		delete snd;
	}
	active_sounds.clear();
}

void AudioManager::fin_snd_collector_func()
{
	while (!fin_snd_collector_finish)
	{
		std::unique_lock<std::mutex> ul(mut_fin_snd_collector_sleep);
		cv_fin_snd_collector_sleep.wait(ul);

		std::lock_guard<std::mutex> lg(mut_finished_sounds);
		while (!finished_sounds.empty())
		{
			ma_sound* snd = finished_sounds.front();
			finished_sounds.pop();
			ma_sound_uninit(snd);
			delete snd;
		}
	}
}

void AudioManager::sound_end_callback(void* pUserData, ma_sound* pSound)
{
	auto t = static_cast<AudioManager*>(pUserData);
	{
		std::lock_guard<std::mutex> lg(t->mut_active_sounds);
		t->active_sounds.erase(pSound);
	}
	{
		std::lock_guard<std::mutex> lg(t->mut_finished_sounds);
		t->finished_sounds.push(pSound);
	}
	t->cv_fin_snd_collector_sleep.notify_one();
}

void AudioManager::bgm_end_callback(void* pUserData, ma_sound* pSound)
{
	auto t = static_cast<AudioManager*>(pUserData);
	ma_sound_seek_to_pcm_frame(pSound, 0);
	if (ma_sound_start(pSound) != MA_SUCCESS)
	{
		std::cerr << "Failed to replay BGM!" << std::endl;
	}
}
