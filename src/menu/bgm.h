/**
 * @file bgm.h
 * @brief Background Music
 * @ingroup menu
 */

#ifndef BGM_H__
#define BGM_H__

#include <stdbool.h>

/**
 * @brief SD card directory (relative to storage_prefix) scanned for background music tracks.
 */
#define BGM_SD_DIRECTORY "menu/n64ever/bgm"

/**
 * @brief Start background music playback.
 *
 * Scans BGM_SD_DIRECTORY on the given storage device for .mp3 files and, if any are
 * found, begins playing one on SOUND_MP3_PLAYER_CHANNEL. Safe to call if no tracks are
 * present (playback simply does not start) or if playback is already active (no-op).
 *
 * @param storage_prefix Storage prefix, e.g. "sd:".
 */
void bgm_start(const char *storage_prefix);

/**
 * @brief Stop background music playback and release its resources.
 */
void bgm_stop(void);

/**
 * @brief Advance background music playback. Call once per frame.
 *
 * Drives MP3 decoding and, when the current track finishes, automatically advances to
 * the next track in the playlist (looping back to the first after the last). Does
 * nothing if background music is not active.
 */
void bgm_process(void);

/**
 * @brief Check whether background music is currently active.
 *
 * @return true if a playlist was loaded and playback is active, false otherwise.
 */
bool bgm_is_active(void);

#endif /* BGM_H__ */
