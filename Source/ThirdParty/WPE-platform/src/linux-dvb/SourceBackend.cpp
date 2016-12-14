#include "SourceBackend.h"
#define TV_DEBUG 1

namespace BCMRPi {

SourceBackend::SourceBackend(SourceType type, dvbfe_handle* feHandle)
    : m_sType(type)
    , m_feHandle(feHandle) {

}

void SourceBackend::startScanning() {
    /* */
}

void SourceBackend::stopScanning() {
    /* */
}

void  SourceBackend::setCurrentChannel(uint64_t channelNo) {
    /* */
}

void SourceBackend::getChannels() {
    /* */
}

} // namespace BCMRPi
