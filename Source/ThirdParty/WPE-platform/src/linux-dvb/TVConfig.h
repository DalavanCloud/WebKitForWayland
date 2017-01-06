#ifndef TV_CONFIG_H_
#define TV_CONFIG_H_

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <libdvbapi/dvbfe.h>
#include <linux/dvb/frontend.h>
#include <wpe/tvcontrol-backend.h>

using namespace std;

#define TUNER_ID_LEN 3
#define TV_CONFIG_FILE  "TVConfig.txt"

struct dvbfe_handle {
    int fd;
    enum dvbfe_type type;
    char *name;
};

inline struct dvbfe_handle* openFE(std::string& tunerId) {
/* Change id to int*/
    int adapter = stoi(tunerId.substr(0, tunerId.find(":")));
    int frontend = stoi(tunerId.substr(tunerId.find(":")+1));
/* Open dvbFe*/
    return dvbfe_open(adapter, frontend, 0);
}

typedef enum {
    AF, AX, AL, DZ, AS, AD, AO, AI, AQ, AG, AR, AM, AW, AU, AT, AZ,
    BS, BH, BD, BB, BY, BE, BZ, BJ, BM, BT, BO, BQ, BA, BW, BV, BR,
    IO, BN, BG, BF, BI, KH, CM, CA, CV, KY, CF, TD, CL, CN, CX, CC,
    CO, KM, CG, CD, CK, CR, CI, HR, CU, CW, CY, CZ, DK, DJ, DM, DO,
    EC, EG, SV, GQ, ER, EE, ET, FK, FO, FJ, FI, FR, GF, PF, TF, GA,
    GM, GE, DE, GH, GI, GR, GL, GD, GP, GU, GT, GG, GN, GW, GY, HT,
    HM, VA, HN, HK, HU, IS, IN, ID, IR, IQ, IM, IE, IL, IT, JM, JP,
    JE, JO, KZ, KE, KI, KP, KR, KW, KG, LA, LV, LB, LS, LR, LY, LI,
    LT, LU, MO, MK, MG, MW, MY, MV, ML, MT, MH, MQ, MR, MU, YT, MX,
    FM, MD, MC, MN, ME, MS, MA, MZ, MM, NA, NR, NP, NL, NC, NZ, NI,
    NE, NG, NU, NF, MP, NO, OM, PK, PW, PS, PA, PG, PY, PE, PH, PN,
    PL, PT, PR, QA, RE, RO, RU, RW, BL, SH, KN, LC, MF, PM, VC, WS,
    SM, ST, SA, SN, RS, SC, SL, SX, SG, SK, SI, SB, SO, ZA, GS, ES,
    LK, SD, SR, SJ, SZ, SE, CH, SY, TW, TJ, TZ, TH, TL, TG, TK, TO,
    TT, TN, TR, TM, TC, TV, UG, UA, AE, GB, US, UM, UY, UZ, VU, VE,
    VN, VG, VI, WF, EH, YE, ZM, ZW
} Country;
      
typedef enum  {
    ATSC_VSB                = 1,
    ATSC_QAM                = 2,
    DVBT_AU                 = 3,
    DVBT_DE                 = 4,
    DVBT_FR                 = 5,
    DVBT_GB                 = 6,
    DVBC_QAM                = 7,
    DVBC_FI                 = 8,
    DVBC_FR                 = 9,
    DVBC_BR                 = 10,
    ISDBT_6MHZ              = 11,
    USERLIST                = 999
} ChannelList;

/******************************************************************************
 * table ids as defined by standards.
 *
 *****************************************************************************/

enum table_id {
    TABLE_PAT                               = 0x00, // program_association_section
    TABLE_CAT                               = 0x01, // conditional_access_section
    TABLE_PMT                               = 0x02, // program_map_section
    TABLE_TSDT                              = 0x03, // transport_stream_description_section
    TABLE_NIT_ACT                           = 0x40, // network_information_section - actual_network
    TABLE_NIT_OTH                           = 0x41, // network_information_section - other_network
    TABLE_SDT_ACT                           = 0x42, // service_description_section - actual_transport_stream
    TABLE_SDT_OTH                           = 0x46, // service_description_section - other_transport_stream
    TABLE_BAT                               = 0x4A, // bouquet_association_section
    TABLE_EIT_ACT                           = 0x4E, // event_information_section - actual_transport_stream, present/following
    TABLE_EIT_OTH                           = 0x4F, // event_information_section - other_transport_stream, present/following
    TABLE_EIT_SCHEDULE_ACT_50               = 0x50, // 0x50 to 0x5F event_information_section - actual_transport_stream, schedule
    TABLE_EIT_SCHEDULE_ACT_5F               = 0x5F, //
    TABLE_EIT_SCHEDULE_OTH_60               = 0x60, // 0x60 to 0x6F event_information_section - other_transport_stream, schedule
    TABLE_EIT_SCHEDULE_OTH_6F               = 0x6F, //
    TABLE_TDT                               = 0x70, // time_date_section
    TABLE_RST                               = 0x71, // running_status_section
    TABLE_STUFFING                          = 0x72, // stuffing_section
    TABLE_TOT                               = 0x73, // time_offset_section
    TABLE_AIT                               = 0x74, // application information section (TS 102 812 [17])
    TABLE_CST                               = 0x75, // container section (TS 102 323 [15])
    TABLE_RCT                               = 0x76, // related content section (TS 102 323 [15])
    TABLE_CIT                               = 0x77, // content identifier section (TS 102 323 [15])
    TABLE_MPE_FEC                           = 0x78, //
    TABLE_RNS                               = 0x79, // resolution notification section (TS 102 323 [15])
    TABLE_DIT                               = 0x7E, // discontinuity_information_section
    TABLE_SIT                               = 0x7F, // selection_information_section
    TABLE_PREMIERE_CIT                      = 0xA0, // premiere content information section
    TABLE_VCT_TERR                          = 0xC8, // ATSC VCT VSB (terr)
    TABLE_VCT_CABLE                         = 0xC9, // ATSC VCT QAM (cable)
};

/******************************************************************************
 * PIDs as defined for accessing tables.
 *
 *****************************************************************************/
enum pid_type {
    PID_PAT                                 = 0x0000,
    PID_CAT                                 = 0x0001,
    PID_TSDT                                = 0x0002,
    PID_NIT_ST                              = 0x0010,
    PID_SDT_BAT_ST                          = 0x0011,
    PID_EIT_ST_CIT                          = 0x0012,
    PID_RST_ST                              = 0x0013,
    PID_TDT_TOT_ST                          = 0x0014,
    PID_RNT                                 = 0x0016,
    PID_DIT                                 = 0x001E,
    PID_SIT                                 = 0x001F,
    PID_VCT                                 = 0x1FFB,
};

enum atsc_descriptor_tag {
    dtag_atsc_stuffing                      = 0x80,
    dtag_atsc_ac3_audio                     = 0x81,
    dtag_atsc_caption_service               = 0x86,
    dtag_atsc_content_advisory              = 0x87,
    dtag_atsc_extended_channel_name         = 0xa0,
    dtag_atsc_service_location              = 0xa1,
    dtag_atsc_time_shifted_service          = 0xa2,
    dtag_atsc_component_name                = 0xa3,
    dtag_atsc_dcc_departing_request         = 0xa8,
    dtag_atsc_dcc_arriving_request          = 0xa9,
    dtag_atsc_redistribution_control        = 0xaa,
    dtag_atsc_private_information           = 0xad,
    dtag_atsc_content_identifier            = 0xb6,
    dtag_atsc_genre                         = 0xab,
};


#endif //TV_CONFIG_H_
