#ifndef SCIL_USER_HINTS_H
#define SCIL_USER_HINTS_H

// ############################################################################
// ## scil_performance_unit
// ############################################################################

enum scil_performance_unit {
    SCIL_PERFORMANCE_IGNORE = 0,
    SCIL_PERFORMANCE_MIB,
    SCIL_PERFORMANCE_GIB,
    SCIL_PERFORMANCE_NETWORK, // this unit indicates the performance of the
                              // network interconnect, e.g., Infiniband
    SCIL_PERFORMANCE_NODELOCAL_STORAGE, // the performance of the local storage
    SCIL_PERFORMANCE_SINGLESTREAM_SHARED_STORAGE // this unit indicates the
                                                 // performance of one thread
                                                 // sending data to the shared
                                                 // storage, e.g., 1 GiB/s with
                                                 // Lustre
};

extern const char * performance_units[];

/**
 * \brief Structure, describing the required performance.
 * It consists of a base unit and a multiplier, the result is "multiplier * unit".
 */
typedef struct
{
    enum scil_performance_unit unit;
    float multiplier;
} scilPr_performance_hint_t;

// ############################################################################
// ## scil_user_hints_t
// ############################################################################

// These values define that the particular metrics is not of interest
#define SCIL_ACCURACY_DBL_IGNORE 0.0
#define SCIL_ACCURACY_INT_IGNORE 0

// These values define the limit for the accuracy
#define SCIL_ACCURACY_DBL_FINEST 1e-307
#define SCIL_ACCURACY_INT_FINEST -1

/**
 * \brief Struct containing information on the tolerable
 * precision loss on compression
 */
typedef struct
{
    /** \brief relative tolerable error (1 means 1%) */
    double relative_tolerance_percent;

    /**
     * \brief With a relative tolerance small numbers may be problematic, e.g.
     * 1% for 0.01 becomes 0.01 +- 0.0001
     * the finest tolerance limits the smallest relative error
     * e.g. when compressing the value 0.01 with a finest absolute tolerance of
     * 0.01 it becomes 0.01 +- 0.01
     * So this is the lower bound of the resolution and guaranteed error for
     * relative errors,
     * where as the absolute tolerance is the guaranteed resolution for all data
     * points.
     */
    double relative_err_finest_abs_tolerance;

    /** \brief absolute tolerable error (e.g. 1 means the value 2 can become
     * 1-3) */
    double absolute_tolerance;

    /** \brief Number of significant digits in decimal */
    int significant_digits;

    /** \brief Alternative to the decimal digits */
    int significant_bits;

    /** Define the value up to which we shall compress lossless*/
    double lossless_data_range_up_to;

    /** Define the value above which we shall compress lossless*/
    double lossless_data_range_from;

    /** a single value to be compressed lossless */
    double fill_value;

    /** \brief Settings for a multi dimensional field
     * The max steepness indicates the delta between two neighboring points that is considered to be tolerable for normal patterns.
     */
    double field_max_steepness;

    /** Describes the performance requirements for the compressors */
    scilPr_performance_hint_t comp_speed;
    scilPr_performance_hint_t decomp_speed;

    /** \brief */
    char *force_compression_methods;

} scil_user_hints_t;

void scilPr_initialize_user_hints(scil_user_hints_t * hints);

void scilPr_copy_user_hints(scil_user_hints_t * out, const scil_user_hints_t* hints);

void scilPr_print_user_hints(const scil_user_hints_t* hints);

#endif // SCIL_USER_HINTS_H
