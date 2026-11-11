#include <string>

#include <gtest/gtest.h>

#include "test_util.h"

TEST(PrintOutputTest, PrintsBodyThenIndentedHeaders) {
    EXPECT_STREQ(torustiq_plugin_get_info()->id, "print_output");
    test::Registered registered;
    const torustiq_registrar registrar = test::MakeRegistrar(&registered);
    ASSERT_EQ(torustiq_plugin_init(&registrar), TORUSTIQ_OK);
    const torustiq_output_vtable& v = registered.output;

    const torustiq_config config{nullptr, 0};
    void* handle = nullptr;
    const torustiq_payload_spec input{TORUSTIQ_PAYLOAD_BYTES};
    ASSERT_EQ(v.create(&config, &input, &handle), TORUSTIQ_OK);

    const std::string body = "HELLO, 1!";
    const torustiq_header headers[] = {{"loop_number", "1"}, {"source", "test"}};
    torustiq_message message{};
    message.payload =
        torustiq_value_from_bytes(reinterpret_cast<const uint8_t*>(body.data()), body.size());
    message.headers = headers;
    message.header_count = 2;

    testing::internal::CaptureStdout();
    const int32_t write_status = v.write(handle, &message);
    const int32_t flush_status = v.flush(handle);
    const std::string printed = testing::internal::GetCapturedStdout();

    EXPECT_EQ(write_status, TORUSTIQ_OK);
    EXPECT_EQ(flush_status, TORUSTIQ_OK);
    EXPECT_EQ(printed, "HELLO, 1!\n  loop_number=1\n  source=test\n");
    v.destroy(handle);
}

TEST(PrintOutputTest, RejectsStructuredPayloadWhenCreated) {
    test::Registered registered;
    const torustiq_registrar registrar = test::MakeRegistrar(&registered);
    ASSERT_EQ(torustiq_plugin_init(&registrar), TORUSTIQ_OK);
    const torustiq_config config{nullptr, 0};
    void* handle = nullptr;
    const torustiq_payload_spec input{TORUSTIQ_PAYLOAD_VALUE};
    EXPECT_EQ(registered.output.create(&config, &input, &handle), TORUSTIQ_UNSUPPORTED_PAYLOAD);
}
