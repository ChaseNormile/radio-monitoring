#include "database.h"

#include <iostream>

Database::Database()
{
    conn = nullptr;
}

Database::~Database()
{
    disconnect();
}

bool Database::connect(const Config& config)
{
    conn = mysql_init(nullptr);

    if (conn == nullptr)
    {
        std::cerr << "mysql_init failed" << std::endl;
        return false;
    }

    std::string host = config.getString("dbHost", "127.0.0.1");
    std::string user = config.getString("dbUser", "root");
    std::string pass = config.getString("dbPass", "");
    std::string dbName = config.getString("dbName", "radio_monitor");
    int port = config.getInt("dbPort", 3306);

    if (mysql_real_connect(
            conn,
            host.c_str(),
            user.c_str(),
            pass.c_str(),
            dbName.c_str(),
            port,
            nullptr,
            0) == nullptr)
    {
        std::cerr << "MySQL connection failed: "
                  << mysql_error(conn)
                  << std::endl;

        mysql_close(conn);
        conn = nullptr;

        return false;
    }

    std::cout << "Connected to MySQL database: " << dbName << std::endl;

    return true;
}

bool Database::insertRecording(
    const std::string& stationName,
    const std::string& startTime,
    const std::string& endTime,
    const std::string& transcription,
    const std::string& filePath,
    int durationSeconds)
{
    if (conn == nullptr)
    {
        std::cerr << "Database is not connected" << std::endl;
        return false;
    }

    std::string query =
        "INSERT INTO recordings "
        "(station_name, start_time, end_time, transcription, file_path, duration_seconds) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    MYSQL_STMT* stmt = mysql_stmt_init(conn);

    if (stmt == nullptr)
    {
        std::cerr << "mysql_stmt_init failed" << std::endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0)
    {
        std::cerr << "mysql_stmt_prepare failed: "
                  << mysql_stmt_error(stmt)
                  << std::endl;

        mysql_stmt_close(stmt);
        return false;
    }

    MYSQL_BIND bind[6]{};

    unsigned long stationNameLength = stationName.length();
    unsigned long startTimeLength = startTime.length();
    unsigned long endTimeLength = endTime.length();
    unsigned long transcriptionLength = transcription.length();
    unsigned long filePathLength = filePath.length();

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)stationName.c_str();
    bind[0].buffer_length = stationNameLength;
    bind[0].length = &stationNameLength;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)startTime.c_str();
    bind[1].buffer_length = startTimeLength;
    bind[1].length = &startTimeLength;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)endTime.c_str();
    bind[2].buffer_length = endTimeLength;
    bind[2].length = &endTimeLength;

    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void*)transcription.c_str();
    bind[3].buffer_length = transcriptionLength;
    bind[3].length = &transcriptionLength;

    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void*)filePath.c_str();
    bind[4].buffer_length = filePathLength;
    bind[4].length = &filePathLength;

    bind[5].buffer_type = MYSQL_TYPE_LONG;
    bind[5].buffer = (void*)&durationSeconds;

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "mysql_stmt_bind_param failed: "
                  << mysql_stmt_error(stmt)
                  << std::endl;

        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "mysql_stmt_execute failed: "
                  << mysql_stmt_error(stmt)
                  << std::endl;

        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);

    std::cout << "Inserted recording for station: "
              << stationName
              << std::endl;

    return true;
}

void Database::disconnect()
{
    if (conn != nullptr)
    {
        mysql_close(conn);
        conn = nullptr;
    }
}