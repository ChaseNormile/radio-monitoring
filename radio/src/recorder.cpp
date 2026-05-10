#include "recorder.h"

#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <sstream>

Recorder::Recorder(const Config& config)
{
    recordingPath = config.getString("recordingPath", "../clips/");
    durationSeconds = config.getInt("recordingDuration", 10);

    std::filesystem::create_directories(recordingPath);
}

RecordingResult Recorder::recordStation(const Station& station)
{
    RecordingResult result;
    result.success = false;
    result.durationSeconds = durationSeconds;

    result.startTime = getCurrentDateTime();

    std::string safeStationName = sanitizeFilename(station.name);
    std::string timestamp = getFileTimestamp();

    result.filePath = recordingPath + safeStationName + "_" + timestamp + ".wav";

    std::stringstream command;

    command << "ffmpeg -y "
            << "-t " << durationSeconds << " "
            << "-i \"" << station.url << "\" "
            << "-ar 16000 "
            << "-ac 1 "
            << "\"" << result.filePath << "\" "
            << "2> \"" << recordingPath << safeStationName << "_" << timestamp << "_ffmpeg.log\"";

    std::cout << "Recording station " << station.name << std::endl;
    std::cout << command.str() << std::endl;

    int status = std::system(command.str().c_str());

    result.endTime = getCurrentDateTime();

    if (status != 0)
    {
        std::cerr << "FFmpeg failed for station: " << station.name << std::endl;
        return result;
    }

    result.success = true;

    std::cout << "Recording saved: " << result.filePath << std::endl;

    return result;
}

std::string Recorder::getCurrentDateTime()
{
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);

    return std::string(buffer);
}

std::string Recorder::getFileTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", localTime);

    return std::string(buffer);
}

std::string Recorder::sanitizeFilename(const std::string& name)
{
    std::string clean = name;

    for (char& c : clean)
    {
        if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*'
            || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
        {
            c = '_';
        }
    }

    return clean;
}