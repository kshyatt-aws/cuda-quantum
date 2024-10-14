/*******************************************************************************
 * Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#include "aws/s3-crt/S3CrtClientConfiguration.h"
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

#include <nlohmann/json.hpp>
#include <string>
#include <thread>

namespace cudaq {
class BraketExecutor : public cudaq::Executor {

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

  details::future
  execute(std::vector<KernelExecution> &codesToExecute) override {

    Aws::Client::ClientConfiguration clientConfig;

    std::promise<sample_result> p;
    CountsDictionary cd;

    clientConfig.verifySSL = false; // TODO... fix all of this
    Aws::Braket::BraketClient braketClient{clientConfig};
    Aws::STS::STSClient stsClient{clientConfig};
    Aws::S3Crt::ClientConfiguration s3config;
    s3config.verifySSL = false;
    Aws::S3Crt::S3CrtClient s3Client{s3config};

    auto callerIdResponse = stsClient.GetCallerIdentity();

    if (!callerIdResponse.IsSuccess()) {
      p.set_exception(
          std::make_exception_ptr(std::runtime_error("STS failed")));
      return p.get_future();
    }

    auto account = callerIdResponse.GetResult().GetAccount();

    std::string defaultBucket =
        fmt::format("amazon-braket-{}-{}", clientConfig.region, account);
    std::string defaultPrefix = "tasks-cudaq";

    Aws::Braket::Model::CreateQuantumTaskRequest req;

    std::string sv1_arn =
        "arn:aws:braket:::device/quantum-simulator/amazon/sv1";
    std::string action =
        "{\"braketSchemaHeader\": {\"name\": \"braket.ir.openqasm.program\", "
        "\"version\": \"1\"}, \"source\": \"OPENQASM 3.0;\\nbit[2] "
        "b;\\nqubit[2] q;\\nh q[0];\\ncnot q[0], q[1];\\nb[0] = measure "
        "q[0];\\nb[1] = measure q[1];\", \"inputs\": {}}";

    req.SetAction(action);
    req.SetDeviceArn(sv1_arn);
    req.SetShots(10);
    req.SetOutputS3Bucket(defaultBucket);
    req.SetOutputS3KeyPrefix(defaultPrefix);

    auto response = braketClient.CreateQuantumTask(req);

    if (response.IsSuccess()) {
      std::string taskArn = response.GetResult().GetQuantumTaskArn();
      std::cout << "Created " << taskArn << "\n";
      Aws::Braket::Model::QuantumTaskStatus taskStatus;
      Aws::Braket::Model::GetQuantumTaskRequest getTaskReq;
      getTaskReq.SetQuantumTaskArn(taskArn);

      auto r = braketClient.GetQuantumTask(getTaskReq);
      do {
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
        r = braketClient.GetQuantumTask(getTaskReq);
        taskStatus = r.GetResult().GetStatus();
      } while (taskStatus != Aws::Braket::Model::QuantumTaskStatus::COMPLETED &&
               taskStatus != Aws::Braket::Model::QuantumTaskStatus::FAILED &&
               taskStatus != Aws::Braket::Model::QuantumTaskStatus::CANCELLED);

      std::string outBucket = r.GetResult().GetOutputS3Bucket();
      std::string outPrefix = r.GetResult().GetOutputS3Directory();

      std::cout << "results at " << outBucket << "/" << outPrefix << "\n";
      Aws::S3Crt::Model::GetObjectRequest resultRequest;
      resultRequest.SetBucket(outBucket);
      resultRequest.SetKey(fmt::format("{}/results.json", outPrefix));

      auto results = s3Client.GetObject(resultRequest);

      auto resultjson =
          nlohmann::json::parse(results.GetResultWithOwnership().GetBody());

      auto measurements = resultjson.at("measurements");

      for (auto const &m : measurements) {
        std::string bitString = "";
        for (int bit : m) {
          bitString += std::to_string(bit);
        }
        cd[bitString] += 1;
      }

    } else {
      std::cout << "Create error\n";
    }

    ExecutionResult ex_r{cd};

    sample_result result{ex_r};
    p.set_value(result);

    return {p.get_future()};
  }
};
} // namespace cudaq

CUDAQ_REGISTER_TYPE(cudaq::Executor, cudaq::BraketExecutor, braket);