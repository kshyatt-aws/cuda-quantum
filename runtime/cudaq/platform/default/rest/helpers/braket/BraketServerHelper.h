/*******************************************************************************
 * Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

# pragma once

#include <iostream>
#include "common/Logger.h"
#include "common/RestClient.h"
#include "common/ServerHelper.h"
#include <map>
#include <regex>
#include <sstream>
#include <thread>

namespace cudaq {

const std::string SV1 = "sv1";
const std::string DM1 = "dm1";
const std::string TN1 = "tn1";
const std::string Aria1 = "aria1";
const std::string Aria2 = "aria2";
const std::string Garnet = "garnet";
const std::string Aquila = "aquila";

const std::string sv1_arn = "arn:aws:braket:::device/quantum-simulator/amazon/sv1";
const std::string dm1_arn = "arn:aws:braket:::device/quantum-simulator/amazon/dm1";
const std::string tn1_arn = "arn:aws:braket:::device/quantum-simulator/amazon/tn1";
const std::string aria1_arn = "arn:aws:braket:us-east-1::device/qpu/ionq/Aria-1";
const std::string aria2_arn = "arn:aws:braket:us-east-1::device/qpu/ionq/Aria-2";
const std::string garnet_arn = "arn:aws:braket:eu-north-1::device/qpu/iqm/Garnet";
const std::string aquila_arn = "arn:aws:braket:us-east-1::device/qpu/quera/Aquila";

// Do we need to add the `device_name.txt` files for these?
const std::map<std::string, uint> Machines = {{SV1, 34}, {DM1, 17}, {TN1, 50}, {Aria1, 25}, {Aria2, 25}, {Garnet, 20}, {Aquila, 256}};

/// @brief The BraketServerHelper class extends the ServerHelper class to handle
/// interactions with the Amazon Braket server for submitting and retrieving quantum
/// computation jobs.
class BraketServerHelper : public ServerHelper {
  /// @brief Returns the name of the server helper.
  const std::string name() const override {return "braket";}

  /// @brief Initializes the server helper with the provided backend
  /// configuration.
  void initialize(BackendConfig config) override;

  RestHeaders getHeaders() override {return {};}

  ServerJobPayload
  createJob(std::vector<KernelExecution> &circuitCodes) override;

  std::string extractJobId(ServerMessage &postResponse) override {return "";}

  std::string constructGetJobPath(std::string &jobId) override {return "";}

  std::string constructGetJobPath(ServerMessage &postResponse) override {return "";}

  bool jobIsDone(ServerMessage &getJobResponse) override {return true;}

  cudaq::sample_result processResults(ServerMessage &postJobResponse,
                                              std::string &jobId) override{
                                                return {};
                                              }
  private:
    /// @brief Helper function to get value from config or return a default value.
    std::string getValueOrDefault(const BackendConfig &config,
                                  const std::string &key,
                                  const std::string &defaultValue) const;
};

} // namespace cuda-q