#include "config.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

bool Config::load(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return false;
    }

    std::string line;

    while (std::getline(file, line))
    {
        line = trim(line);

        if (line.empty())
        {
            continue;
        }

        if (line[0] == '#' || line.rfind("--", 0) == 0)
        {
            continue;
        }

        size_t equalPos = line.find('=');

        if (equalPos == std::string::npos)
        {
            continue;
        }

        std::string key = trim(line.substr(0, equalPos));
        std::string value = trim(line.substr(equalPos + 1));

        values[key] = value;
    }

    return true;
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) const
{
    auto it = values.find(key);

    if (it == values.end())
    {
        return defaultValue;
    }

    return it->second;
}

int Config::getInt(const std::string& key, int defaultValue) const
{
    auto it = values.find(key);

    if (it == values.end())
    {
        return defaultValue;
    }

    try
    {
        return std::stoi(it->second);
    }
    catch (...)
    {
        return defaultValue;
    }
}

bool Config::getBool(const std::string& key, bool defaultValue) const
{
    auto it = values.find(key);

    if (it == values.end())
    {
        return defaultValue;
    }

    std::string value = it->second;

    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return value == "1" || value == "true" || value == "yes";
}

std::vector<Station> Config::getStations() const
{
    std::vector<Station> stations;

    int stationCount = getInt("stationCount", 0);

    for (int i = 1; i <= stationCount; i++)
    {
        std::string nameKey = "station" + std::to_string(i) + "Name";
        std::string urlKey = "station" + std::to_string(i) + "URL";

        std::string name = getString(nameKey);
        std::string url = getString(urlKey);

        if (!name.empty() && !url.empty())
        {
            stations.push_back({name, url});
        }
    }

    return stations;
}

std::string Config::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");

    if (start == std::string::npos)
    {
        return "";
    }

    size_t end = str.find_last_not_of(" \t\r\n");

    return str.substr(start, end - start + 1);
}