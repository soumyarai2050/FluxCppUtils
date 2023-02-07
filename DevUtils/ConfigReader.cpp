//
// Created by  on 4/26/2020.
//

#include "ConfigReader.h"
#include "NanoLog.h"

using namespace std;

ConfigReader *ConfigReader::psConfigReaderInstance = nullptr;
std::mutex ConfigReader::sConfigReaderMutex;

ConfigReader::ConfigReader() {
    m_ConfigSettingMap.clear();
}

ConfigReader::~ConfigReader() {
    m_ConfigSettingMap.clear();
}

ConfigReader *ConfigReader::getInstance() {
    // No need to use double re-check lock mechanism here
    // because this getInstance() will be called at the time of
    // initialization - not latency sensitive code
    std::lock_guard <std::mutex> guard(ConfigReader::sConfigReaderMutex);
    if (nullptr == psConfigReaderInstance) {
        psConfigReaderInstance = new ConfigReader;
    }
    return psConfigReaderInstance;
}

bool ConfigReader::getValue(const std::string &tag, int &value) {
    map<string, string>::iterator it;
    it = m_ConfigSettingMap.find(tag);
    if (it != m_ConfigSettingMap.end()) {
        value = static_cast<int>(strtol((it->second).c_str(), nullptr, 10));
        return true;
    }
    return false;
}

bool ConfigReader::getValue(const std::string &tag, std::string &value) {
    map<string, string>::iterator it;
    it = m_ConfigSettingMap.find(tag);
    if (it != m_ConfigSettingMap.end()) {
        value = it->second;
        return true;
    }
    return false;
}

bool ConfigReader::parseFile(const string &fileName) {
    ifstream inputFile;
    inputFile.open(fileName.c_str());
    string delimiter = "=";
    int initPos = 0;

    if (inputFile.fail()) {
        LOG_WARN << "Unable to find or open Config file: " << fileName;
        return false;
    }

    string line;
    while (getline(inputFile, line)) {
        // Remove comment Lines
        size_t found = line.find_first_of('#');
        string configData = line.substr(0, found);

        // Remove ^M from configData
        configData.erase(std::remove(configData.begin(), configData.end(), '\r'), configData.end());

        if (configData.empty())
            continue;

        size_t length = configData.find(delimiter);

        string tag, value;

        if (length != string::npos) {
            tag = configData.substr(initPos, length);
            value = configData.substr(length + 1);
        }

        // Trim white spaces
        tag = reduce(tag);
        value = reduce(value);

        if (tag.empty()) {
            if (value.empty()) {
                //Log error and exit - we don't want silent errors in config file
                LOG_CRIT << "Tag without value found in config file for tag: "
                          << tag
                          << "If user need to keep this in the file for any reason - they MUST comment the line out";
                exit(1);
            } else {
                //This is an empty line - ignore
                continue;
            }
        }

        // Check if any of the tags is repeated more than one times
        // it needs to pick the latest one instead of the old one.

        // Search, if the tag is already present or not
        // If it is already present, then delete an existing one

        auto itr = m_ConfigSettingMap.find(tag);
        if (itr != m_ConfigSettingMap.end()) {
            //Log the duplicate config as error and exit to avoid silent ignore
            LOG_CRIT << "Duplicate entry found in config file for tag: "
                      << itr->first << " first found value: " << itr->second
                      << " second found value: " << value;
            exit(1);
            //This line is redundant - we can enhance the configurator to support repeat by
            // enabling this line and making using a multimap instead of map
            //m_ConfigSettingMap.erase(tag);
        }

        m_ConfigSettingMap.insert(std::pair<string, string>(tag, value));
    }
    return true;
}

//static method impl
std::string ConfigReader::trim(const std::string &str, const std::string &whitespace) {
    size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return "";

    size_t strEnd = str.find_last_not_of(whitespace);
    size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

//static method impl
std::string ConfigReader::reduce(const std::string &str,
                                 const std::string &fill,
                                 const std::string &whitespace) {
    // trim first
    string result = trim(str, whitespace);

    // replace sub ranges
    size_t beginSpace = result.find_first_of(whitespace);
    while (beginSpace != std::string::npos) {
        size_t endSpace = result.find_first_not_of(whitespace, beginSpace);
        size_t range = endSpace - beginSpace;

        result.replace(beginSpace, range, fill);

        size_t newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

void ConfigReader::dumpFileValues() {
    map<string, string>::iterator it;
    for (it = m_ConfigSettingMap.begin(); it != m_ConfigSettingMap.end(); ++it) {
        LOG_INFO << it->first << " = " << it->second;
    }
}
