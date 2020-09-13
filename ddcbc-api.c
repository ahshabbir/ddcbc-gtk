// Wrapper api to control monitor brightness via the ddcutil API  

#include <stdio.h>
#include <stdlib.h>

#include "ddcutil_c_api.h"
#include "ddcutil_status_codes.h"

#define BRIGHTNESS_FEATURE_CODE 0x10

/* ddcbc_bundle is a record of several ddca types necessary for this program.
 * Information about these types can be found in the ddcutil api.
 * underscored types are private and should'nt be used outside of this file. */
typedef struct 
ddcbc_bundle {
    DDCA_Display_Info_List *dlist;
    DDCA_Display_Handle *_dhs;
    bool *supported;
    unsigned int ct; // exposed for 'getAll' 'setAll' commands
    unsigned int supported_count;
} ddcbc_bundle;

/* in_bounds checks if dispno is valid. */
bool 
in_bounds(ddcbc_bundle *bun, unsigned int dispno) 
{ 
    return dispno >= 1 && dispno <= bun->ct; 
}

/* ddchc_bundle_init gets the display information and the display handlers 
 * for all displays in the user's system. It also checks each display for the
 * existence of the brightness feature. It is the responsibility of the caller
 * to call 'ddcdb_bundle_free' to deallocate the struct returned. */
ddcbc_bundle 
ddcbc_bundle_init() 
{
    ddcbc_bundle bun;
    ddca_get_display_info_list2(false, &bun.dlist);
    bun.ct = bun.dlist->ct;
    bun._dhs = malloc(bun.ct * sizeof(DDCA_Display_Handle));
    bun.supported = malloc(bun.ct * sizeof(DDCA_Display_Handle));
    bun.supported_count = 0;

    // Open all displays and check for the brightness feature:
    for (unsigned int i = 0; i < bun.ct; i++) {
        DDCA_Display_Handle dh = NULL;
        
        DDCA_Status ddcrc = ddca_open_display2(bun.dlist->info[i].dref, false, &dh);
        bool support = false;
        // Warn of unopened display, but do not fail, in case others are of use.
        if (ddcrc != 0) {
            fprintf(stderr, "Cannot open display %u", i + 1);
            support = false;
        } else {
            bun._dhs[i] = dh;
            support = true;
        }
        // Check if we can set the brightness on displayno 'i':
        DDCA_Feature_Metadata* info;
        ddcrc = ddca_get_feature_metadata_by_dh
            (BRIGHTNESS_FEATURE_CODE, dh, false, &info);

        bun.supported[i] = (ddcrc == 0) && support;
        if (bun.supported[i])
            bun.supported_count++;
    }

    return bun;
}

// ddcbc_bundle_free frees the memory space of the bundle.
void 
ddcbc_bundle_free(ddcbc_bundle *bun) 
{
    ddca_free_display_info_list(bun->dlist);
    // Close all displays:
    for (unsigned int i = 0; i < bun->ct; i++) {
        DDCA_Status ddcrc = ddca_close_display(bun->_dhs[i]);
        if (ddcrc != 0)
            fprintf(stderr, "Couldn't close display %u", i + 1);
    }
    free(bun->supported);
}

// brightness_stats is the record of the return type when getting brightness.
typedef struct brightness_stats {
    uint16_t max_val;
    uint16_t cur_val;
} brightness_stats;

/* ddcbc_bundle_get_brightness retrives the current and maximum brightness
 * values for a given monitor. */
brightness_stats 
ddcbc_bundle_get_brightness(ddcbc_bundle *bun, unsigned int dispno) 
{

    brightness_stats bs;
    bs.max_val = 0;
    bs.cur_val = 0;
    
    if (!in_bounds(bun, dispno)) {
        fprintf(stderr, "Bad monitor number %u\n", dispno);
        return bs;
    } else if (!bun->supported[dispno - 1]) {
        fprintf(stderr, "Monitor %d not supported\n", dispno);
        return bs;
    }

    DDCA_Non_Table_Vcp_Value valrec;
    DDCA_Status ddcrc = ddca_get_non_table_vcp_value(bun->_dhs[dispno - 1],
        BRIGHTNESS_FEATURE_CODE, &valrec);

    if (ddcrc != 0) {
        fprintf(stderr, 
            "Failed to get current brightness for display number: %d\n", 
            dispno);
        return bs;
    }

    bs.max_val = valrec.mh << 8 | valrec.ml;
    bs.cur_val = valrec.sh << 8 | valrec.sl;

    return bs;
}

// ddcbc_bundle_set_brightness sets the brightness of a given monitor.
void 
ddcbc_bundle_set_brightness(ddcbc_bundle *bun, unsigned int dispno, 
    uint16_t new_val) 
{

    brightness_stats bs = ddcbc_bundle_get_brightness(bun, dispno);
    if (new_val > bs.max_val) {
        return;
    }

    uint8_t new_val_high = new_val >> 8;  
    uint8_t new_val_low = new_val & 0xff;

    DDCA_Status ddcrc = ddca_set_non_table_vcp_value(bun->_dhs[dispno - 1], 
        BRIGHTNESS_FEATURE_CODE, new_val_high, new_val_low);

    if (ddcrc != 0) {
        fprintf(stderr, "Couldn't set brightness on monitor %u", dispno - 1);
        return;
    }

    ddca_enable_verify(true);
}