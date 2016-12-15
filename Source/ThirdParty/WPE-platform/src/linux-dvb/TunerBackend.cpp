#include "TunerBackend.h"
#include <inttypes.h>


namespace BCMRPi {

TvTunerBackend::TvTunerBackend(struct dvbfe_handle* feHandle, int tunerCnt)
    : m_feHandle(feHandle)
    , m_srcTypeListPtr(NULL)
    , m_supportedSysCount(0) {

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    printf("FE handle  priv = %s , param = %s \n", m_feHandle->name, feHandle->name);

    getTunerInfo();
    initializeSourceList();
    setModulation(tunerCnt);
}

TvTunerBackend::~TvTunerBackend() {
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    free(m_feHandle->name);
    m_feHandle->name = nullptr;

    free(m_feHandle);
    m_feHandle = nullptr;

    m_supportedSysCount = 0;
}

void TvTunerBackend::initializeSourceList() {

    getAvailableSrcList(&m_srcList);
}

void TvTunerBackend::getTunerInfo()
{
    printf("Info: using DVB adapter auto detection.\n");

    /* determine FE type and caps */
    if (ioctl(m_feHandle->fd, FE_GET_INFO, &m_feInfo) == -1) {
        printf("   ERROR: unable to determine frontend type\n");
    }
    printf("\n%s - - -> ", m_feInfo.name);
    getCapabilities();
}

void TvTunerBackend::getCapabilities() {
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    dvbfe_get_info(m_feHandle, dvbfe_info_mask(FE_STATUS_PARAMS), &fe_info, DVBFE_INFO_QUERYTYPE_IMMEDIATE, 0);
    if (m_feInfo.caps & FE_CAN_INVERSION_AUTO) {
        printf("INVERSION_AUTO\n");
    }
    else {
        printf("INVERSION_AUTO not supported, trying INVERSION_OFF.\n");
    }
    if (m_feInfo.caps & FE_CAN_8VSB) {
        printf("8VSB\n");
    }
    if (m_feInfo.caps & FE_CAN_16VSB) {
        printf("16VSB\n");
    }
    if (m_feInfo.caps & FE_CAN_QAM_64) {
        printf("QAM_64\n");
    }
    if (m_feInfo.caps & FE_CAN_QAM_256) {
        printf("QAM_256\n");
    }
}

void TvTunerBackend::setModulation(int tunerCnt) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    string modulation, data, tunerStr;
    char tmpStr[20];
    printf("Getting Modulation for Tuner:%d", tunerCnt);
    fstream fObj;
    fObj.open(TV_CONFIG_FILE, ios::in);
    snprintf(tmpStr, 20, "TUNER_%d_MODULATION", tunerCnt + 1);
    tunerStr = string(tmpStr);
    while (!fObj.eof()) {
        fObj >> data;
        if (!data.find(tunerStr)) {
            fObj.seekp(3, ios::cur);
            fObj >> modulation;
            if (!modulation.find("8VSB") && (m_feHandle->type ==  DVBFE_TYPE_ATSC) && (m_feInfo.caps & FE_CAN_8VSB)) {
                fe_info.feparams.u.atsc.modulation = DVBFE_ATSC_MOD_VSB_8;
                 m_channel = ATSC_VSB;
                cout << "\nModulation set to 8VSB";
                populateFreq(m_channel);
            } else {
                cout << "Modulation set ERROR";
            }
            break;
        }
    }
    fObj.close();
}

void TvTunerBackend::populateFreq(ChannelList channel) {
    long populatedFrequencies[150];

    for (int channel = 0; channel < 68 ;channel++)
    {
        if (baseOffset(channel+2, m_channel) != -1)
            populatedFrequencies[channel] = baseOffset( channel+2, m_channel) + ((channel+2) * freqStep(channel+2, m_channel));
    }
    for (int channel = 0; channel < 68; channel++)
    {
        cout <<"\n" <<populatedFrequencies[channel] <<"\n";
    }
}

void TvTunerBackend::getSignalStrength(double* signalStrength) {
    printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
    ioctl(m_feHandle->fd, FE_READ_SIGNAL_STRENGTH, fe_info.signal_strength);
    *signalStrength = static_cast<double>(10 *(log10(fe_info.signal_strength)) + 30);
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

void TvTunerBackend::getSupportedSrcTypeList(wpe_tvcontrol_src_types_vector* out_source_types_list) {
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);

    /* Get supported source list from platform*/
    int  ret = 0;
    ret  =  getSupportedSourcesTypeList(out_source_types_list);
    if (ret < 0)
        printf("Failed to get supported source list \n");
}

void TvTunerBackend::getAvailableSrcList(wpe_tvcontrol_src_types_vector* out_source_list) {
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    /* Get avaiable source list from platform*/
    int  ret = 0;
    ret  =  getSupportedSourcesTypeList(out_source_list);
    if (ret < 0)
        printf("Failed to get supported source list \n");
    /* Create private list of sources  */
    getSources();
}

