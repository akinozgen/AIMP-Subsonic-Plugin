#include "PlaybackTracker.h"

#include <algorithm>

#include "Diagnostics.h"
#include "SubsonicRepository.h"

PlaybackTracker::PlaybackTracker(SubsonicRepository* repository)
    : repository_(repository) {
}

void PlaybackTracker::SetRepository(SubsonicRepository* repository) {
    repository_ = repository;
    Reset();
}

void PlaybackTracker::Reset() {
    songId_.clear();
    durationSeconds_ = 0.0;
    nowPlayingSent_ = false;
    submissionSent_ = false;
}

void PlaybackTracker::HandleStreamStarted(const std::wstring& songId, double durationSeconds, double positionSeconds) {
    if (songId.empty()) {
        Reset();
        return;
    }

    if (songId != songId_) {
        songId_ = songId;
        durationSeconds_ = durationSeconds;
        nowPlayingSent_ = false;
        submissionSent_ = false;
        LogInfo(L"Scrobble state started. SongId=" + songId +
            L", Duration=" + std::to_wstring(durationSeconds) +
            L", Position=" + std::to_wstring(positionSeconds));
    } else if (durationSeconds > 0.0) {
        durationSeconds_ = durationSeconds;
    }

    if (!nowPlayingSent_) {
        SendScrobble(songId, false);
        nowPlayingSent_ = true;
    }
}

void PlaybackTracker::HandleStreamEnded() {
    if (!songId_.empty() && !submissionSent_) {
        SendScrobble(songId_, true);
        submissionSent_ = true;
        LogInfo(L"Scrobble submitted on stream end. SongId=" + songId_);
    }
}

void PlaybackTracker::HandlePlaybackPositionUpdate(const std::wstring& songId, double durationSeconds, double positionSeconds) {
    if (songId_.empty() || submissionSent_) {
        return;
    }
    if (songId.empty() || songId != songId_) {
        return;
    }
    if (durationSeconds > 0.0) {
        durationSeconds_ = durationSeconds;
    }

    const double threshold = Threshold(durationSeconds_);
    if (positionSeconds >= threshold) {
        SendScrobble(songId_, true);
        submissionSent_ = true;
        LogInfo(L"Scrobble submitted by threshold. SongId=" + songId_ +
            L", Position=" + std::to_wstring(positionSeconds) +
            L", Threshold=" + std::to_wstring(threshold));
    }
}

double PlaybackTracker::Threshold(double durationSeconds) const {
    if (durationSeconds > 0.0) {
        return std::min(240.0, std::max(30.0, durationSeconds * 0.5));
    }
    return 30.0;
}

void PlaybackTracker::SendScrobble(const std::wstring& songId, bool submission) {
    if (!repository_ || songId.empty()) {
        return;
    }
    const bool ok = repository_->Scrobble(songId, submission);
    if (!ok) {
        LogInfo(L"Scrobble failed. SongId=" + songId +
            L", Submission=" + std::to_wstring(submission ? 1 : 0));
    }
}
