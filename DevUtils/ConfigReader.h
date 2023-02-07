//
// Created by  on 4/26/2020.
//

#ifndef TACTICALTRADING_CONFIG_READER_H
#define TACTICALTRADING_CONFIG_READER_H

//System headers
#include <fstream>
#include <algorithm>
#include <mutex>
#include <map>

//https://cppsecrets.com -- C++ CONFIG READER -- https://cppsecrets.com/users/41129711010797106994610011511264103109971051084699111109/C00-Config-Reader.php

// Define namespace for this class. so that anyone
// can easily integrate it. The class name is very generic
// so the chance to collapse the class name is high.
// So need to define the class inside the namespace.

// Defining ConfigReader as singleton class
// Easy to access across the multiple classes
//
// The responsibility of this class is to parse the
// Config file and store it in the std::map
// Defined getter function getValue() to get the
// data from the std::map.
//
// To use this class, pass the config file path to
// the function getInstance()
//
// This is one of the advance config reader, because this
// class is handling the comment line as well. Comment line
// will start from hash(#). So all the string after
// semicolon will be discarded.
//
// NOTE: NO NEED TO MAKE THIS CLASS THREAD-SAFE. IT IS EXTRA OVERHEAD.
// BECAUSE MOSTLY WE ARE DOING ONLY READ OPERATION. WRITE OPERATION IS
// HAPPENING ONLY ONE TIME, WHICH IS IN THE FUNCTION parse(). SO CALL
// parse() FUNCTION AT THE TIME OF INITIALIZATION ONLY.
//
// IF YOUR CONFIGURATION FILE IS UPDATING AT THE RUN TIME AND YOU NEED
// UPDATED DATA FROM THE CONFIGURATION FILE AT RUN TIME, THEN YOU NEED
// TO MAKE THIS CLASS THREAD-SAFE.


class ConfigReader {
private:

    // Define the map to store data from the config file
    std::map <std::string, std::string> m_ConfigSettingMap;

    // Static pointer instance to make this class singleton.
    static ConfigReader *psConfigReaderInstance;
    //mutex to protect psConfigReaderInstance initialization
    static std::mutex sConfigReaderMutex;

public:

    // Public static method getInstance(). This function is
    // responsible for object creation.
    static ConfigReader *getInstance();

    // Parse the config file.
    bool parseFile(const std::string &fileName = "/tmp/default_config");

    // Overloaded getValue() function.
    // Value of the tag in configuration file could be
    // string or integer. So the caller need to take care this.
    // Caller need to call appropriate function based on the
    // data type of the value of the tag.

    bool getValue(const std::string &tag, int &value);

    bool getValue(const std::string &tag, std::string &value);

    // Function dumpFileValues is for only debug purpose
    void dumpFileValues();

private:

    // Define constructor in the private section to make this
    // class as singleton.
    ConfigReader();

    // Define destructor in private section, so no one can delete
    // the instance of this class.
    ~ConfigReader();

    // Define copy constructor in the private section, so that no one can
    // violate the singleton policy of this class
    ConfigReader(const ConfigReader &obj) {}

    // Define assignment operator in the private section, so that no one can
    // violate the singleton policy of this class
    void operator=(const ConfigReader &obj) {}

    // Helper function to trim un-necessary spaces in the tag and value
    static std::string trim(const std::string &str, const std::string &whitespace = " \t");

    static std::string reduce(const std::string &str,
                       const std::string &fill = " ",
                       const std::string &whitespace = " \t");
};

#include <fstream>
#include <chrono>
#include <iostream>
#include <future>
#include <mutex>
#include <thread>
#include<tuple>
#include<map>
#include<vector>
#include<string>
#include<stdexcept>
#include<algorithm>
#include <Contract.h>
#include "../TT_Engine/TT_IBConnectParams.h"

#endif //TACTICALTRADING_CONFIG_READER_H
