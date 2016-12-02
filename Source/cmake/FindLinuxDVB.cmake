 #
 # Copyright (C) 2016 TATA ELXSI
 # Copyright (C) 2016 Metrological
 # All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 # 1. Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 # 2. Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 # "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 # LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 # PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 # HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 # SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 # LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 # DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 # THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 #/
 #  LINUX_DVB_INCLUDE_DIR - Linux DVB include directory
 #  LINUX_DVB_LIB - The libraries needed to use linux DVB


#**************************************************************
#Find the Linux DVB  LIBRARIES and INCLUDES
#
#************************************************************

find_path(LINUX_DVB_INCLUDE_DIRS
    NAMES dvbfe.h
    PATH_SUFFIXES libdvbapi
)

find_library(LINUX_DVB_LIBRARIES
    NAMES dvbapi
)

include(FindPackageHandleStandardArgs)
set(LINUX_DVB_LIBRARIES ${LINUX_DVB_LIBRARIES}  CACHE PATH "Path to LINUX_DVB library")
set(LINUX_DVB_INCLUDE_DIRS ${LINUX_DVB_INCLUDE_DIRS}  CACHE PATH "Path to LINUX_DVB include")

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LINUX_DVB DEFAULT_MSG  LINUX_DVB_LIBRARIES LINUX_DVB_INCLUDE_DIRS)
mark_as_advanced(LINUX_DVB_INCLUDE_DIRS LINUX_DVB_LIBRARIES)
