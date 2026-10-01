/**
 * @file
 * Day 60 – API Authentication.
 *
 * Scenario: a *shipping-rate aggregator* that queries three carriers, each with a different
 * authentication scheme: an API key header, a short-lived Bearer token that must be refreshed,
 * and HTTP Basic auth. Credentials come from environment variables, are wrapped so they cannot
 * be printed by accident, and are redacted from every log line.
 *
 * Deliverables (syllabus):
 * - API keys
 * - Bearer tokens
 * - Basic auth with Base64
 * - Secrets from environment variables
 * - Redaction
 */
#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day60 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a secret type that refuses to print itself", "Secret"},
    {"loading every credential from the environment at once", "load_credentials"},
    {"Base64 for HTTP Basic auth", "basic_auth_header"},
    {"a Bearer token cache that refreshes before expiry", "TokenCache::header"},
    {"one Authorization header per carrier scheme", "auth_headers"},
    {"removing secrets from log lines", "redact"},
};

/// Holds a credential. Streaming it prints *** – the real value needs an explicit reveal().
class Secret {
public:
    explicit Secret(std::string value) : value_(std::move(value)) {}
    const std::string& reveal() const { return value_; }
    friend std::ostream& operator<<(std::ostream& out, const Secret&) { return out << "***"; }

private:
    std::string value_;
};

struct Credentials {
    Secret alpha_api_key;
    Secret beta_client_id;
    Secret beta_client_secret;
    std::string gamma_user;
    Secret gamma_password;
};

/// Read SHIP_* variables from @p env (the real program passes the process environment). Every
/// missing variable is reported in one message, so a deployment is fixed in one go.
inline Credentials load_credentials(const std::map<std::string, std::string>& env) {
    static const std::vector<std::string> required{"SHIP_ALPHA_API_KEY", "SHIP_BETA_CLIENT_ID", "SHIP_BETA_CLIENT_SECRET",
                                                   "SHIP_GAMMA_USER", "SHIP_GAMMA_PASSWORD"};
    std::string missing;
    for (const auto& name : required) {
        const auto it = env.find(name);
        if (it == env.end() || it->second.empty()) missing += (missing.empty() ? "" : ", ") + name;
    }
    if (!missing.empty()) throw std::runtime_error("missing environment variables: " + missing);
    return {Secret(env.at("SHIP_ALPHA_API_KEY")), Secret(env.at("SHIP_BETA_CLIENT_ID")),
            Secret(env.at("SHIP_BETA_CLIENT_SECRET")), env.at("SHIP_GAMMA_USER"), Secret(env.at("SHIP_GAMMA_PASSWORD"))};
}

inline std::string base64(std::string_view data) {
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    for (std::size_t i = 0; i < data.size(); i += 3) {
        unsigned triple = static_cast<unsigned>(static_cast<unsigned char>(data[i])) << 16;
        if (i + 1 < data.size()) triple |= static_cast<unsigned>(static_cast<unsigned char>(data[i + 1])) << 8;
        if (i + 2 < data.size()) triple |= static_cast<unsigned>(static_cast<unsigned char>(data[i + 2]));
        out += alphabet[(triple >> 18) & 63];
        out += alphabet[(triple >> 12) & 63];
        out += i + 1 < data.size() ? alphabet[(triple >> 6) & 63] : '=';
        out += i + 2 < data.size() ? alphabet[triple & 63] : '=';
    }
    return out;
}

/// "Basic " + base64("user:password"). Base64 is an *encoding*, not encryption – only use it over TLS.
inline std::string basic_auth_header(const std::string& user, const Secret& password) {
    if (user.find(':') != std::string::npos) throw std::invalid_argument("Basic auth user names cannot contain ':'");
    return "Basic " + base64(user + ":" + password.reveal());
}

/// Issued bearer tokens; the fetch function stands in for a POST to the carrier's /oauth/token.
struct Token {
    std::string value;
    std::chrono::seconds lifetime;
};

