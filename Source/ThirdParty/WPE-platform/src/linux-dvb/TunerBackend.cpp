#include "TunerBackend.h"

namespace BCMRPi {

TvTunerBackend::TvTunerBackend(struct dvbfe_handle* feHandle) {
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    m_feHandle.fd = feHandle->fd;
    m_feHandle.type = feHandle->type;
    m_feHandle.name = strdup(feHandle->name);

    getTunerInfo();
}

TvTunerBackend::~TvTunerBackend() {
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    free( m_feHandle.name);
    m_feHandle.name = nullptr;
}

void TvTunerBackend::getTunerInfo()
{
    printf("Info: using DVB adapter auto detection.\n");

    /* determine FE type and caps */
    if (ioctl(m_feHandle.fd, FE_GET_INFO, &m_feInfo) == -1) {
        printf("   ERROR: unable to determine frontend type\n");
    }
    printf("\n%s - - -> ", m_feInfo.name);
    getModulation();
}

void TvTunerBackend::getModulation() {
    int choice = 1;
    dvbfe_get_info(&m_feHandle, dvbfe_info_mask(FE_STATUS_PARAMS), &fe_info, DVBFE_INFO_QUERYTYPE_IMMEDIATE, 0);
    // printf("\n %d \n",feHandle->type);
    if (m_feInfo.caps & FE_CAN_INVERSION_AUTO) {
        printf("INVERSION_AUTO\n");
    }
    else {
        printf("INVERSION_AUTO not supported, trying INVERSION_OFF.\n");
    }
    if (m_feInfo.caps & FE_CAN_8VSB) {
        printf("8VSB\n");
        m_channel = ATSC_VSB;
    }
    if (m_feInfo.caps & FE_CAN_16VSB) {
        printf("16VSB\n");
        m_channel = ATSC_VSB;
    }
    if (m_feInfo.caps & FE_CAN_QAM_64) {
        printf("QAM_64\n");
        m_channel = ATSC_QAM;
    }
    if (m_feInfo.caps & FE_CAN_QAM_256) {
        printf("QAM_256\n");
        m_channel = ATSC_QAM;
    }
    cout<<"\nselected modulation\n"<<"1)ATSC_VSB \n2)ATSC_QAM";
    //TODO: Read value from config file before setting
    if (choice == 1)
        m_channel = ATSC_VSB;
    else
        m_channel = ATSC_QAM;

    populateFreq(m_channel);
}

void TvTunerBackend::populateFreq(ChannelList channel)
{
    long populatedFrequencies[150];

    for (int channel = 0; channel < 68 ;channel++)
    {
        if (baseOffset(channel+2, m_channel) != -1)
            populatedFrequencies[channel] = baseOffset( channel+2, m_channel) + ((channel+2) * freqStep(channel+2, m_channel));
    }
    for (int channel = 0; channel < 68; channel++)
    {
        cout <<populatedFrequencies[channel] <<"\n";
    }
}

/*
 * return the base offsets for specified channellist and channel.
 */
int TvTunerBackend::baseOffset(int channel, int channelList) {
    switch (channelList) {
        case ATSC_QAM: //ATSC cable, US EIA/NCTA Std Cable center freqs + IRC list
        case DVBC_BR:  //BRAZIL - same range as ATSC IRCi
            switch (channel) {
                case  2 ...  4:   return   45000000;
                case  5 ...  6:   return   49000000;
                case  7 ... 13:   return  135000000;
                case 14 ... 22:   return   39000000;
                case 23 ... 94:   return   81000000;
                case 95 ... 99:   return -477000000;
                case 100 ... 133: return   51000000;
                default:          return   -1;
            }
        case ATSC_VSB: //ATSC terrestrial, US NTSC center freqs
            switch (channel) {
                case  2 ...  4: return   45000000;
                case  5 ...  6: return   49000000;
                case  7 ... 13: return  135000000;
                case 14 ... 69: return  389000000;
                default:        return  -1;
            }
        case ISDBT_6MHZ: // ISDB-T, 6 MHz central frequencies
            switch (channel) {
                // Channels 7-13 are reserved but aren't used yet
                //case  7 ... 13: return  135000000;
                case 14 ... 69: return  389000000;
                default:        return  -1;
            }
        case DVBT_AU:  //AUSTRALIA, 7MHz step list
            switch (channel) {
               case  5 ... 12: return  142500000;
               case 21 ... 69: return  333500000;
               default:        return  -1;
            }
        case DVBT_DE:  //GERMANY
        case DVBT_FR:  //FRANCE, +/- offset 166kHz & +offset 332kHz & +offset 498kHz
        case DVBT_GB:  //UNITED KINGDOM, +/- offset
            switch (channel) {
                case  5 ... 12: return  142500000; // VHF unused in FRANCE, skip those in offset loop
                case 21 ... 69: return  306000000;
                default:        return  -1;
            }

        case DVBC_QAM: //EUROPE
            switch (channel) {
                case  0 ... 1:
                case  5 ... 12: return   73000000;
                case 22 ... 90: return  138000000;
                default:        return  -1;
            }
        case DVBC_FI:  //FINLAND, QAM128
            switch (channel) {
                case  1 ... 90: return  138000000;
                default:        return  -1;
            }
        case DVBC_FR:  //FRANCE, needs user response.
            switch (channel) {
                case  1 ... 39: return  107000000;
                case 40 ... 89: return  138000000;
                default:        return  -1;
            }
        default:
                return -1;
    }
}

/*
 * return the freq step size for specified channellist
 */
int TvTunerBackend::freqStep(int channel, int channelList) {
    switch (channelList) {
        case ATSC_QAM:
        case ATSC_VSB:
        case DVBC_BR:
        case ISDBT_6MHZ:
                       return  6000000; // atsc, 6MHz step
        case DVBT_AU:  return  7000000; // dvb-t australia, 7MHz step
        case DVBT_DE:
        case DVBT_FR:
        case DVBT_GB:  switch (channel) { // dvb-t europe, 7MHz VHF ch5..12, all other 8MHz
                              case  5 ... 12:    return 7000000;                              case 21 ... 69:    return 8000000;
                              default:           return 8000000; // should be never reached.
                              }
        case DVBC_QAM:
        case DVBC_FI:
        case DVBC_FR:  return  8000000; // dvb-c, 8MHz step
        default:
             return 0;
    }
}

} // namespace BCMRPi
