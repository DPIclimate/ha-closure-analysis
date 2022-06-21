#ifndef HA_CLOSURE_ANALYSIS_MAIN_H
#define HA_CLOSURE_ANALYSIS_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <assert.h>
#include <unistd.h>
#include <log.h>

#include "BOM/historical_weather.h"
#include "BOM/stations.h"
#include "Ubidots/devices.h"
#include "WillyWeather/authenticate.h"
#include "WillyWeather/tide.h"
#include "WillyWeather/precipitation.h"
#include "IBM_EIS/authenticate.h"
#include "IBM_EIS/timeseries.h"
#include "FoodAuthority/harvest_area.h"
#include "FoodAuthority/harvest_areas.h"
#include "severity_index.h"
#include "http.h"
#include "ftp.h"

#endif // HA_CLOSURE_ANALYSIS_MAIN_H
