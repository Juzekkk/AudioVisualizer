#include "INIFileParser.h"

INIFileParser::INIFileParser() = default;

INIFileParser::INIFileParser(const std::string &iniFilePath)
{
    load(iniFilePath);
}

INIFileParser::~INIFileParser() = default;

bool INIFileParser::load(const std::string &iniFilePath)
{
    std::ifstream inputFile(iniFilePath);
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open " << iniFilePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(inputFile, line))
    {
        size_t separatorPosition = line.find('=');
        if (separatorPosition != std::string::npos)
        {
            std::string key = line.substr(0, separatorPosition);
            std::string value = line.substr(separatorPosition + 1);
            settingsMap[key] = value;
        }
    }

    inputFile.close();
    return true;
}

bool INIFileParser::save(const std::string &iniFilePath)
{
    std::ofstream outputFile(iniFilePath);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open " << iniFilePath << std::endl;
        return false;
    }

    for (const auto &setting : settingsMap)
    {
        outputFile << setting.first << "=" << setting.second << std::endl;
    }

    outputFile.close();
    return true;
}