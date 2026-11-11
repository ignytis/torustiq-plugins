#include <cstring>
#include <string>

#include <gtest/gtest.h>

#include "test_util.h"

namespace {

class HelloInputTest : public testing::Test {
  protected:
    void SetUp() override {
        const torustiq_registrar registrar = test::MakeRegistrar(&registered_);
        ASSERT_EQ(torustiq_plugin_init(&registrar), TORUSTIQ_OK);
    }
    const torustiq_input_vtable& vtable() const {
        return registered_.input;
    }

    test::Registered registered_;
};

} // namespace

TEST_F(HelloInputTest, ReportsInfoAndRegisters) {
    const torustiq_plugin_info* info = torustiq_plugin_get_info();
    EXPECT_STREQ(info->id, "hello_input");
    EXPECT_EQ(info->abi_version, TORUSTIQ_SDK_ABI_VERSION);
    EXPECT_STREQ(vtable().name, "hello_input");
}

TEST_F(HelloInputTest, EmitsNMessagesWithLoopNumberHeader) {
    const torustiq_header items[] = {{"n", "3"}};
    const torustiq_config config{items, 1};
    torustiq_payload_spec spec{};
    void* handle = nullptr;
    ASSERT_EQ(vtable().create(&config, &spec, &handle), TORUSTIQ_OK);
    EXPECT_EQ(spec.kind, TORUSTIQ_PAYLOAD_BYTES); // declares what it emits
    std::vector<test::Captured> out;
    EXPECT_EQ(vtable().run(handle, test::Collect, &out), TORUSTIQ_OK);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0].kind, static_cast<uint32_t>(TORUSTIQ_VALUE_BYTES));
    for (size_t i = 0; i < out.size(); ++i) {
        const std::string n = std::to_string(i + 1);
        EXPECT_EQ(out[i].body, "hello, " + n + "!");
        EXPECT_EQ(out[i].header_order, std::vector<std::string>{"loop_number"});
        EXPECT_EQ(out[i].headers.at("loop_number"), n);
    }
    vtable().destroy(handle);
}

TEST_F(HelloInputTest, StepsGetIndependentHandles) {
    const torustiq_header items[] = {{"n", "1"}};
    const torustiq_config config{items, 1};
    torustiq_payload_spec spec{};
    void* a = nullptr;
    void* b = nullptr;
    ASSERT_EQ(vtable().create(&config, &spec, &a), TORUSTIQ_OK);
    ASSERT_EQ(vtable().create(&config, &spec, &b), TORUSTIQ_OK);
    EXPECT_NE(a, b);
    vtable().destroy(a);
    vtable().destroy(b);
}

TEST_F(HelloInputTest, RejectsInvalidN) {
    for (const char* bad : {"0", "-1", "abc", "", "1.5", "99999999999999999999999"}) {
        const torustiq_header items[] = {{"n", bad}};
        const torustiq_config config{items, 1};
        torustiq_payload_spec spec{};
        void* handle = nullptr;
        EXPECT_EQ(vtable().create(&config, &spec, &handle), TORUSTIQ_ERROR) << "n=" << bad;
    }
    const torustiq_config empty{nullptr, 0};
    torustiq_payload_spec spec{};
    void* handle = nullptr;
    EXPECT_EQ(vtable().create(&empty, &spec, &handle), TORUSTIQ_ERROR);
}

TEST_F(HelloInputTest, StopBeforeRunEmitsNothing) {
    const torustiq_header items[] = {{"n", "5"}};
    const torustiq_config config{items, 1};
    torustiq_payload_spec spec{};
    void* handle = nullptr;
    ASSERT_EQ(vtable().create(&config, &spec, &handle), TORUSTIQ_OK);
    vtable().stop(handle);
    std::vector<test::Captured> out;
    EXPECT_EQ(vtable().run(handle, test::Collect, &out), TORUSTIQ_OK);
    EXPECT_TRUE(out.empty());
    vtable().destroy(handle);
}
