#include "SourceBackend.h"
#define TV_DEBUG 1

namespace BCMRPi {

SourceBackend::SourceBackend(SourceType type, dvbfe_handle* feHandle)
    : m_sType(type)
    , m_feHandle(feHandle) {
    adapter = stoi(m_feHandle->tunerId.substr(0, m_feHandle->tunerId.find(":")));
    demux = stoi(m_feHandle->tunerId.substr(m_feHandle->tunerId.find(":")+1));
}

void SourceBackend::startScanning() {
    int frequency = 575000000; // TODO: remove hardcoding
    int modulation = DVBFE_ATSC_MOD_VSB_8; //TODO: remove hardcoding
    if (tuneToFrequency(frequency, modulation)) {
       switch(m_sType) {
           case Atsc:
           case AtscMH:
               atscScan(frequency, modulation);
               break;
           case DvbT:
           case DvbT2:
           case DvbC:
           case DvbC2:
           case DvbS:
           case DvbS2:
           case DvbH:
           case DvbSh:
               dvbScan();
               break;
           default:
               printf("Type Not supported!!");
               break;
       }
    }
}

void SourceBackend::processPmt(int pmtFd)
{
    int size;
    uint8_t siBuf[4096];

    // read the section
    if ((size = read(pmtFd, siBuf, sizeof(siBuf))) < 0) {
        return;
    }

    // parse section
    struct section *section = section_codec(siBuf, size);
    if (section == NULL) {
        return;
    }

    // parse section_ext
    struct section_ext *sectionExt = section_ext_decode(section, 0);
    if (sectionExt == NULL) {
        return;
    }

    // parse PMT
    struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(sectionExt);
    if (pmt == NULL) {
        return;
    }

    struct mpeg_pmt_stream *curStream;
    mpeg_pmt_section_streams_for_each(pmt, curStream) {
        printf("stream_type : %x pid : %x \n", curStream->stream_type, curStream->pid);
    }
}

void SourceBackend::stopScanning() {
    /* */
}

void  SourceBackend::setCurrentChannel(uint64_t channelNo) {
    /* */
}

void SourceBackend::getChannels(wpe_tvcontrol_channel_vector* channelVector) {

    /*Populate channel list */
    if(!m_channelList.empty())
      printf(" Populate the channel vector \n");
    else
      printf("Channel list is empty .Scanning is incomplete \n");

}

void SourceBackend::dvbScan(){
    /* */
}

bool SourceBackend::tuneToFrequency(int frequency, int modulation){
    struct dvbfe_info feInfo;
    int timeout = 3;
    dvbfe_get_info(m_feHandle, DVBFE_INFO_FEPARAMS, &feInfo, DVBFE_INFO_QUERYTYPE_IMMEDIATE, 0);
    switch (m_sType) {
        case Atsc:
        case AtscMH:
            if (DVBFE_TYPE_ATSC != feInfo.type) {
                fprintf(stderr, "%s(): only ATSC frontend supported currently\n",
                __FUNCTION__);
                return false;
            }
            switch (modulation) {
                case DVBFE_ATSC_MOD_VSB_8:
                    feInfo.feparams.frequency = frequency;
                    feInfo.feparams.inversion = DVBFE_INVERSION_AUTO;
                    feInfo.feparams.u.atsc.modulation = DVBFE_ATSC_MOD_VSB_8;
                    break;
                default:
                    printf("Modulation not supported!!!\n");
                    return false;
            }
            break;
        default:
            printf("Only Atsc supported!!\n");
            return false;
    }
    fprintf(stdout, "tuning to %d Hz, please wait...\n", frequency);
    if (dvbfe_set(m_feHandle, &feInfo.feparams, timeout * 1000)) {
        fprintf(stderr, "%s(): cannot lock to %d Hz in %d seconds\n",
            __FUNCTION__, frequency, timeout);
        return false;
    }
    fprintf(stdout, "tuner locked.\n");
    return true;
}

void SourceBackend::mpegScan(){
    struct pollfd pollFd;
    int patFd = createSectionFilter(PID_PAT, TABLE_PAT);
    pollFd.fd = patFd;
    pollFd.events = POLLIN|POLLPRI|POLLERR;
    bool flag = true;;
    while (flag){
        int count = poll(&pollFd, 1, 100);
                if (count < 0) {
            fprintf(stderr, "Poll error\n");
            break;
        }
        if (count == 0) {
            continue;
        }
        if (pollFd.revents & (POLLIN|POLLPRI)){
            if (processPAT(pollFd.fd)){
                dvbdemux_stop(pollFd.fd);
                flag = false;
            }
        }
    }
}

void SourceBackend::atscScan(int frequency, int modulation) {
    int vctFd;
    struct pollfd pollFd;
    switch (modulation) {
        case DVBFE_ATSC_MOD_VSB_8:
            vctFd = createSectionFilter(PID_VCT, TABLE_VCT_TERR);
            break;
        default:
            printf("Modulation not supported!!\n");
            return;
    }
    pollFd.fd = vctFd;
    pollFd.events = POLLIN|POLLPRI|POLLERR;
    bool flag = true;;
    while (flag) {
        int count = poll(&pollFd, 1, 100);
                if (count < 0) {
            fprintf(stderr, "Poll error\n");
            break;
        }
        if (count == 0) {
            continue;
        }
        if (pollFd.revents & (POLLIN|POLLPRI)) {
            switch(modulation){
                case DVBFE_ATSC_MOD_VSB_8:
                    if (processTVCT(pollFd.fd, frequency) ) {
                        fprintf(stderr, "%s(): error calling parse_stt()\n",
                        __FUNCTION__);
                    }
                    else {
                        flag = false;
                        dvbdemux_stop(pollFd.fd);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

int SourceBackend::createSectionFilter(uint16_t pid, uint8_t tableId) {
    int demuxFd = -1;
    uint8_t filter[18];
    uint8_t mask[18];

    // open the demuxer
    if ((demuxFd = dvbdemux_open_demux(adapter, demux, 0)) < 0) {
        return -1;
    }

    // create a section filter
    memset(filter, 0, sizeof(filter));
    memset(mask, 0, sizeof(mask));
    filter[0] = tableId;
    mask[0] = 0xFF;
    if (dvbdemux_set_section_filter(demuxFd, pid, filter, mask, 1, 1)) {
        close(demuxFd);
        return -1;
    }

    // done
    return demuxFd;
}

bool SourceBackend::processPAT(int patFd) {
    int size;
    uint8_t siBuf[4096];
    // read the section
    if ((size = read(patFd, siBuf, sizeof(siBuf))) < 0) {
        return false;
    }

    // parse section
    struct section *section = section_codec(siBuf, size);
    if (section == NULL) {
        return false;
    }
    // parse sectionExt
    struct section_ext *sectionExt = section_ext_decode(section, 0);
    if (sectionExt == NULL) {
        return false;
    }
    /*if (pat_version == sectionExt->version_number) { //TODO: recheck again
        return false;
    }*/

    // parse PAT
    struct mpeg_pat_section *pat = mpeg_pat_section_codec(sectionExt);
    if (pat == NULL) {
        return false;
    }

    // try and find the requested program
    struct mpeg_pat_program *cur_program;
    mpeg_pat_section_programs_for_each(pat, cur_program) {
        printf("Program Number:- %d PMT Pid:- %x\n", cur_program->program_number, cur_program->pid);
    }

    // remember the PAT version
    //pat_version = sectionExt->version_number;
    return true;
}

int SourceBackend::processTVCT(int dmxfd, int frequency) {
    int numSections;
    uint32_t sectionPattern;
    struct atsc_tvct_section *tvct;
    struct atsc_tvct_channel *ch;
    int i, k, ret;
    struct section *section;
    struct section_ext *sectionExt;
    struct atsc_section_psip *psip;
    int size;
    unsigned char siBuf[4096];
    sectionPattern = 0;
    numSections = -1;
    do {
         /* read it */
        if ((size = read(dmxfd, siBuf, sizeof(siBuf))) < 0) {
            fprintf(stderr, "%s(): error calling read()\n", __FUNCTION__);
            return -1;
        }

        section = section_codec(siBuf, size);
        if (NULL == section) {
            fprintf(stderr, "%s(): error calling section_codec()\n",
                __FUNCTION__);
            return -1;
        }
        sectionExt = section_ext_decode(section, 0);
        if (NULL == sectionExt) {
            fprintf(stderr, "%s(): error calling section_ext_decode()\n",
                __FUNCTION__);
            return -1;
        }

        psip = atsc_section_psip_decode(sectionExt);
        if (NULL == psip) {
            fprintf(stderr,
                "%s(): error calling atsc_section_psip_decode()\n",
                __FUNCTION__);
            return -1;
        }

        tvct = atsc_tvct_section_codec(psip);
        if (NULL == tvct) {
            fprintf(stderr, "%s(): error decode table section\n",
                __FUNCTION__);
            return -1;
        }

        if (-1 == numSections) {
            numSections = 1 + tvct->head.ext_head.last_section_number;
            if(32 < numSections) {
                fprintf(stderr, "%s(): no support yet for "
                    "tables having more than 32 sections\n",
                    __FUNCTION__);
                return -1;
            }
        } else {
            if (numSections !=
                1 + tvct->head.ext_head.last_section_number) {
                fprintf(stderr,
                    "%s(): last section number does not match\n",
                    __FUNCTION__);
                return -1;
            }
        }
        if (sectionPattern & (1 << tvct->head.ext_head.section_number)) {
            continue;
        }
        sectionPattern |= 1 << tvct->head.ext_head.section_number;

        atsc_tvct_section_channels_for_each (tvct, ch, i) {
            /* initialize the currInfo structure */
            /* each EIT covers 3 hours */
            ChannelBackend *currInfo = new ChannelBackend();
            char *servicename = reinterpret_cast<char*>(malloc(sizeof(char)*8));
            for (k = 0; k < 7; k++) {
                servicename[k] =
                    getBits((const uint8_t *)ch->short_name,
                    k * 16, 16);
            }

            servicename[7] = '\0';
            int majorNum = ch->major_channel_number;
            int minorNum = ch->minor_channel_number;
            int logicalChannelNumber = majorNum << 10 | minorNum;

            printf("TSID : %d\n", ch->channel_TSID);
            printf("ProgramNumbe : %d\n", ch->program_number);
            printf("LCN : %d\n",logicalChannelNumber);
            printf("ServiceID : %d\n",ch->source_id);

            currInfo->setNumber(to_string(logicalChannelNumber));
            currInfo->setTransportStreamId(to_string(ch->channel_TSID));
            currInfo->setServiceId(to_string(ch->source_id));
            currInfo->setProgramNumber(ch->program_number);
            currInfo->setFrequency(frequency);
            printf("currInfo->servicename : %c%c%c%c%c%c%c\n", servicename[0], servicename[1], servicename[2], servicename[3], servicename[4], servicename[5], servicename[6]);
            struct descriptor *desc;
            int i, j;
            atsc_tvct_channel_descriptors_for_each(ch, desc) {
                struct atsc_extended_channel_name_descriptor *extendedChannelName = atsc_extended_channel_name_descriptor_codec(desc);
                if (extendedChannelName) {
                    struct atsc_text* text = atsc_extended_channel_name_descriptor_text(extendedChannelName);
                    struct atsc_text_string *str;
                    atsc_text_strings_for_each(text, str, i) {
                        struct atsc_text_string_segment *seg;
                        int size = 7, pos = 0;
                        atsc_text_string_segments_for_each(str, seg, j) {
                            if (0 > atsc_text_segment_decode(seg,
                                (uint8_t **)&servicename,
                                (size_t *)&size,
                                (size_t *)&pos)) {
                                fprintf(stderr, "%s(): error calling "
                                    "atsc_text_segment_decode()\n",
                                    __FUNCTION__);
                                return -1;
                            }
                        }
                    }
                }
                else{
                    printf("No extended channel name descriptor\n");
                }
            }
            string name(servicename);
            currInfo->setName(name);
            m_channelList.push_back(currInfo);
        }
    } while(sectionPattern != (uint32_t)((1 << numSections) - 1));

    return 0;
}

uint32_t SourceBackend::getBits(const uint8_t *buf, int startbit, int bitlen)
{
    const uint8_t *b;
    uint32_t mask,tmp_long;
    int bitHigh,i;

    b = &buf[startbit / 8];
    startbit %= 8;

    bitHigh = 8;
    tmp_long = b[0];
    for (i = 0; i < ((bitlen-1) >> 3); i++) {
        tmp_long <<= 8;
        tmp_long  |= b[i+1];
        bitHigh   += 8;
    }

    startbit = bitHigh - startbit - bitlen;
    tmp_long = tmp_long >> startbit;
    mask     = (1ULL << bitlen) - 1;
    return tmp_long & mask;
}

} // namespace BCMRPi
