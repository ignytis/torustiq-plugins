#include <string>

#include <gtest/gtest.h>

#include "test_util.h"

TEST(UppercaseTransformerTest, UppercasesBodyAndKeepsHeaders) {
    EXPECT_STREQ(torustiq_plugin_get_info()->id, "uppercase_transformer");
    test::Registered registered;
    const torustiq_registrar registrar = test::MakeRegistrar(&registered);
    ASSERT_EQ(torustiq_plugin_init(&registrar), TORUSTIQ_OK);
    const torustiq_transform_vtable& v = registered.transform;

    const torustiq_config config{nullptr, 0};
    void* handle = nullptr;
    const torustiq_payload_spec input{TORUSTIQ_PAYLOAD_BYTES};
    torustiq_payload_spec output{TORUSTIQ_PAYLOAD_VALUE};
    ASSERT_EQ(v.create(&config, &input, &output, &handle), TORUSTIQ_OK);
    EXPECT_EQ(output.kind, TORUSTIQ_PAYLOAD_BYTES);

    const std::string body = "hello, World 42!";
    const torustiq_header header{"loop_number", "1"};
    torustiq_message message{};
    message.payload =
        torustiq_value_from_bytes(reinterpret_cast<const uint8_t*>(body.data()), body.size());
    message.headers = &header;
    message.header_count = 1;

    std::vector<test::Captured> out;
    EXPECT_EQ(v.process(handle, &message, test::Collect, &out), TORUSTIQ_OK);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0].body, "HELLO, WORLD 42!");
    EXPECT_EQ(out[0].headers.at("loop_number"), "1");
    EXPECT_EQ(body, "hello, World 42!"); // the input is not modified
    v.destroy(handle);
}

TEST(UppercaseTransformerTest, RejectsStructuredPayloadWhenCreated) {
    test::Registered registered;
    const torustiq_registrar registrar = test::MakeRegistrar(&registered);
    ASSERT_EQ(torustiq_plugin_init(&registrar), TORUSTIQ_OK);
    const torustiq_config config{nullptr, 0};
    void* handle = nullptr;
    const torustiq_payload_spec input{TORUSTIQ_PAYLOAD_VALUE};
    torustiq_payload_spec output{};
    EXPECT_EQ(registered.transform.create(&config, &input, &output, &handle),
              TORUSTIQ_UNSUPPORTED_PAYLOAD);
}
