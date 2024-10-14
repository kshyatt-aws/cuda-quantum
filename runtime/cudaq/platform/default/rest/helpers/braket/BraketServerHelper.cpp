/*******************************************************************************
 * Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#include <iostream>
#include "common/ServerHelper.h"

namespace cudaq {

class BraketServerHelper : public ServerHelper {
    const std::string name() const override {return "braket";}
    void initialize(BackendConfig config) override {
        std::cerr<<"initialize a braket\n";
    }

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

}

CUDAQ_REGISTER_TYPE(cudaq::ServerHelper, cudaq::BraketServerHelper, braket)