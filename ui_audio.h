// ui_audio.h - Audio system for UI feedback in Browserwind
#ifndef UI_AUDIO_H
#define UI_AUDIO_H

#include "raylib.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace UIAudio {

// ============================================================================
// AUDIO TYPES AND STRUCTURES
// ============================================================================

enum class SoundEffect {
    UI_CLICK,           // Button clicks, selections
    UI_HOVER,           // Mouse hover over interactive elements
    UI_OPEN,            // Opening menus, windows
    UI_CLOSE,           // Closing menus, windows
    UI_SUCCESS,         // Positive feedback (level up, quest complete)
    UI_ERROR,           // Error feedback (invalid action)
    UI_NOTIFICATION,    // General notifications
    UI_ITEM_PICKUP,     // Item acquisition
    UI_INVENTORY_FULL,  // Inventory full warning
    UI_QUEST_UPDATE     // Quest progress updates
};

enum class MusicTrack {
    MENU_AMBIENT,       // Menu background music
    GAME_AMBIENT,       // In-game ambient music
    COMBAT,             // Combat music
    VICTORY,            // Victory/achievement music
    DEFEAT              // Defeat music
};

struct AudioSettings {
    float masterVolume = 1.0f;      // 0.0 to 1.0
    float sfxVolume = 0.8f;         // 0.0 to 1.0
    float musicVolume = 0.6f;       // 0.0 to 1.0
    bool soundEnabled = true;
    bool musicEnabled = true;
};

class AudioManager {
public:
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    // Initialization and cleanup
    void initialize();
    void shutdown();

    // Sound effects management
    void loadSoundEffect(SoundEffect effect, const std::string& filePath);
    void playSoundEffect(SoundEffect effect, float volume = -1.0f);
    void stopAllSounds();

    // Music management
    void loadMusicTrack(MusicTrack track, const std::string& filePath);
    void playMusicTrack(MusicTrack track, bool loop = true);
    void stopMusicTrack(MusicTrack track);
    void fadeMusicTrack(MusicTrack track, float targetVolume, float duration);
    void stopAllMusic();

    // Volume controls
    void setMasterVolume(float volume);
    void setSFXVolume(float volume);
    void setMusicVolume(float volume);
    void setSoundEnabled(bool enabled);
    void setMusicEnabled(bool enabled);

    // Update (call every frame)
    void update(float deltaTime);

    // Internal volume management
    void updateVolumes();

    // Utility functions
    bool isSoundLoaded(SoundEffect effect);
    bool isMusicLoaded(MusicTrack track);
    float getMasterVolume() const { return settings.masterVolume; }
    float getSFXVolume() const { return settings.sfxVolume; }
    float getMusicVolume() const { return settings.musicVolume; }

private:
    AudioManager() = default;
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    AudioSettings settings;

    std::unordered_map<SoundEffect, Sound> soundEffects;
    std::unordered_map<MusicTrack, Music> musicTracks;
    std::unordered_map<MusicTrack, float> musicVolumes;  // Track current music volumes
    std::unordered_map<MusicTrack, float> fadeTargets;
    std::unordered_map<MusicTrack, float> fadeSpeeds;

    // Internal volume calculations
    float calculateEffectiveVolume(float baseVolume) const;
};

// ============================================================================
// GLOBAL FUNCTIONS FOR EASY ACCESS
// ============================================================================

// Sound effect shortcuts
void playClickSound();
void playHoverSound();
void playOpenSound();
void playCloseSound();
void playSuccessSound();
void playErrorSound();
void playNotificationSound();
void playItemPickupSound();
void playInventoryFullSound();
void playQuestUpdateSound();

// Music shortcuts
void playMenuMusic();
void playGameMusic();
void playCombatMusic();
void playVictoryMusic();
void playDefeatMusic();

// Volume controls
void setUIVolume(float volume);
void setMusicVolume(float volume);
void enableSound(bool enabled);
void enableMusic(bool enabled);

// Initialize/shutdown
void initializeUIAudio();
void shutdownUIAudio();

} // namespace UIAudio

#endif // UI_AUDIO_H
