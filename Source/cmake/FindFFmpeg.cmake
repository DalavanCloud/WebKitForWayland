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

#  FFMPEG_INCLUDE_DIR - The FFMPEG include directory
#  FFMPEG_LIB - The libraries needed to use FFMPEG


#**************************************************************
#Find the FFMPEG LIBRARIES
#
#************************************************************


find_library(FFMPEG_FILTER_LIB NAMES avfilter)
find_library(FFMPEG_FORMAT_LIB NAMES avformat)
find_library(FFMPEG_CODEC_LIB NAMES avcodec)
find_library(FFMPEG_UTIL_LIB NAMES avutil)

set(FFMPEG_LIB ${FFMPEG_FILTER_LIB} ${FFMPEG_FORMAT_LIB} ${FFMPEG_CODEC_LIB} ${FFMPEG_UTIL_LIB} )

include(FindPackageHandleStandardArgs)
set(FFMPEG_LIB ${FFMPEG_LIB}  CACHE PATH "Path to FFMPEG library")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFmpeg DEFAULT_MSG  FFMPEG_LIB)
mark_as_advanced(FFMPEG_LIB)


#//TODO  Add headers  and other FFMPEG libraries

