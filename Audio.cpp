// Audio.cpp
#include "Audio.h"
#include <string>
#include <atomic>
#include <mutex>

#ifdef USE_IRRKLANG
#include <irrKlang.h>
#pragma comment(lib, "irrKlang.lib")
using namespace irrklang;
static ISoundEngine* g_engine = nullptr;
static ISound* g_bgm = nullptr;
static std::mutex g_audioMutex;

namespace Audio {
	void init() {
		std::lock_guard<std::mutex> lk(g_audioMutex);
		if (!g_engine) g_engine = createIrrKlangDevice();
	}
	void shutdown() {
		std::lock_guard<std::mutex> lk(g_audioMutex);
		if (g_engine) {
			if (g_bgm) { g_bgm->stop(); g_bgm = nullptr; }
			g_engine->drop(); g_engine = nullptr;
		}
	}
	void playBGM(const std::string& path, float volume) {
		init();
		std::lock_guard<std::mutex> lk(g_audioMutex);
		if (!g_engine) return;
		if (g_bgm) { g_bgm->stop(); g_bgm = nullptr; }
		// play2D returns ISound* when track is not looped; use play2D with loop=true
		g_bgm = g_engine->play2D(path.c_str(), true, false, true);
		if (g_bgm) g_bgm->setVolume(volume);
	}
	void stopBGM() {
		std::lock_guard<std::mutex> lk(g_audioMutex);
		if (g_bgm) { g_bgm->stop(); g_bgm = nullptr; }
	}
	void playSE(const std::string& path, float volume) {
		init();
		std::lock_guard<std::mutex> lk(g_audioMutex);
		if (!g_engine) return;
		ISound* s = g_engine->play2D(path.c_str(), false, false, true);
		if (s) s->setVolume(volume);
	}
	void setMasterVolume(float volume) {
		std::lock_guard<std::mutex> lk(g_audioMutex);
		if (g_engine) g_engine->setSoundVolume(volume);
	}
}

#else
// Fallback stubs when USE_IRRKLANG is not defined.
namespace Audio {
	static std::atomic<float> s_masterVolume{1.0f};
	void init() {}
	void shutdown() {}
	void playBGM(const std::string& /*path*/, float /*volume*/ ) {}
	void stopBGM() {}
	void playSE(const std::string& /*path*/, float /*volume*/ ) {}
	void setMasterVolume(float vol) { s_masterVolume = vol; }
}
#endif
