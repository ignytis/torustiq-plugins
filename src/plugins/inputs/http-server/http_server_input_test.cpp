// These tests never start an HTTP server: `create` only validates config and `run` returns
// before binding a socket when `stop` was called first.
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "test_util.h"
#include "torustiq/sdk.h"

namespace {

class HttpServerInputTest : public ::testing::Test {
  protected:
    void SetUp() override {
        const torustiq_registrar registrar = test::MakeRegistrar(&registered_);
        ASSERT_EQ(torustiq_plugin_init(&registrar), TORUSTIQ_OK);
    }

    test::Registered registered_;
};

TEST_F(HttpServerInputTest, ReportsInfoAndRegisters) {
    EXPECT_STREQ(torustiq_plugin_get_info()->id, "http_server_input");
    EXPECT_STREQ(registered_.input.name, "http_server_input");
    EXPECT_NE(registered_.input.run, nullptr);
    EXPECT_NE(registered_.input.stop, nullptr);
}

TEST_F(HttpServerInputTest, AcceptsValidConfig) {
    for (const char* port : {"1", "8080", "65535"}) {
        torustiq_payload_spec spec{};
        void* handle = nullptr;
        const torustiq_header items[] = {{"host", "127.0.0.1"}, {"port", port}};
        const torustiq_config config{items, 2};
        ASSERT_EQ(registered_.input.create(&config, &spec, &handle), TORUSTIQ_OK) << port;
        EXPECT_EQ(spec.kind, TORUSTIQ_PAYLOAD_BYTES);
        registered_.input.destroy(handle);
    }
}

TEST_F(HttpServerInputTest, HostIsOptional) {
    torustiq_payload_spec spec{};
    void* handle = nullptr;
    const torustiq_header items[] = {{"port", "8080"}};
    const torustiq_config config{items, 1};
    ASSERT_EQ(registered_.input.create(&config, &spec, &handle), TORUSTIQ_OK);
    registered_.input.destroy(handle);
}

TEST_F(HttpServerInputTest, RejectsInvalidPort) {
    for (const char* port :
         {"", "0", "-1", "abc", "65536", "70000", "1.5", "99999999999999999999"}) {
        torustiq_payload_spec spec{};
        void* handle = nullptr;
        const torustiq_header items[] = {{"port", port}};
        const torustiq_config config{items, 1};
        EXPECT_EQ(registered_.input.create(&config, &spec, &handle), TORUSTIQ_ERROR) << port;
    }
}

TEST_F(HttpServerInputTest, RejectsMissingPort) {
    torustiq_payload_spec spec{};
    void* handle = nullptr;
    const torustiq_config empty{nullptr, 0};
    EXPECT_EQ(registered_.input.create(&empty, &spec, &handle), TORUSTIQ_ERROR);
}

TEST_F(HttpServerInputTest, StopBeforeRunReturnsWithoutServing) {
    torustiq_payload_spec spec{};
    void* handle = nullptr;
    const torustiq_header items[] = {{"port", "8080"}};
    const torustiq_config config{items, 1};
    ASSERT_EQ(registered_.input.create(&config, &spec, &handle), TORUSTIQ_OK);

    std::vector<test::Captured> captured;
    registered_.input.stop(handle);
    registered_.input.stop(handle); // Idempotent.
    EXPECT_EQ(registered_.input.run(handle, test::Collect, &captured), TORUSTIQ_OK);
    EXPECT_TRUE(captured.empty());
    registered_.input.destroy(handle);
}

} // namespace
