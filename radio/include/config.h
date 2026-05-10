#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>

struct Station
{
    std::string name;
    std::string url;
};

class Config
{
public:
    bool load(const std::string& filename);

    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;

    std::vector<Station> getStations() const;

private:
    std::unordered_map<std::string, std::string> values;

    static std::string trim(const std::string& str);
};

#endif