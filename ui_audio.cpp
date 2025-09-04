// ui_audio.cpp - Implementation of the UI audio system
#include "ui_audio.h"
#include <iostream>

namespace UIAudio {

// ============================================================================
// AUDIO MANAGER IMPLEMENTATION
// ============================================================================

void AudioManager::initialize() {
    // Initialize audio device
    InitAudioDevice();
    std::cout << "UI Audio system initialized successfully" << std::endl;

    // Pre-load some basic sound effects (placeholder for when files are available)
    // Note: These will fail gracefully if files don't exist
    loadSoundEffect(SoundEffect::UI_CLICK, "assets/sounds/ui_click.wav");
    loadSoundEffect(SoundEffect::UI_HOVER, "assets/sounds/ui_hover.wav");
    loadSoundEffect(SoundEffect::UI_OPEN, "assets/sounds/ui_open.wav");
    loadSoundEffect(SoundEffect::UI_CLOSE, "assets/sounds/ui_close.wav");
    loadSoundEffect(SoundEffect::UI_SUCCESS, "assets/sounds/ui_success.wav");
    loadSoundEffect(SoundEffect::UI_ERROR, "assets/sounds/ui_error.wav");
    loadSoundEffect(SoundEffect::UI_NOTIFICATION, "assets/sounds/ui_notification.wav");

    // Load music tracks (placeholder)
    loadMusicTrack(MusicTrack::MENU_AMBIENT, "assets/sounds/menu_ambient.ogg");
    loadMusicTrack(MusicTrack::GAME_AMBIENT, "assets/sounds/game_ambient.ogg");
}

void AudioManager::shutdown() {
    // Stop all audio
    stopAllSounds();
    stopAllMusic();

    // Unload all sound effects
    for (auto& pair : soundEffects) {
        UnloadSound(pair.second);
    }
    soundEffects.clear();

    // Unload all music tracks
    for (auto& pair : musicTracks) {
        UnloadMusicStream(pair.second);
    }
    musicTracks.clear();

    // Close audio device
    CloseAudioDevice();
    std::cout << "UI Audio system shutdown successfully" << std::endl;
}

void AudioManager::loadSoundEffect(SoundEffect effect, const std::string& filePath) {
    Sound sound = LoadSound(filePath.c_str());
    if (sound.stream.buffer != nullptr) {  // Check if sound loaded successfully
        soundEffects[effect] = sound;
        std::cout << "Loaded sound effect: " << filePath << std::endl;
    } else {
        std::cout << "Warning: Failed to load sound effect: " << filePath << std::endl;
    }
}

void AudioManager::playSoundEffect(SoundEffect effect, float volume) {
    if (!settings.soundEnabled) return;

    auto it = soundEffects.find(effect);
    if (it != soundEffects.end()) {
        float effectiveVolume = (volume < 0.0f) ?
            calculateEffectiveVolume(settings.sfxVolume) : volume;
        SetSoundVolume(it->second, effectiveVolume);
        PlaySound(it->second);
    }
}

void AudioManager::stopAllSounds() {
    for (auto& pair : soundEffects) {
        StopSound(pair.second);
    }
}

void AudioManager::loadMusicTrack(MusicTrack track, const std::string& filePath) {
    Music music = LoadMusicStream(filePath.c_str());
    if (music.stream.buffer != nullptr) {  // Check if music loaded successfully
        musicTracks[track] = music;
        std::cout << "Loaded music track: " << filePath << std::endl;
    } else {
        std::cout << "Warning: Failed to load music track: " << filePath << std::endl;
    }
}

void AudioManager::playMusicTrack(MusicTrack track, bool loop) {
    if (!settings.musicEnabled) return;

    auto it = musicTracks.find(track);
    if (it != musicTracks.end()) {
        float effectiveVolume = calculateEffectiveVolume(settings.musicVolume);
        SetMusicVolume(it->second, effectiveVolume);
        musicVolumes[track] = effectiveVolume;  // Track current volume
        PlayMusicStream(it->second);
        if (loop) {
            // Music will loop automatically when using PlayMusicStream with loop enabled
        }
    }
}

void AudioManager::stopMusicTrack(MusicTrack track) {
    auto it = musicTracks.find(track);
    if (it != musicTracks.end()) {
        StopMusicStream(it->second);
    }
}

void AudioManager::fadeMusicTrack(MusicTrack track, float targetVolume, float duration) {
    auto it = musicTracks.find(track);
    if (it != musicTracks.end()) {
        float currentVolume = musicVolumes[track];  // Use tracked volume
        fadeTargets[track] = targetVolume;
        fadeSpeeds[track] = (targetVolume - currentVolume) / duration;
    }
}

void AudioManager::stopAllMusic() {
    for (auto& pair : musicTracks) {
        StopMusicStream(pair.second);
    }
    fadeTargets.clear();
    fadeSpeeds.clear();
}

void AudioManager::setMasterVolume(float volume) {
    settings.masterVolume = std::max(0.0f, std::min(1.0f, volume));
    // Update all currently playing audio
    updateVolumes();
}

void AudioManager::setSFXVolume(float volume) {
    settings.sfxVolume = std::max(0.0f, std::min(1.0f, volume));
    updateVolumes();
}

void AudioManager::setMusicVolume(float volume) {
    settings.musicVolume = std::max(0.0f, std::min(1.0f, volume));
    updateVolumes();
}

void AudioManager::setSoundEnabled(bool enabled) {
    settings.soundEnabled = enabled;
    if (!enabled) {
        stopAllSounds();
    }
}

void AudioManager::setMusicEnabled(bool enabled) {
    settings.musicEnabled = enabled;
    if (!enabled) {
        stopAllMusic();
    }
}

void AudioManager::update(float deltaTime) {
    // Update music streams (required for streaming)
    for (auto& pair : musicTracks) {
        UpdateMusicStream(pair.second);
    }

    // Handle music fading
    for (auto it = fadeTargets.begin(); it != fadeTargets.end(); ) {
        auto musicIt = musicTracks.find(it->first);
        if (musicIt != musicTracks.end()) {
            float currentVolume = musicVolumes[it->first];  // Use tracked volume
            float newVolume = currentVolume + (fadeSpeeds[it->first] * deltaTime);

            // Check if we've reached the target
            if ((fadeSpeeds[it->first] > 0 && newVolume >= it->second) ||
                (fadeSpeeds[it->first] < 0 && newVolume <= it->second)) {
                newVolume = it->second;
                musicVolumes[it->first] = newVolume;  // Update tracked volume
                SetMusicVolume(musicIt->second, calculateEffectiveVolume(newVolume));
                it = fadeTargets.erase(it);
            } else {
                musicVolumes[it->first] = newVolume;  // Update tracked volume
                SetMusicVolume(musicIt->second, calculateEffectiveVolume(newVolume));
                ++it;
            }
        } else {
            it = fadeTargets.erase(it);
        }
    }
}

void AudioManager::updateVolumes() {
    // Update all currently playing sounds
    for (auto& pair : soundEffects) {
        float effectiveVolume = calculateEffectiveVolume(settings.sfxVolume);
        SetSoundVolume(pair.second, effectiveVolume);
    }

    // Update all music tracks and track their volumes
    for (auto& pair : musicTracks) {
        float effectiveVolume = calculateEffectiveVolume(settings.musicVolume);
        SetMusicVolume(pair.second, effectiveVolume);
        musicVolumes[pair.first] = effectiveVolume;  // Update tracked volume
    }
}

bool AudioManager::isSoundLoaded(SoundEffect effect) {
    return soundEffects.find(effect) != soundEffects.end();
}

bool AudioManager::isMusicLoaded(MusicTrack track) {
    return musicTracks.find(track) != musicTracks.end();
}

float AudioManager::calculateEffectiveVolume(float baseVolume) const {
    return baseVolume * settings.masterVolume;
}

// ============================================================================
// GLOBAL FUNCTIONS FOR EASY ACCESS
// ============================================================================

void playClickSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_CLICK);
}

void playHoverSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_HOVER);
}

void playOpenSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_OPEN);
}

void playCloseSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_CLOSE);
}

void playSuccessSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_SUCCESS);
}

void playErrorSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_ERROR);
}

void playNotificationSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_NOTIFICATION);
}

void playItemPickupSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_ITEM_PICKUP);
}

void playInventoryFullSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_INVENTORY_FULL);
}

void playQuestUpdateSound() {
    AudioManager::getInstance().playSoundEffect(SoundEffect::UI_QUEST_UPDATE);
}

void playMenuMusic() {
    AudioManager::getInstance().playMusicTrack(MusicTrack::MENU_AMBIENT);
}

void playGameMusic() {
    AudioManager::getInstance().playMusicTrack(MusicTrack::GAME_AMBIENT);
}

void playCombatMusic() {
    AudioManager::getInstance().playMusicTrack(MusicTrack::COMBAT);
}

void playVictoryMusic() {
    AudioManager::getInstance().playMusicTrack(MusicTrack::VICTORY);
}

void playDefeatMusic() {
    AudioManager::getInstance().playMusicTrack(MusicTrack::DEFEAT);
}

void setUIVolume(float volume) {
    AudioManager::getInstance().setSFXVolume(volume);
}

void setMusicVolume(float volume) {
    AudioManager::getInstance().setMusicVolume(volume);
}

void enableSound(bool enabled) {
    AudioManager::getInstance().setSoundEnabled(enabled);
}

void enableMusic(bool enabled) {
    AudioManager::getInstance().setMusicEnabled(enabled);
}

void initializeUIAudio() {
    AudioManager::getInstance().initialize();
}

void shutdownUIAudio() {
    AudioManager::getInstance().shutdown();
}

} // namespace UIAudio
