/*******************************************************************************
 * Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

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

const std::map<std::string, uint> Machines = {{SV1, 34}, {DM1, 17}, {TN1, 50}};

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
  createJob(std::vector<KernelExecution> &circuitCodes) override {return {};}

  std::string extractJobId(ServerMessage &postResponse) override {return "";}

  std::string constructGetJobPath(std::string &jobId) override {return "";}

  std::string constructGetJobPath(ServerMessage &postResponse) override {return "";}

  bool jobIsDone(ServerMessage &getJobResponse) override {return true;}

  cudaq::sample_result processResults(ServerMessage &postJobResponse,
                                              std::string &jobId) override{
                                                return {};
                                              }
};

namespace {
  std::string get_from_config(BackendConfig config, const std::string &key,
                            const auto &missing_functor) {
    const auto iter = config.find(key);
    auto item = iter != config.end() ? iter->second : missing_functor();
    std::transform(item.begin(), item.end(), item.begin(),
                  [](auto c) { return std::tolower(c); });
    return item;
  }
  void check_machine_allowed(const std::string &machine) {
  if (Machines.find(machine) == Machines.end()) {
    std::string allowed;
    for (const auto &machine : Machines)
      allowed += machine.first + " ";
    std::stringstream stream;
    stream << "machine " << machine
           << " is not of known braket-machine set: " << allowed;
    throw std::runtime_error(stream.str());
  }
}
}

// Initialize the Braket server helper with a given backend configuration
void BraketServerHelper::initialize(BackendConfig config) {
  cudaq::info("Initializing Amazon Braket Backend.");

  // Fetch machine info before checking emulate because we want to be able to
  // emulate specific machines.
  auto machine = get_from_config(config, "machine", []() { return SV1; });
  check_machine_allowed(machine);
  config["machine"] = machine;
  cudaq::info("Running on machine {}", machine);

  const auto emulate_it = config.find("emulate");
  if (emulate_it != config.end() && emulate_it->second == "true") {
    cudaq::info("Emulation is enabled, ignore all braket connection specific "
                "information.");
    backendConfig = std::move(config);
    return;
  }
  config["version"] = "v0.3";
  config["user_agent"] = "cudaq/0.3.0";
  config["target"] = "SV1";
  config["qubits"] = Machines.at(machine);
  // Construct the API job path
  config["job_path"] = "/tasks"; // config["url"] + "/tasks";

  parseConfigForCommonParams(config);

  // Move the passed config into the member variable backendConfig
  backendConfig = std::move(config);
};
}; // namespace cuda-q

CUDAQ_REGISTER_TYPE(cudaq::ServerHelper, cudaq::BraketServerHelper, braket)