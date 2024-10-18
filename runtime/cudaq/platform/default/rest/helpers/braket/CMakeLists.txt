# ============================================================================ #
# Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

target_sources(cudaq-rest-qpu PRIVATE BraketExecutor.cpp BraketServerHelper.cpp)
add_target_config(braket)
add_library(cudaq-serverhelper-braket SHARED BraketExecutor.cpp BraketServerHelper.cpp)

target_link_libraries(cudaq-serverhelper-braket 
aws-cpp-sdk-braket aws-cpp-sdk-s3-crt aws-cpp-sdk-sts aws-cpp-sdk-core
cudaq-common fmt::fmt-header-only)

install(TARGETS cudaq-serverhelper-braket DESTINATION lib)
