# Changelog

## 1.0.0 - Initial public release

- Added a Subsonic/Navidrome music library storage for AIMP with Artists, Albums, Favorites, Tracks, and Playlists sections.
- Added DirectURL playback through Subsonic `/rest/stream.view` with configurable stream format and maximum bitrate.
- Added a persistent metadata index for tracks, artists, albums, playlists, starred tracks, and ordered album/playlist snapshots.
- Added a background `Build / Refresh Metadata Index` command using AIMP's thread service.
- Added a file-info provider and playlist metadata repair so direct stream URLs can recover title, artist, album, duration, rating, and Subsonic identity from the metadata cache.
- Added cover art loading and local artwork cache via Subsonic `getCoverArt`.
- Added Subsonic now-playing and played scrobble events.
- Added quick search support for central Music Library table rows, with `search3` fallback for global track searches.
- Added AIMP Options settings for server URL, username, password, stream format, bitrate, library page size, debug logging, and self-signed TLS mode.
- Added Windows DPAPI-protected password storage for AIMP settings, with `subsonic.local.json` kept only as an optional development fallback.
- Added diagnostic redaction for Subsonic auth query values and common secret fields before writing logs.
- Kept audio offline cache and server-side playlist write/delete operations out of the stable `main` release line.
