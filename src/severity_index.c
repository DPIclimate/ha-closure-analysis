#include "severity_index.h"


int8_t SI_CalculateFoodRisk(SI_Dataset_TypeDef *dataset,
                            const double *tide_diff,
                            const double *daily_precipitation,
                            const time_t *timestamps,
                            uint16_t n_days) {

    // Need atleast 15 values for computation
    if (n_days < 15) return SI_MIN_DAYS_ERROR;

    // Only have a set amount of allocated memory
    if (n_days > SI_MAX_NUM_DAYS) return SI_ALLOCATION_EXCEEDED;

    double scaled_min = 0;
    double scaled_max = 0;
    uint16_t offset = 4;
    uint16_t index = offset;
    for (; index < (n_days - 9); index++) {

        double sum_tide = 0;
        double sum_precip = 0;
        for (uint16_t x = index - 4; x < (index + 9); x++) {
            sum_tide += tide_diff[x];
            sum_precip += daily_precipitation[x];
        }

        // Prevent zero division error
        if (sum_precip == 0 || sum_tide == 0) continue;

        double scaled_precip_tide = sum_precip / sum_tide;

        // Set maximum value
        if (scaled_max < scaled_precip_tide) {
            scaled_max = scaled_precip_tide;
        }

        // Set minimum value (requires initialisation)
        if (scaled_min > scaled_precip_tide || index == offset) {
            scaled_min = scaled_precip_tide;
        }

        dataset->flooding_severity_index[index - offset] = scaled_precip_tide;
    }

    // Normalise
    for (uint16_t p = 0; p < ((index - offset) - 10); p++) {
        double max_sv = dataset->flooding_severity_index[p];
        dataset->flooding_severity_index[p] =
                (max_sv - scaled_min) / (scaled_max - scaled_min);
        dataset->timestamps[p] = timestamps[p];
    }

    return SI_OK;
}
