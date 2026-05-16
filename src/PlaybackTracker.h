#pragma once

#include <string>

class SubsonicRepository;

class PlaybackTracker {
public:
    explicit PlaybackTracker(SubsonicRepository* repository = nullptr);

    void SetRepository(SubsonicRepository* repository);
    void Reset();
    void HandleStreamStarted(const std::wstring& songId, double durationSeconds, double positionSeconds);
    void HandleStreamEnded();
    void HandlePlaybackPositionUpdate(const std::wstring& songId, double durationSeconds, double positionSeconds);

private:
    double Threshold(double durationSeconds) const;
    void SendScrobble(const std::wstring& songId, bool submission);

    SubsonicRepository* repository_{nullptr};
    std::wstring songId_;
    double durationSeconds_{0.0};
    bool nowPlayingSent_{false};
    bool submissionSent_{false};
};
