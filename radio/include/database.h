#ifndef DATABASE_H
#define DATABASE_H

#include <mysql/mysql.h>
#include <string>

#include "config.h"

class Database
{
public:
    Database();
    ~Database();

    bool connect(const Config& config);

    bool insertRecording(
        const std::string& stationName,
        const std::string& startTime,
        const std::string& endTime,
        const std::string& transcription,
        const std::string& filePath,
        int durationSeconds
    );

    void disconnect();

private:
    MYSQL* conn;
};

#endif