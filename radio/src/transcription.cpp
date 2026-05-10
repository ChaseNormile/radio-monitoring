#include "transcription.h"

#include <curl/curl.h>
#include <iostream>

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

TranscriptionClient::TranscriptionClient(const Config& config)
{
    transcriptionUrl = config.getString("transcriptionHost", "http://127.0.0.1:6000");

    if (!transcriptionUrl.empty() && transcriptionUrl.back() == '/')
    {
        transcriptionUrl.pop_back();
    }

    transcriptionUrl += "/transcribe";
    std::cout << "Transcription URL: " << transcriptionUrl << std::endl;
}

std::string TranscriptionClient::transcribeFile(const std::string& filePath)
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        std::cerr << "Failed to initialize curl" << std::endl;
        return "";
    }

    std::string response;

    curl_mime* form = curl_mime_init(curl);
    curl_mimepart* field = curl_mime_addpart(form);

    curl_mime_name(field, "audio");
    curl_mime_filedata(field, filePath.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, transcriptionUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        std::cerr << "Transcription request failed: "
                  << curl_easy_strerror(res)
                  << std::endl;

        curl_mime_free(form);
        curl_easy_cleanup(curl);

        return "";
    }

    curl_mime_free(form);
    curl_easy_cleanup(curl);

    return extractTranscription(response);
}

std::string TranscriptionClient::extractTranscription(const std::string& jsonResponse)
{
    std::string key = "\"transcription\"";
    size_t keyPos = jsonResponse.find(key);

    if (keyPos == std::string::npos)
    {
        std::cerr << "Could not find transcription in response: "
                  << jsonResponse
                  << std::endl;
        return "";
    }

    size_t colonPos = jsonResponse.find(":", keyPos);
    size_t firstQuote = jsonResponse.find("\"", colonPos + 1);
    size_t secondQuote = jsonResponse.find("\"", firstQuote + 1);

    if (colonPos == std::string::npos ||
        firstQuote == std::string::npos ||
        secondQuote == std::string::npos)
    {
        std::cerr << "Failed to parse transcription JSON: "
                  << jsonResponse
                  << std::endl;
        return "";
    }

    return jsonResponse.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}