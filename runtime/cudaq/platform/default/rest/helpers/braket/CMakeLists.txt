# ============================================================================ #
# Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

message("building cudaq-braket")
set(LIBRARY_NAME cudaq-braket)
add_library(cudaq-braket SHARED BraketExecutor.cpp BraketServerHelper.cpp)

find_package(AWSSDK REQUIRED COMPONENTS braket s3-crt sts)

message("found ${AWSSDK_LINK_LIBRARIES}")

target_include_directories(cudaq-braket PRIVATE . 
    PUBLIC 
       $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/runtime>
       $<INSTALL_INTERFACE:include>)
target_link_libraries(cudaq-braket
PUBLIC cudaq-spin 
cudaq-common
PRIVATE
${AWSSDK_LINK_LIBRARIES} 
pthread
cudaq-mlir-runtime fmt::fmt-header-only cudaq cudaq-platform-default)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)


add_target_config(braket)