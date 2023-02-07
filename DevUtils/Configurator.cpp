//
// Created by  on 4/13/2020.
//
//

//Application headers
#include "Configurator.h"

Configurator *Configurator::instance = nullptr;
std::once_flag Configurator::initInstanceFlag;
