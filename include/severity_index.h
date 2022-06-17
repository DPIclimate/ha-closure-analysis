#ifndef HA_CLOSURE_ANALYSIS_SEVERITY_INDEX_H
#define HA_CLOSURE_ANALYSIS_SEVERITY_INDEX_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "utils.h"

#define SI_MAX_NUM_DAYS 5000

/// Handles severity index errors
typedef enum{
    SI_MIN_DAYS_ERROR = -2,
    SI_ALLOCATION_EXCEEDED = -1,
    SI_OK = 0,
} SI_Handle_TypeDef;

/// Handles severity index dataset
typedef struct{
    time_t timestamps[SI_MAX_NUM_DAYS];
    double flooding_severity_index[SI_MAX_NUM_DAYS];
    double heat_severity_index[SI_MAX_NUM_DAYS];
} SI_Dataset_TypeDef;

int8_t SI_CalculateFloodRisk(const double* tide_diff,
                             const double* daily_precipitation,
                             const time_t* timestamps,
                             uint16_t n_days);

#endif //HA_CLOSURE_ANALYSIS_SEVERITY_INDEX_H
