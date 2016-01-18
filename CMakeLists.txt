cmake_minimum_required(VERSION 3.0.0)

# FOR clion only.
project(mutilate)

include_directories(
    ./
    ./config.h
)

set(SOURCES
    src/barrier.cc
    src/cmdline.cc
    src/distributions.cc
    src/Generator.cc
    src/log.cc
    src/mutilate.cc
    src/Protocol.cc
    src/TestGenerator.cc
    src/util.cc
    src/agent/masterless.cpp
    src/connection/Connection.cc
)


# Mutilate library.
add_executable(mutilate ${SOURCES})

