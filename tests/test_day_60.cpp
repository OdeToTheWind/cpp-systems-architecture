// Tests for Day 60 – API Authentication. Credentials are fake and come from an injected environment.
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cppm/testing.hpp"
#include "day_60_api_auth/lesson.hpp"

using namespace cppm::day60;
using namespace std::chrono_literals;

namespace {
const std::map<std::string, std::string> env{{"SHIP_ALPHA_API_KEY", "ak_test_key"},
                                             {"SHIP_BETA_CLIENT_ID", "id"},
                                             {"SHIP_BETA_CLIENT_SECRET", "secret"},
                                             {"SHIP_GAMMA_USER", "Aladdin"},
                                             {"SHIP_GAMMA_PASSWORD", "open sesame"}};
}

TEST_CASE("a Secret never prints its value") {
    const Secret secret("sk_live_abc");
    std::ostringstream out;
    out << secret;
    CHECK_EQ(out.str(), "***");
    CHECK_EQ(secret.reveal(), "sk_live_abc");
}

TEST_CASE("credentials load from the environment and every gap is reported") {
    const Credentials credentials = load_credentials(env);
    CHECK_EQ(credentials.gamma_user, "Aladdin");
    try {
        load_credentials({{"SHIP_ALPHA_API_KEY", "x"}, {"SHIP_GAMMA_USER", ""}});
        CHECK(false);
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        CHECK(message.find("SHIP_BETA_CLIENT_ID") != std::string::npos);
        CHECK(message.find("SHIP_GAMMA_USER") != std::string::npos);
        CHECK(message.find("SHIP_ALPHA_API_KEY") == std::string::npos);
    }
}

TEST_CASE("Basic auth matches the RFC 7617 example") {
    CHECK_EQ(basic_auth_header("Aladdin", Secret("open sesame")), "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==");
    CHECK_EQ(base64("ab"), "YWI=");
    CHECK_THROWS_AS(basic_auth_header("a:b", Secret("x")), std::invalid_argument);
}

TEST_CASE("bearer tokens are cached and refreshed shortly before expiry") {
    long long now = 0;
    int issued = 0;
    TokenCache cache([&] { return Token{"t" + std::to_string(++issued), 600s}; },
                     [&] { return std::chrono::seconds{now}; });
    CHECK_EQ(cache.header(), "Bearer t1");
    now = 500;
    CHECK_EQ(cache.header(), "Bearer t1");
    now = 571;  // within 30 s of expiry at 600
    CHECK_EQ(cache.header(), "Bearer t2");
    CHECK_EQ(cache.fetches(), 2);
}

TEST_CASE("each carrier gets its own authorisation scheme") {
    const Credentials credentials = load_credentials(env);
    TokenCache tokens([] { return Token{"tok", 3600s}; }, [] { return 0s; });
    CHECK_EQ(auth_headers("alpha", credentials, tokens).at("X-Api-Key"), "ak_test_key");
    CHECK_EQ(auth_headers("beta", credentials, tokens).at("Authorization"), "Bearer tok");
    CHECK_EQ(auth_headers("gamma", credentials, tokens).at("Authorization"), "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==");
    CHECK_THROWS_AS(auth_headers("delta", credentials, tokens), std::invalid_argument);
}

TEST_CASE("redact removes secrets and their Base64 forms from logs") {
    const std::string line = "X-Api-Key: ak_test_key, Authorization: Basic " + base64("Aladdin:open sesame");
    const std::string clean = redact(line, {"ak_test_key", "Aladdin:open sesame"});
    CHECK_EQ(clean, "X-Api-Key: ***, Authorization: Basic ***");
    CHECK_EQ(redact("tiny abc here", {"abc"}), "tiny abc here");
}

TEST_CASE("run logs headers without ever showing a secret") {
    std::istringstream in("alpha\nbeta\nwait 3590\nbeta\ngamma\nnone\nstop now\n");
    std::ostringstream out;
    CHECK_EQ(run(in, out), 0);
    const auto text = out.str();
    CHECK(text.find("alpha key = ***") != std::string::npos);
    CHECK(text.find("log: X-Api-Key: ***") != std::string::npos);
    CHECK(text.find("log: Authorization: Bearer tok_1") != std::string::npos);
    CHECK(text.find("log: Authorization: Bearer tok_2") != std::string::npos);
    CHECK(text.find("log: Authorization: Basic ***") != std::string::npos);
    CHECK(text.find("hunter2") == std::string::npos);
    CHECK(text.find("2 token fetch(es)") != std::string::npos);
}
