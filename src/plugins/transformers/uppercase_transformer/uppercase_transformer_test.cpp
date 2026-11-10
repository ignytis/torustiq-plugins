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
    ASSERT_EQ(v.create(&config, &handle), TORUSTIQ_OK);

    const std::string body = "hello, World 42!";
    const torustiq_header header{"loop_number", "1"};
    torustiq_message message{};
    message.body.data = reinterpret_cast<const uint8_t*>(body.data());
    message.body.len = body.size();
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
