#
#   Configuration parts of GNU Make/GCC build system.
#   Copyright (C) 2014 by KO Myung-Hun <komh@chollian.net>
#
#   This file is part of GNU Make/GCC build system.
#
#   This program is free software. It comes without any warranty, to
#   the extent permitted by applicable law. You can redistribute it
#   and/or modify it under the terms of the Do What The Fuck You Want
#   To Public License, Version 2, as published by Sam Hocevar. See
#   http://www.wtfpl.net/ for more details.
#

##### Configuration parts that you can modify

# specify sub directories
SUBDIRS :=

# specify gcc compiler flags for all the programs
CFLAGS := -Wall -std=gnu99 -DOS2EMX_PLAIN_CHAR -funsigned-char

# specify g++ compiler flags for all the programs
CXXFLAGS :=

# specify linker flags such as -L option for all the programs
LDFLAGS := -Zomf

# specify dependent libraries such as -l option for all the programs
LDLIBS := -lmmpm2

ifdef RELEASE
# specify flags for release mode
CFLAGS   +=
CXXFLAGS +=
LDFLAGS  +=
else
# specify flags for debug mode
CFLAGS   +=
CXXFLAGS +=
LDFLAGS  +=
endif

# specify resource compiler, default is rc if not set
RC :=

# specify resource compiler flags
RCFLAGS :=

# set if you want not to compress resources
NO_COMPRESS_RES :=

# specify BLDLEVEL VENDOR string
BLDLEVEL_VENDOR := OS/2 Factory

# specify a macro defining version, and a file including that macro
# to generate BLDLEVEL version string
BLDLEVEL_VERSION_MACRO := KSOFTSEQ_VERSION
BLDLEVEL_VERSION_FILE := mcdtemp.h

# specify BLDLEVEL VERSION string if you want to set VERSION string manually,
# default is generated with BLDLEVEL_VERSION_MACRO and BLDLEVEL_VERSION_FILE
# if unset
BLDLEVEL_VERSION :=

# Variables for programs
#
# 1. specify a list of programs without an extension with
#
#   BIN_PROGRAMS
#
# Now, assume
#
#   BIN_PROGRAMS := program
#
# 2. specify sources for a specific program with
#
#   program_SRCS
#
# the above is REQUIRED
#
# 3. specify various OPTIONAL flags for a specific program with
#
#   program_CFLAGS      for gcc compiler flags
#   program_CXXFLAGS    for g++ compiler flags
#   program_LDFLAGS     for linker flags
#   program_LDLIBS      for dependent libraries
#   program_RCSRC       for rc source
#   program_RCFLAGS     for rc flags
#   program_DEF         for .def file
#   program_EXTRADEPS   for extra dependencies
#   program_DESC        for a BLDLEVEL description string

BIN_PROGRAMS :=

# Variables for libraries
#
# 1. specify a list of libraries without an extension with
#
#   BIN_LIBRARIES
#
# Now, assume
#
#   BIN_LIBRARIES := library
#
# 2. specify sources for a specific library with
#
#   library_SRCS
#
# the above is REQUIRED
#
# 3. set library type flags for a specific library to a non-empty value
#
#   library_LIB         to create a static library
#   library_DLL         to build a DLL
#
# either of the above SHOULD be SET
#
# 4. specify various OPTIONAL flags for a specific library with
#
#   library_CFLAGS      for gcc compiler flags
#   library_CXXFLAGS    for g++ compiler flags
#
# the above is common for LIB and DLL
#
#   library_DLLNAME     for customized DLL name without an extension
#   library_LDFLAGS     for linker flags
#   library_LDLIBS      for dependent libraries
#   library_RCSRC       for rc source
#   library_RCFLAGS     for rc flags
#   library_DEF         for .def file, if not set all the symbols are exported
#   library_NO_EXPORT   if set, no symbols are exported in .def file
#   library_EXTRADEPS   for extra dependencies
#   library_NO_IMPLIB   if set, implib is not generated.
#   library_DESC        for a BLDLEVEL description string
#
# the above is only for DLL

BIN_LIBRARIES := ksoftseq

ksoftseq_SRCS      := mcdproc.c mcdclose.c mcddrvrt.c mcddrvsv.c mcdfuncs.c \
                      mcdinfo.c mcdopen.c mcdstat.c \
                      mcdcaps.c mcdload.c mcdpause.c mcdplay.c mcdresume.c \
                      mcdseek.c mcdset.c mcdcue.c mcdpos.c mcdstop.c \
                      klogger.c
ksoftseq_DLL       := yes
ksoftseq_LDLIBS    := -lkai -lkmididec -lfluidsynth
ksoftseq_DEF       := mcdtemp.def
ksoftseq_NO_IMPLIB := yes
ksoftseq_DESC      := K Soft Sequencer

include Makefile.common

# additional stuffs
