#pragma once

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

class INIFileParser
{
public:
    INIFileParser();
    explicit INIFileParser(const std::string &iniFilePath);
    ~INIFileParser();

    bool load(const std::string &iniFilePath);
    bool save(const std::string &iniFilePath);

    template <typename T>
    T getSetting(const std::string &key) const
    {
        auto it = settingsMap.find(key);
        if (it != settingsMap.end())
        {
            T value;
            std::istringstream iss(it->second);
            iss >> value;
            return value;
        }
        return T();
    }

    template <typename T>
    void setSetting(const std::string &key, const T &value)
    {
        std::ostringstream oss;
        oss << value;
        settingsMap[key] = oss.str();
    }

private:
    std::map<std::string, std::string> settingsMap;
};
