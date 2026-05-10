#ifndef RECORDER_H
#define RECORDER_H

#include <string>

#include "config.h"

struct RecordingResult
{
    bool success;
    std::string filePath;
    std::string startTime;
    std::string endTime;
    int durationSeconds;
};

class Recorder
{
public:
    explicit Recorder(const Config& config);

    RecordingResult recordStation(const Station& station);

private:
    std::string recordingPath;
    int durationSeconds;

    std::string getCurrentDateTime();
    std::string getFileTimestamp();
    std::string sanitizeFilename(const std::string& name);
};

#endif