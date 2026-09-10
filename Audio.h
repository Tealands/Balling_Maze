// Audio.h
#pragma once
#include <string>

namespace Audio {
	// Initialize audio system (optional)
	void init();
	void shutdown();

	// Play background music (looped)
	void playBGM(const std::string& path, float volume = 1.0f);
	void stopBGM();

	// Play one-shot sound effect
	void playSE(const std::string& path, float volume = 1.0f);

	// Global volume
	void setMasterVolume(float volume);
}

// To enable real irrKlang support, define USE_IRRKLANG and make sure irrKlang headers/libs are available.
