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
 * If selected_filename is NULL or empty, scans BGM_SD_DIRECTORY for .mp3 files and, if
 * any are found, plays through them as a playlist (looping back to the first after the
 * last). If selected_filename is set, plays only that one file from BGM_SD_DIRECTORY,
 * looping it indefinitely, ignoring any other files present in the folder.
 *
 * Safe to call if no matching track is found (playback simply does not start) or if
 * playback is already active (no-op -- call bgm_stop() first to change tracks/mode).
 *
 * @param storage_prefix Storage prefix, e.g. "sd:".
 * @param selected_filename Filename (not full path) of a single track to loop, or NULL/"".
 */
void bgm_start(const char *storage_prefix, const char *selected_filename);

/**
 * @brief Stop background music playback and release its resources.
 */
void bgm_stop(void);

/**
 * @brief Advance background music playback. Call once per frame.
 *
 * Drives MP3 decoding. In playlist mode, automatically advances to the next track when
 * the current one finishes (looping back to the first after the last). In single-track
 * mode, replays the same track when it finishes. Does nothing if background music is
 * not active.
 */
void bgm_process(void);

/**
 * @brief Check whether background music is currently active.
 *
 * @return true if a playlist was loaded and playback is active, false otherwise.
 */
bool bgm_is_active(void);

/**
 * @brief Scan BGM_SD_DIRECTORY for .mp3 files and return their filenames.
 *
 * Allocates an array of strdup'd filenames (not full paths). The caller owns the
 * returned array and must free it with bgm_free_track_names().
 *
 * @param storage_prefix Storage prefix, e.g. "sd:".
 * @param out_names Set to a newly allocated array of filenames (NULL if none found).
 * @return Number of filenames found (0 if the folder is missing or empty).
 */
int bgm_scan_track_names(const char *storage_prefix, char ***out_names);

/**
 * @brief Free an array returned by bgm_scan_track_names().
 *
 * @param names Array to free (safe to call with NULL).
 * @param count Number of entries in the array.
 */
void bgm_free_track_names(char **names, int count);

#endif /* BGM_H__ */
