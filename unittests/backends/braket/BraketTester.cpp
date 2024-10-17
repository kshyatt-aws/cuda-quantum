/*******************************************************************************
 * Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

#include "CUDAQTestUtils.h"
#include "common/FmtCore.h"
#include "cudaq/algorithm.h"
#include <fstream>
#include <gtest/gtest.h>
#include <regex>

// port number and localhost connect to mock_qpu backend server within the
// container (mock_qpu/braket).
std::string mockPort = "5000";
std::string machine = "telegraph-8q";
std::string backendStringTemplate =
//    "braket;emulate;false;url;http://localhost:{};credentials;{};machine;{}";
    "braket;emulate;false;url;http://localhost:{};credentials;{}";
bool isValidExpVal(double value) {
  // give us some wiggle room while keep the tests fast
  return value < -1.1 && value > -2.3;
}

CUDAQ_TEST(BraketTester, checkSampleSync) {
  std::cout << "running Braket sample sync test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
//                                   mockPort, fileName, machine);
                                   mockPort, fileName);

  auto &platform = cudaq::get_platform();
  std::cout << "sample sync test got platform. backendString: " << backendString << std::endl;
  platform.setTargetBackend(backendString);
  std::cout << "sample sync test set target backend" << std::endl;

  auto kernel = cudaq::make_kernel();
  auto qubit = kernel.qalloc(2);
  kernel.h(qubit[0]);
  kernel.mz(qubit[0]);
  std::cout << "sample sync test created kernel" << std::endl;
  auto counts = cudaq::sample(kernel);
  std::cout << "sample sync test got counts" << std::endl;
  counts.dump();
  std::cout << "sample sync test dumped counts" << std::endl;
  EXPECT_EQ(counts.size(), 2);
  std::cout << "done running Braket sample sync test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkSampleSyncEmulate) {
  std::cout << "running Braket sample sync emulate test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);
  backendString =
      std::regex_replace(backendString, std::regex("false"), "true");

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto kernel = cudaq::make_kernel();
  auto qubit = kernel.qalloc(2);
  kernel.h(qubit[0]);
  kernel.x<cudaq::ctrl>(qubit[0], qubit[1]);
  kernel.mz(qubit[0]);
  kernel.mz(qubit[1]);

  auto counts = cudaq::sample(kernel);
  counts.dump();
  EXPECT_EQ(counts.size(), 2);
  std::cout << "done running Braket sample sync emulate test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkSampleAsync) {
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto kernel = cudaq::make_kernel();
  auto qubit = kernel.qalloc(2);
  kernel.h(qubit[0]);
  kernel.mz(qubit[0]);

  auto future = cudaq::sample_async(kernel);
  auto counts = future.get();
  EXPECT_EQ(counts.size(), 2);
}

CUDAQ_TEST(BraketTester, checkSampleAsyncEmulate) {
  std::cout << "running Braket sample async emulate test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);
  backendString =
      std::regex_replace(backendString, std::regex("false"), "true");

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto kernel = cudaq::make_kernel();
  auto qubit = kernel.qalloc(2);
  kernel.h(qubit[0]);
  kernel.mz(qubit[0]);

  auto future = cudaq::sample_async(kernel);
  auto counts = future.get();
  counts.dump();
  EXPECT_EQ(counts.size(), 2);
  std::cout << "done running Braket sample async emulate test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkSampleAsyncLoadFromFile) {
  std::cout << "running Braket sample async load from file test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto kernel = cudaq::make_kernel();
  auto qubit = kernel.qalloc(2);
  kernel.h(qubit[0]);
  kernel.mz(qubit[0]);

  // Can sample asynchronously and get a future
  auto future = cudaq::sample_async(kernel);

  // Future can be persisted for later
  {
    std::ofstream out("saveMe.json");
    out << future;
  }

  // Later you can come back and read it in
  cudaq::async_result<cudaq::sample_result> readIn;
  std::ifstream in("saveMe.json");
  in >> readIn;

  // Get the results of the read in future.
  auto counts = readIn.get();
  EXPECT_EQ(counts.size(), 2);

  std::remove("saveMe.json");
  std::cout << "done running Braket sample async load from file test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkObserveSync) {
  std::cout << "running Braket observe sync test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto [kernel, theta] = cudaq::make_kernel<double>();
  auto qubit = kernel.qalloc(2);
  kernel.x(qubit[0]);
  kernel.ry(theta, qubit[1]);
  kernel.x<cudaq::ctrl>(qubit[1], qubit[0]);

  using namespace cudaq::spin;
  cudaq::spin_op h = 5.907 - 2.1433 * x(0) * x(1) - 2.1433 * y(0) * y(1) +
                     .21829 * z(0) - 6.125 * z(1);
  auto result = cudaq::observe(10000, kernel, h, .59);
  result.dump();

  printf("ENERGY: %lf\n", result.expectation());
  EXPECT_TRUE(isValidExpVal(result.expectation()));
  std::cout << "done running Braket observe sync test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkObserveSyncEmulate) {
  std::cout << "running Braket observe sync emulate test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);
  backendString =
      std::regex_replace(backendString, std::regex("false"), "true");

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto [kernel, theta] = cudaq::make_kernel<double>();
  auto qubit = kernel.qalloc(2);
  kernel.x(qubit[0]);
  kernel.ry(theta, qubit[1]);
  kernel.x<cudaq::ctrl>(qubit[1], qubit[0]);

  using namespace cudaq::spin;
  cudaq::spin_op h = 5.907 - 2.1433 * x(0) * x(1) - 2.1433 * y(0) * y(1) +
                     .21829 * z(0) - 6.125 * z(1);
  auto result = cudaq::observe(100000, kernel, h, .59);
  result.dump();

  printf("ENERGY: %lf\n", result.expectation());
  EXPECT_TRUE(isValidExpVal(result.expectation()));
  std::cout << "done running Braket observe sync test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkObserveAsync) {
  std::cout << "running Braket observe async test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto [kernel, theta] = cudaq::make_kernel<double>();
  auto qubit = kernel.qalloc(2);
  kernel.x(qubit[0]);
  kernel.ry(theta, qubit[1]);
  kernel.x<cudaq::ctrl>(qubit[1], qubit[0]);

  using namespace cudaq::spin;
  cudaq::spin_op h = 5.907 - 2.1433 * x(0) * x(1) - 2.1433 * y(0) * y(1) +
                     .21829 * z(0) - 6.125 * z(1);
  auto future = cudaq::observe_async(kernel, h, .59);

  auto result = future.get();
  result.dump();

  printf("ENERGY: %lf\n", result.expectation());
  EXPECT_TRUE(isValidExpVal(result.expectation()));
  std::cout << "done running Braket observe async test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkObserveAsyncEmulate) {
  std::cout << "running Braket observe async emulate test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);
  backendString =
      std::regex_replace(backendString, std::regex("false"), "true");

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto [kernel, theta] = cudaq::make_kernel<double>();
  auto qubit = kernel.qalloc(2);
  kernel.x(qubit[0]);
  kernel.ry(theta, qubit[1]);
  kernel.x<cudaq::ctrl>(qubit[1], qubit[0]);

  using namespace cudaq::spin;
  cudaq::spin_op h = 5.907 - 2.1433 * x(0) * x(1) - 2.1433 * y(0) * y(1) +
                     .21829 * z(0) - 6.125 * z(1);
  auto future = cudaq::observe_async(100000, 0, kernel, h, .59);

  auto result = future.get();
  result.dump();

  printf("ENERGY: %lf\n", result.expectation());
  EXPECT_TRUE(isValidExpVal(result.expectation()));
  std::cout << "done running Braket observe async emulate test" << std::endl;
}

CUDAQ_TEST(BraketTester, checkObserveAsyncLoadFromFile) {
  std::cout << "running Braket observe async load from file test" << std::endl;
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  auto backendString = fmt::format(fmt::runtime(backendStringTemplate),
                                   mockPort, fileName, machine);

  auto &platform = cudaq::get_platform();
  platform.setTargetBackend(backendString);

  auto [kernel, theta] = cudaq::make_kernel<double>();
  auto qubit = kernel.qalloc(2);
  kernel.x(qubit[0]);
  kernel.ry(theta, qubit[1]);
  kernel.x<cudaq::ctrl>(qubit[1], qubit[0]);

  using namespace cudaq::spin;
  cudaq::spin_op h = 5.907 - 2.1433 * x(0) * x(1) - 2.1433 * y(0) * y(1) +
                     .21829 * z(0) - 6.125 * z(1);
  auto future = cudaq::observe_async(kernel, h, .59);

  {
    std::ofstream out("saveMeObserve.json");
    out << future;
  }

  // Later you can come back and read it in
  cudaq::async_result<cudaq::observe_result> readIn(&h);
  std::ifstream in("saveMeObserve.json");
  in >> readIn;

  // Get the results of the read in future.
  auto result = readIn.get();

  std::remove("saveMeObserve.json");
  result.dump();

  printf("ENERGY: %lf\n", result.expectation());
  EXPECT_TRUE(isValidExpVal(result.expectation()));
  std::cout << "done running Braket observe async load from file test" << std::endl;
}

int main(int argc, char **argv) {
  std::string home = std::getenv("HOME");
  std::string fileName = home + "/FakeCppBraket.config";
  std::ofstream out(fileName);
  out << "credentials: "
         "{\"username\":\"testuser0\",\"password\":\"testuser0passwd\"}";
  out.close();
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  std::remove(fileName.c_str());
  return ret;
}
