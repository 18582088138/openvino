// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "behavior/ov_plugin/core_integration.hpp"
#include <openvino/runtime/properties.hpp>
#include "ie_system_conf.h"
#include "openvino/runtime/core.hpp"
#include "openvino/core/type/element_type.hpp"

using namespace ov::test::behavior;
using namespace InferenceEngine::PluginConfigParams;

namespace {
//
// IE Class Common tests with <pluginName, deviceName params>
//

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassCommon, OVClassBasicTestP,
        ::testing::Values(std::make_pair("openvino_intel_cpu_plugin", "CPU")));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassNetworkTestP, OVClassNetworkTestP,
        ::testing::Values("CPU"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassImportExportTestP, OVClassImportExportTestP,
        ::testing::Values("HETERO:CPU"));

//
// IE Class GetMetric
//

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_SUPPORTED_CONFIG_KEYS,
        ::testing::Values("CPU", "MULTI", "HETERO", "AUTO"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_SUPPORTED_METRICS,
        ::testing::Values("CPU", "MULTI", "HETERO", "AUTO"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_AVAILABLE_DEVICES,
        ::testing::Values("CPU"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_FULL_DEVICE_NAME,
        ::testing::Values("CPU", "MULTI", "HETERO", "AUTO"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_OPTIMIZATION_CAPABILITIES,
        ::testing::Values("CPU"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_RANGE_FOR_ASYNC_INFER_REQUESTS,
        ::testing::Values("CPU"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_RANGE_FOR_STREAMS,
        ::testing::Values("CPU"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetMetricTest, OVClassGetMetricTest_ThrowUnsupported,
        ::testing::Values("CPU", "MULTI", "HETERO", "AUTO"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetConfigTest, OVClassGetConfigTest_ThrowUnsupported,
        ::testing::Values("CPU", "MULTI", "HETERO", "AUTO"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetAvailableDevices, OVClassGetAvailableDevices,
        ::testing::Values("CPU"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassSetModelPriorityConfigTest, OVClassSetModelPriorityConfigTest,
        ::testing::Values("MULTI", "AUTO"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassSetTBBForceTerminatePropertyTest, OVClassSetTBBForceTerminatePropertyTest,
        ::testing::Values("AUTO", "GPU"));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassSetLogLevelConfigTest, OVClassSetLogLevelConfigTest,
        ::testing::Values("MULTI", "AUTO"));

const std::vector<ov::AnyMap> multiConfigs = {
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU)}
};

const std::vector<ov::AnyMap> configsWithSecondaryProperties = {
    {ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT))},
    {ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT)),
     ov::device::properties("GPU", ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY))}};

const std::vector<ov::AnyMap> multiConfigsWithSecondaryProperties = {
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU),
     ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT))},
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU),
     ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT)),
     ov::device::properties("GPU", ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY))}};

const std::vector<ov::AnyMap> autoConfigsWithSecondaryProperties = {
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU),
     ov::device::properties("AUTO",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT))},
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU),
     ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT))},
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU),
     ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT)),
     ov::device::properties("GPU", ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY))},
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU),
     ov::device::properties("AUTO",
                            ov::enable_profiling(false),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY)),
     ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT))},
    {ov::device::priorities(CommonTestUtils::DEVICE_CPU),
     ov::device::properties("AUTO",
                            ov::enable_profiling(false),
                            ov::device::priorities(CommonTestUtils::DEVICE_GPU),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY)),
     ov::device::properties("CPU",
                            ov::enable_profiling(true),
                            ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT)),
     ov::device::properties("GPU", ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY))}};

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassSetDevicePriorityConfigTest, OVClassSetDevicePriorityConfigTest,
        ::testing::Combine(::testing::Values("MULTI", "AUTO"),
                           ::testing::ValuesIn(multiConfigs)));
//
// IE Class GetConfig
//

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassGetConfigTest, OVClassGetConfigTest,
        ::testing::Values("CPU"));

//////////////////////////////////////////////////////////////////////////////////////////

TEST(OVClassBasicTest, smoke_SetConfigInferenceNumThreads) {
    ov::Core ie;
    int32_t value = 0;
    int32_t num_threads = 1;

    OV_ASSERT_NO_THROW(ie.set_property("CPU", ov::inference_num_threads(num_threads)));
    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::inference_num_threads));
    ASSERT_EQ(num_threads, value);

    num_threads = 4;

    OV_ASSERT_NO_THROW(ie.set_property("CPU", ov::inference_num_threads(num_threads)));
    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::inference_num_threads));
    ASSERT_EQ(num_threads, value);
}

TEST(OVClassBasicTest, smoke_SetConfigStreamsNum) {
    ov::Core ie;
    int32_t value = 0;
    int32_t num_streams = 1;

    auto setGetProperty = [&ie](int32_t& getProperty, int32_t setProperty){
        OV_ASSERT_NO_THROW(ie.set_property("CPU", ov::num_streams(setProperty)));
        OV_ASSERT_NO_THROW(getProperty = ie.get_property("CPU", ov::num_streams));
    };

    setGetProperty(value, num_streams);
    ASSERT_EQ(num_streams, value);

    num_streams = ov::streams::NUMA;

    setGetProperty(value, num_streams);
    ASSERT_GT(value, 0); // value has been configured automatically

    num_streams = ov::streams::AUTO;

    setGetProperty(value, num_streams);
    ASSERT_GT(value, 0); // value has been configured automatically
}

