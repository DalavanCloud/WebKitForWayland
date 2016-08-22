# - Try to find the CFLite library
# Once done this will define
#
#  CFLITE_INCLUDE_DIR - The CFLite include directory
#  CFLITE_LIBRARIES - The libraries needed to use CFLite
#
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


#**************************************************************
#Extract the list of supported voices fron conf file
#
#**************************************************************
#locate default.lv
set(FLITE_SHARE /share/flite)
set(FLITE_IN /usr/include/flite)
find_path(CFLITE_CONF_FILE NAMES default.lv
  HINTS ${FLITE_SHARE}
 )
set(CFLITE_CONF_FILE_NAME ${CFLITE_CONF_FILE}/default.lv)
message("Flite configuration  flie -" ${CFLITE_CONF_FILE_NAME} )

#get value of voice
file(READ ${CFLITE_CONF_FILE_NAME} CONFFILE)
string(LENGTH "${CONFFILE}" LEN)
set(SUB_STR "VOXES =")
string(FIND "${CONFFILE}" ${SUB_STR} VOXES_START)
string(SUBSTRING "${CONFFILE}" ${VOXES_START} ${LEN} VOICE  )
string(REGEX MATCHALL "cmu_[^ \n]*" VOICES "${VOICE}")

message("Flite supported voices - " ${VOICES})
#populate flite include and shared directories
set (SHARE_DIR "${CMAKE_FIND_ROOT_PATH}${FLITE_SHARE}" )
set (FLITE_INC_DIR "${CMAKE_FIND_ROOT_PATH}${FLITE_IN}")

#**************************************************************
#Execute the script to create voice list .c file
#
#**************************************************************
find_program(FLITE_CONFIG_SCRIPT NAMES make_voice_list
          HINTS ${SHARE_DIR}
          )
message("Flite voicelist creation script - " ${FLITE_CONFIG_SCRIPT})
if(FLITE_CONFIG_SCRIPT)
    execute_process(
        COMMAND "${FLITE_CONFIG_SCRIPT}"  ${VOICES}
        WORKING_DIRECTORY ${FLITE_INC_DIR}
        RESULT_VARIABLE CONFIGSCRIPT_RESULT
        OUTPUT_VARIABLE PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

#**************************************************************
#Find FLite libraries
#**************************************************************

#flite detfault libraries
set(CFLITE_LIBS flite)
find_library(CFLITE_LIB NAMES flite)
find_library(CFLITE_CMULEX_LIB NAMES flite_cmulex)
find_library(CFLITE_USENGLISH NAMES flite_usenglish)
find_library(ASOUND_LIB NAMES asound)

#flite voices libraries identified using default.lv
foreach (_library ${VOICES})
    find_library(FLITE_LIBRARIES_${_library} flite_${_library})
    set(CFLITE_VOICES_LIBRARIES  ${CFLITE_VOICES_LIBRARIES}  ${FLITE_LIBRARIES_${_library}})
endforeach() 

set(CFLITE_LIBRARIES ${CFLITE_LIB} ${CFLITE_CMULEX_LIB}  
    ${CFLITE_USENGLISH} ${ASOUND_LIB} ${CFLITE_VOICES_LIBRARIES} )

#**************************************************************
#find include directory
#**************************************************************
find_path(CFLITE_INCLUDE_DIR NAMES flite.h 
      HINTS ${FLITE_IN}
    )

include(FindPackageHandleStandardArgs)
set(CFLITE_LIBRARIES ${CFLITE_LIBRARIES}  CACHE PATH "Path to Flite library" )
set(CFLITE_INCLUDE_DIR ${CFLITE_INCLUDE_DIR}  CACHE PATH "Path to Flite headers" )

FIND_PACKAGE_HANDLE_STANDARD_ARGS(CFLite DEFAULT_MSG CFLITE_LIBRARIES  CFLITE_INCLUDE_DIR)
mark_as_advanced(CFLITE_INCLUDE_DIR CFLITE_LIBRARIES )
