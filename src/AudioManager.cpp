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

bool AudioManager::play3D(const std::string& name, float soundX, float soundY, float soundZ, float volume)
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
	ma_sound_set_volume(copy_snd, volume);

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

bool AudioManager::play2D(const std::string& name, float volume)
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
	ma_sound_set_volume(copy_snd, volume);

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

bool AudioManager::playBGM(const std::string& name, float volume)
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
	ma_sound_set_volume(&active_bgm, volume);

	if (ma_sound_start(&active_bgm) != MA_SUCCESS) {
		std::cerr << "Failed to play sound: " << name << std::endl;
		ma_sound_uninit(&active_bgm);
		return false;
	}

	return true;
}

void AudioManager::stopBGM()
{
	if (ma_sound_is_playing(&active_bgm))
	{
		ma_sound_stop(&active_bgm);
		ma_sound_uninit(&active_bgm);
	}
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

bool AudioManager::initMicrophone()
{
	ma_device_config micConfig;
	micConfig = ma_device_config_init(ma_device_type_capture);
	micConfig.capture.format = ma_format_f32;
	micConfig.capture.channels = 1;
	micConfig.sampleRate = 0; // Use native sample rate
	micConfig.dataCallback = mic_callback;
	micConfig.periodSizeInFrames = MIC_BUFFER_FRAMES;

	if (ma_device_init(NULL, &micConfig, &microphone))
	{
		std::cerr << "Failed to initialize microphone." << std::endl;
		return false;
	}

	if (ma_device_start(&microphone))
	{
		std::cerr << "Failed to start microphone." << std::endl;
		ma_device_uninit(&microphone);
		return false;
	}

	int bytesPerFrame = ma_get_bytes_per_frame(micConfig.capture.format, micConfig.capture.channels);
	int totalBytes = MIC_BUFFER_FRAMES * bytesPerFrame;

	std::cout << "Microphone configuration" << std::endl;
	std::cout << "Sample rate: " << microphone.sampleRate << " Hz" << std::endl;
	std::cout << "Buffer size: " << MIC_BUFFER_FRAMES << " frames, " << totalBytes << " bytes" << std::endl;

	return true;
}

float AudioManager::getMicLoudness()
{
	return mic_loudness;
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

void AudioManager::mic_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	const float* pInputFloat = (float*)pInput;

	float sumSquares{};
	for (ma_uint32 i = 0; i < frameCount; ++i)
	{
		sumSquares += pInputFloat[i] * pInputFloat[i];
	}
	float rms = std::sqrt(sumSquares / frameCount);

	AudioManager::getInstance().mic_loudness = rms;
}