TEST(OVClassBasicTest, smoke_SetConfigAffinity) {
    ov::Core ie;
    ov::Affinity value = ov::Affinity::NONE;

#if (defined(__APPLE__) || defined(_WIN32))
    auto numaNodes = InferenceEngine::getAvailableNUMANodes();
    auto coreTypes = InferenceEngine::getAvailableCoresTypes();
    auto defaultBindThreadParameter = ov::Affinity::NONE;
    if (coreTypes.size() > 1) {
        defaultBindThreadParameter = ov::Affinity::HYBRID_AWARE;
    } else if (numaNodes.size() > 1) {
        defaultBindThreadParameter = ov::Affinity::NUMA;
    }
#else
    auto defaultBindThreadParameter = ov::Affinity::CORE;
#endif
    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::affinity));
    ASSERT_EQ(defaultBindThreadParameter, value);

    const ov::Affinity affinity = defaultBindThreadParameter == ov::Affinity::HYBRID_AWARE ? ov::Affinity::NUMA : ov::Affinity::HYBRID_AWARE;
    OV_ASSERT_NO_THROW(ie.set_property("CPU", ov::affinity(affinity)));
    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::affinity));
    ASSERT_EQ(affinity, value);
}

TEST(OVClassBasicTest, smoke_SetConfigHintInferencePrecision) {
    ov::Core ie;
    auto value = ov::element::f32;
    const auto precision = InferenceEngine::with_cpu_x86_bfloat16() ? ov::element::bf16 : ov::element::f32;

    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::hint::inference_precision));
    ASSERT_EQ(precision, value);

    const auto forcedPrecision = ov::element::f32;

    OV_ASSERT_NO_THROW(ie.set_property("CPU", ov::hint::inference_precision(forcedPrecision)));
    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::hint::inference_precision));
    ASSERT_EQ(value, forcedPrecision);
}

TEST(OVClassBasicTest, smoke_SetConfigEnableProfiling) {
    ov::Core ie;
    bool value;
    const bool enableProfilingDefault = false;

    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::enable_profiling));
    ASSERT_EQ(enableProfilingDefault, value);

    const bool enableProfiling = true;

    OV_ASSERT_NO_THROW(ie.set_property("CPU", ov::enable_profiling(enableProfiling)));
    OV_ASSERT_NO_THROW(value = ie.get_property("CPU", ov::enable_profiling));
    ASSERT_EQ(enableProfiling, value);
}

// IE Class Query network

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassQueryNetworkTest, OVClassQueryNetworkTest,
        ::testing::Values("CPU"));

// IE Class Load network
INSTANTIATE_TEST_SUITE_P(smoke_CPU_OVClassLoadNetworkWithCorrectSecondaryPropertiesTest,
                         OVClassLoadNetworkWithCorrectPropertiesTest,
                         ::testing::Combine(::testing::Values("CPU", "AUTO:CPU", "MULTI:CPU"),
                                            ::testing::ValuesIn(configsWithSecondaryProperties)));

INSTANTIATE_TEST_SUITE_P(smoke_Multi_OVClassLoadNetworkWithSecondaryPropertiesTest,
                         OVClassLoadNetworkWithCorrectPropertiesTest,
                         ::testing::Combine(::testing::Values("MULTI"),
                                            ::testing::ValuesIn(multiConfigsWithSecondaryProperties)));

INSTANTIATE_TEST_SUITE_P(smoke_AUTO_OVClassLoadNetworkWithSecondaryPropertiesTest,
                         OVClassLoadNetworkWithCorrectPropertiesTest,
                         ::testing::Combine(::testing::Values("AUTO"),
                                            ::testing::ValuesIn(autoConfigsWithSecondaryProperties)));

INSTANTIATE_TEST_SUITE_P(
        smoke_OVClassLoadNetworkTest, OVClassLoadNetworkTest,
        ::testing::Values("CPU"));

const std::vector<ov::AnyMap> default_properties = {{ov::device::priorities("CPU")}};
const std::vector<ov::AnyMap> auto_default_properties = {{}};
INSTANTIATE_TEST_SUITE_P(smoke_OVClassLoadNetworkWithDefaultPropertiesTest,
                         OVClassLoadNetworkWithDefaultPropertiesTest,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_AUTO, CommonTestUtils::DEVICE_MULTI),
                                            ::testing::ValuesIn(default_properties)));

INSTANTIATE_TEST_SUITE_P(smoke_AUTO_OVClassLoadNetworkWithDefaultPropertiesTest,
                         OVClassLoadNetworkWithDefaultPropertiesTest,
                         ::testing::Combine(::testing::Values("AUTO:CPU", "MULTI:CPU"),
                                            ::testing::ValuesIn(auto_default_properties)));

const std::vector<ov::AnyMap> default_incorrect_properties = {
        {ov::device::priorities("CPU"), ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY)},
        {ov::device::priorities("CPU"), ov::device::properties("CPU", ov::hint::allow_auto_batching(true))}
        };
const std::vector<ov::AnyMap> default_multi_incorrect_properties = {
        {ov::device::priorities("CPU"), ov::affinity(ov::Affinity::NONE)},
        {ov::device::priorities("CPU"), ov::num_streams(ov::streams::AUTO)},
        {ov::device::priorities("CPU"), ov::inference_num_threads(1)}
        };
INSTANTIATE_TEST_SUITE_P(smoke_OVClassLoadNetworkWithDefaultIncorrectPropertiesTest,
                         OVClassLoadNetworkWithDefaultIncorrectPropertiesTest,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_AUTO, CommonTestUtils::DEVICE_MULTI),
                                            ::testing::ValuesIn(default_incorrect_properties)));

INSTANTIATE_TEST_SUITE_P(smoke_Multi_OVClassLoadNetworkWithDefaultIncorrectPropertiesTest,
                         OVClassLoadNetworkWithDefaultIncorrectPropertiesTest,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_MULTI),
                                            ::testing::ValuesIn(default_multi_incorrect_properties)));
} // namespace