void TvTunerBackend::getSources() {

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    int i;

    if (m_srcTypeListPtr && (m_supportedSysCount != 0) && m_sourceList.empty()) {
        /* Read supported type list from the private list
                       and create list of source objects */
        for (i = 0; i < m_supportedSysCount; i++) {
            SourceBackend* sInfo = (SourceBackend* )new SourceBackend(m_srcTypeListPtr[i], m_feHandle);
            m_sourceList.push_back(sInfo);
            printf("\n%s:%s:%d\n", __FILE__, __func__, __LINE__);
        }
    } else {
        printf("Private source list already created \n");
    }
}

int TvTunerBackend::getSupportedSourcesTypeList(wpe_tvcontrol_src_types_vector* out_source_types_list) {

    int i = 0;
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);

    printf("Number of supported Source = \n ");
    printf("%" PRIu64 "\n",  m_supportedSysCount);
    if (!m_srcTypeListPtr && (m_supportedSysCount == 0)) {
        printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);

        struct dtv_property p = {.cmd = DTV_ENUM_DELSYS };
        struct dtv_properties cmdName = {.num = 1, .props = &p};

        printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
        if (ioctl(m_feHandle->fd, FE_GET_PROPERTY, &cmdName) == -1) {
            printf("FE_GET_PROPERTY failed \n");
            return -1;
        }

        m_supportedSysCount = cmdName.props->u.buffer.len;
        printf("Number of supported Source = ");
        printf("%" PRIu64 "\n",  m_supportedSysCount);


        /*Create an array of  Type */
        m_srcTypeListPtr = (SourceType *)new SourceType[cmdName.props->u.buffer.len];
        printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);

        for (i = 0; i < m_supportedSysCount; i++) {
            /*Map the list  to W3C spec */
            switch (cmdName.props->u.buffer.data[i]) {
                case SYS_DVBC_ANNEX_A:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    //TODO m_srcTypeListPtr[i] = ;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DVBC_ANNEX_B:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] =  DvbC; //TODO
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DVBT:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = DvbT;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DSS:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    //TODO m_srcTypeListPtr[i] = ;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DVBS:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = DvbS;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DVBS2:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = DvbS2;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DVBH:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = DvbH;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_ISDBT:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = IsdbT;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_ISDBS:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = IsdbS;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_ISDBC:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = IsdbC;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_ATSC:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = Atsc;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_ATSCMH:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = AtscMH;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DTMB:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = Dtmb;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_CMMB:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = Cmmb;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DAB:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    //TODO m_srcTypeListPtr[i] = ;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DVBT2:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = DvbT2;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_TURBO:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    //TODO m_srcTypeListPtr[i] = ;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_DVBC_ANNEX_C:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    //TODO m_srcTypeListPtr[i] = ;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                case SYS_UNDEFINED:
                    printf("STP: CASE = %d \t", cmdName.props->u.buffer.data[i]);
                    m_srcTypeListPtr[i] = Undifined;
                    //printf("ST: %s \n", p_delivery_system_name[ cmdName.props->u.buffer.data[i]]);
                    break;
                default:
                    printf("ST: DEFAULT  \n");
                    m_srcTypeListPtr[i] = Undifined;
                    break;
            } //switch
        } // Loop
        if (m_supportedSysCount == 0) {
            printf("driver returned 0 supported delivery source type!");
            return -1;
        }
    } //List already created

    /*Update the number of supported Sources*/
    out_source_types_list->length = m_supportedSysCount;

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    /* update source type  ptr */
    out_source_types_list->types = m_srcTypeListPtr;
    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);

    return m_supportedSysCount;
}

void TvTunerBackend::startScanning() {

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    /* Get source corresponds to this type  */
     SourceBackend *source;
     getSource(m_sType, &source);
     source->startScanning();
}

void TvTunerBackend::getSource(SourceType sType, SourceBackend **source) {

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    /* Iterate list and get the source matching the list*/
    for(auto& src : m_sourceList) {
        printf("SRC type from list =  %d from param = %d", src->srcType(), sType);
        if(src->srcType() == sType) {
            *source = src;
        }
    }
}

void TvTunerBackend::stopScanning() {

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    /* Get source corresponds to this type  */
    SourceBackend *source;
    getSource(m_sType, &source);
    source->stopScanning();
}

void TvTunerBackend::getChannelList(SourceType sType, struct wpe_tvcontrol_channel_vector* channelVector) {

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    /* Get source corresponds to this type  */
    SourceBackend *source;
    getSource(sType, &source);
    source->getChannelList(channelVector);
}

void TvTunerBackend::setCurrentChannel(SourceType sType ,uint64_t channelNumber) {

    printf("%s:%s:%d \n", __FILE__, __func__, __LINE__);
    /* Get source corresponds to this type  */
    SourceBackend *source;
    getSource(sType, &source);
    source->setCurrentChannel(channelNumber);
}

} // namespace BCMRPi
