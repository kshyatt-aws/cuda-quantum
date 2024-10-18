/*******************************************************************************
 * Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#pragma once
// #include "aws/s3-crt/S3CrtClientConfiguration.h"
#include "common/Executor.h"
#include "common/FmtCore.h"
#include "common/MeasureCounts.h"
#include "cudaq.h"

#include <chrono>
#include <iostream>

#include <aws/core/Aws.h>

#include <aws/braket/BraketClient.h>
#include <aws/braket/model/CreateQuantumTaskRequest.h>
#include <aws/braket/model/GetQuantumTaskRequest.h>
#include <aws/braket/model/QuantumTaskStatus.h>

#include <aws/sts/STSClient.h>

#include <aws/s3-crt/S3CrtClient.h>
#include <aws/s3-crt/model/GetObjectRequest.h>

#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/LogLevel.h>

#include "common/Logger.h"
#include "BraketServerHelper.h"

#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <thread>

namespace cudaq {
/// @brief The Executor subclass for Amazon Braket
class BraketExecutor : public Executor {
  Aws::SDKOptions options;

  std::string region;
  std::string accountId;

  class ScopedApi {
    Aws::SDKOptions &options;

  public:
    ScopedApi(Aws::SDKOptions &options) : options(options) {

      // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;

      Aws::InitAPI(options);
    }
    ~ScopedApi() { Aws::ShutdownAPI(options); }
  };

  ScopedApi api;

public:
  BraketExecutor() : api(options) {
    std::cerr << "made a braket exec\n";

    // Aws::InitAPI(options);

  }


  ~BraketExecutor() {
    std::cerr << "destroy braket exec\n";

    // Aws::ShutdownAPI(options);
  }

  /// @brief Execute the provided Braket task
  details::future execute(std::vector<KernelExecution> &codesToExecute) override;
};
} // namespace cuda-q