class TokenCache {
public:
    using Clock = std::function<std::chrono::seconds()>;  // seconds since some epoch, injectable for tests
    using Fetch = std::function<Token()>;

    TokenCache(Fetch fetch, Clock clock) : fetch_(std::move(fetch)), clock_(std::move(clock)) {}

    /// "Bearer <token>", fetching a new token when none is cached or it expires within 30 seconds.
    std::string header() {
        const auto now = clock_();
        if (!token_ || now + std::chrono::seconds{30} >= expires_at_) {
            const Token fresh = fetch_();
            token_ = fresh.value;
            expires_at_ = now + fresh.lifetime;
            ++fetches_;
        }
        return "Bearer " + *token_;
    }
    int fetches() const { return fetches_; }

private:
    Fetch fetch_;
    Clock clock_;
    std::optional<std::string> token_;
    std::chrono::seconds expires_at_{0};
    int fetches_{0};
};

/// The header each carrier expects.
inline std::map<std::string, std::string> auth_headers(const std::string& carrier, const Credentials& credentials,
                                                       TokenCache& beta_tokens) {
    if (carrier == "alpha") return {{"X-Api-Key", credentials.alpha_api_key.reveal()}};
    if (carrier == "beta") return {{"Authorization", beta_tokens.header()}};
    if (carrier == "gamma") return {{"Authorization", basic_auth_header(credentials.gamma_user, credentials.gamma_password)}};
    throw std::invalid_argument("unknown carrier " + carrier);
}

/// Replace every occurrence of each secret (and its Base64 form) in a log line with ***.
inline std::string redact(std::string line, const std::vector<std::string>& secrets) {
    for (const auto& secret : secrets) {
        if (secret.size() < 4) continue;  // too short to search for safely
        for (const std::string& form : {secret, base64(secret)}) {
            for (auto at = line.find(form); at != std::string::npos; at = line.find(form, at + 3)) {
                line.replace(at, form.size(), "***");
            }
        }
    }
    return line;
}

/// The interactive demo: type a carrier name; the headers it would send are logged, redacted.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 60 – API Authentication\n";
    const std::map<std::string, std::string> env{{"SHIP_ALPHA_API_KEY", "ak_9f8e7d6c"},
                                                 {"SHIP_BETA_CLIENT_ID", "beta-client"},
                                                 {"SHIP_BETA_CLIENT_SECRET", "bs_s3cr3t"},
                                                 {"SHIP_GAMMA_USER", "warehouse"},
                                                 {"SHIP_GAMMA_PASSWORD", "hunter2!"}};  // demo values only
    const Credentials credentials = load_credentials(env);
    long long now = 0;
    int issued = 0;
    TokenCache tokens([&] { return Token{"tok_" + std::to_string(++issued), std::chrono::seconds{3600}}; },
                      [&] { return std::chrono::seconds{now}; });
    const std::vector<std::string> secrets{"ak_9f8e7d6c", "bs_s3cr3t", "hunter2!", "warehouse:hunter2!"};
    out << "loaded credentials, alpha key = " << credentials.alpha_api_key << '\n';
    while (auto line = prompt_line(in, out, "carrier (alpha|beta|gamma) or 'wait <s>'> ")) {
        std::istringstream words(*line);
        std::string word;
        if (!(words >> word)) break;
        if (word == "wait") {
            long long seconds = 0;
            words >> seconds;
            now += seconds;
            continue;
        }
        try {
            for (const auto& [name, value] : auth_headers(word, credentials, tokens)) {
                out << "  log: " << redact(name + ": " + value, secrets) << '\n';
            }
        } catch (const std::invalid_argument& error) {
            out << "  " << error.what() << '\n';
        }
    }
    out << tokens.fetches() << " token fetch(es)\n";
    return 0;
}

}  // namespace cppm::day60
