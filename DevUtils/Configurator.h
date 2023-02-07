//
// Created by  on 4/13/2020.
//

#ifndef TACTICALTRADING_CONFIGURATOR_H
#define TACTICALTRADING_CONFIGURATOR_H

//System
#include <fstream>
#include <chrono>
#include <iostream>
#include <future>
#include <mutex>
#include <thread>

#include<tuple>
#include<fstream>
#include<map>
#include<vector>
#include<string>
#include<stdexcept>
#include<algorithm>
#include<iostream>
#include <map>

//App
#include <Contract.h>
#include "../TT_Engine/TT_IBConnectParams.h"
#include "../TT_Engine/TT_CommonAlgoParams.h"
#include "ConfigReader.h"
#include "NanoLog.h" 

class Configurator {
public:
    static Configurator& getInstance() {
        std::call_once(initInstanceFlag, &Configurator::initSingleton);
        return *instance;
    }

    bool get_TT_IBConnectParams_FromConfig(TT_IBConnectParams &rTT_IBConnectParams) {
        if (!mInitDone) {
            LOG_CRIT << "Configurator get_TT_IBConnectParams_FromConfig called without init!!";
            return false;
        }//else continue , all good

        pConfigReader->getValue("host", rTT_IBConnectParams.host); // "ayra.me"
        pConfigReader->getValue("port", rTT_IBConnectParams.port); // 8099
        pConfigReader->getValue("clientId", rTT_IBConnectParams.clientId); // 1
        pConfigReader->getValue("reconnectIntervalInSeconds", rTT_IBConnectParams.reconnectIntervalInSeconds); // 3
        pConfigReader->getValue("reconnectMaxAttempts", rTT_IBConnectParams.reconnectMaxAttempts); // 3

        LOG_INFO << "IB Connection : " << rTT_IBConnectParams.host << " : " << rTT_IBConnectParams.port
                 << " : " << rTT_IBConnectParams.clientId << " : " << rTT_IBConnectParams.reconnectIntervalInSeconds
                 << " : " << rTT_IBConnectParams.reconnectMaxAttempts;

        return true;
    }

    ///Stocks (STK) and Futures (FUT) are only supported for now
    ///Index are not directly traded, it's either index futures (FUT) or index ETF's (STK)
    ///Besides FUT and STK , we have: BOND, CFD, EFP, CASH, FUND, FOP, OPT, WAR
    ///Selectively enable more as / when we decide to trade any other product
    ///TODO SKY : Enable FOREX ?
    bool get_Contract_FromConfig(Contract &rContract) {
        if (!mInitDone) {
            LOG_CRIT << "Configurator get_Contract_FromConfig called without init!!";
            return false;
        }//else continue , all good
        pConfigReader->getValue("symbol", rContract.symbol); // "CL" / "MSFT"
        pConfigReader->getValue("secType", rContract.secType); // "FUT" / "STK"
        pConfigReader->getValue("exchange", rContract.exchange); // "NYMEX" / "NYSE"
        pConfigReader->getValue("currency", rContract.currency); // "USD" / "USD"
        if (rContract.secType == "FUT") {
            pConfigReader->getValue("lastTradeDateOrContractMonth",
                                    rContract.lastTradeDateOrContractMonth); // "202005"; //or "20200421" */
        } else if (rContract.secType == "STK" || rContract.secType == "CFD" || rContract.secType == "CASH") {
            //Stock details are covered in above basics, no specific stock only field required
            //TODO SKY : Test Stock , Index ETF and Index Future confirm all good - save sample contracts
        } else {
            // throw for now.
            LOG_CRIT << "NON supported security type: " << rContract.secType;
            throw std::runtime_error("Only FUT supported for now");
        }
        LOG_INFO << "get_Contract_FromConfig:- symbol: " << rContract.symbol << " ; secType: " << rContract.secType
                 << " ; exchange: " << rContract.exchange << " ; currency: " << rContract.currency
                 << " ; lastTradeDateOrContractMonth: "
                 << (rContract.secType == "FUT") ? rContract.lastTradeDateOrContractMonth
                                                 : "Not Applicable for non future contracts";
        return true;
    }

    [[maybe_unused]] bool get_upcomingExpiryDates_FromConfig(std::string &rUpcomingExpiryDates) {
        if (!mInitDone) {
            LOG_CRIT << "Configurator get_upcomingExpiryDates_FromConfig called without init!!";
            return false;
        }//else continue , all good
        //std::vector<std::string> rUpcomingExpiryDates;
        // Unused : Where to use ?

        pConfigReader->getValue("rUpcomingExpiryDates", rUpcomingExpiryDates);
        LOG_INFO << "rUpcomingExpiryDates:";
        for (auto dt : rUpcomingExpiryDates) LOG_INFO << " " << dt;

        //TODO SKY Day-2: Auto-rollover support will need this logic
        // set the rContract.lastTradeDateOrContractMonth based on required logic using one of upcomingExpiryDate
        // required_expiry ==> >curr_date &&  <next_upcoming_expiry in vector
        // also consider how many days before actual expiry the rContract should be switched.

        return true;
    }

    bool get_account_FromConfig(std::string &rAccount) {
	if (!mInitDone) {
           LOG_CRIT << "Configurator get_account_FromConfig called without init!!";
           return false;
        }
        pConfigReader->getValue("account", rAccount);
        LOG_INFO << "rAccount:"<<rAccount;
        return true;
    }

    bool get_TT_CommonAlgoParams_FromConfig(TT_CommonAlgoParams &rTT_CommonAlgoParams) {
        if (!mInitDone) {
            LOG_CRIT << "Configurator get_TT_CommonAlgoParams_FromConfig called without init!!";
            return false;
        }//else continue , all good

        pConfigReader->getValue("takeProfitTicks", rTT_CommonAlgoParams.takeProfitTicks);
        pConfigReader->getValue("stopLossTicks", rTT_CommonAlgoParams.stopLossTicks); // // Default is 10 ticks

        LOG_INFO << "IB Algo Common Params takeProfitTicks: " << rTT_CommonAlgoParams.takeProfitTicks
                 << " stopLossTicks: " << rTT_CommonAlgoParams.stopLossTicks;

        return true;
    }

    void init(const std::string &configFileName_) {
        configFileName = configFileName_;
        pConfigReader = ConfigReader::getInstance();
        pConfigReader->parseFile(configFileName);
        pConfigReader->dumpFileValues();
        mInitDone = true;
    }

    Configurator(const Configurator &) = delete;
    Configurator &operator=(const Configurator &) = delete;

private:
    static Configurator *instance;
    static std::once_flag initInstanceFlag;

    static void initSingleton() {
        instance = new Configurator;
    }

    Configurator() = default;
    ~Configurator() = default;

    std::string configFileName;
    ConfigReader *pConfigReader= nullptr;
    std::atomic<bool> mInitDone = false;
};

//Configurator* Configurator::instance= nullptr;
//std::once_flag Configurator::initInstanceFlag;
#endif //TACTICALTRADING_CONFIGURATOR_H
