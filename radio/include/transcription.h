#ifndef TRANSCRIPTION_H
#define TRANSCRIPTION_H

#include <string>
#include "config.h"

class TranscriptionClient
{
public:
    explicit TranscriptionClient(const Config& config);

    std::string transcribeFile(const std::string& filePath);

private:
    std::string transcriptionUrl;

    std::string extractTranscription(const std::string& jsonResponse);
};

#endif