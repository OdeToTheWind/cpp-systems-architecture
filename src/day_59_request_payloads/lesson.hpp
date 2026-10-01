/**
 * @file
 * Day 59 – Query Parameters, Headers & Payloads.
 *
 * Scenario: a *hotel-search API client*. What matters is exactly what goes on the wire, so every
 * request is first *prepared* offline – URL with percent-encoded query, headers, form or JSON
 * body with the right Content-Type and Content-Length – and can be inspected byte by byte before
 * anything would be sent.
 *
 * Deliverables (syllabus):
 * - Percent-encoding
 * - Query strings
 * - Custom headers
 * - Form and JSON request bodies
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>
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

namespace cppm::day59 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"RFC 3986 percent-encoding", "percent_encode"},
    {"query strings with repeated keys", "query_string"},
    {"case-insensitive custom headers", "Headers"},
    {"a form-encoded body", "RequestBuilder::form"},
    {"a JSON body with content headers set automatically", "RequestBuilder::json"},
    {"the prepared request as wire text", "PreparedRequest::wire"},
};

/// Leave A–Z a–z 0–9 - . _ ~ unchanged and encode every other byte as %XX (upper-case hex).
/// In form bodies a space becomes '+' instead of %20.
inline std::string percent_encode(std::string_view text, bool form = false) {
    std::string out;
    for (const char c : text) {
        const auto u = static_cast<unsigned char>(c);
        if (std::isalnum(u) || c == '-' || c == '.' || c == '_' || c == '~') {
            out += c;
        } else if (form && c == ' ') {
            out += '+';
        } else {
            char hex[4];
            std::snprintf(hex, sizeof hex, "%%%02X", static_cast<unsigned>(u));
            out += hex;
        }
    }
    return out;
}

using Params = std::vector<std::pair<std::string, std::string>>;  // order kept, repeats allowed

inline std::string query_string(const Params& params, bool form = false) {
    std::string out;
    for (const auto& [key, value] : params) {
        out += (out.empty() ? "" : "&") + percent_encode(key, form) + "=" + percent_encode(value, form);
    }
    return out;
}

/// Header names compare case-insensitively; setting a header again replaces it.
class Headers {
public:
    void set(const std::string& name, std::string value) {
        if (name.find_first_of("\r\n:") != std::string::npos || value.find_first_of("\r\n") != std::string::npos) {
            throw std::invalid_argument("header names and values must not contain line breaks");
        }
        for (auto& [existing, current] : items_) {
            if (lower(existing) == lower(name)) {
                current = std::move(value);
                return;
            }
        }
        items_.emplace_back(name, std::move(value));
    }
    std::optional<std::string> get(const std::string& name) const {
        for (const auto& [existing, value] : items_) {
            if (lower(existing) == lower(name)) return value;
        }
        return std::nullopt;
    }
    const std::vector<std::pair<std::string, std::string>>& items() const { return items_; }

private:
    static std::string lower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text;
    }
    std::vector<std::pair<std::string, std::string>> items_;
};

struct PreparedRequest {
    std::string method;
    std::string host;
    std::string target;  // path plus query
    Headers headers;
    std::string body;

    /// The exact bytes an HTTP/1.1 client would send. Secrets can be masked for logs.
    std::string wire(bool mask_secrets = false) const {
        std::string out = method + " " + target + " HTTP/1.1\r\nHost: " + host + "\r\n";
        for (const auto& [name, value] : headers.items()) {
            const bool secret = name == "Authorization" || name == "X-Api-Key";
            out += name + ": " + (mask_secrets && secret ? "***" : value) + "\r\n";
        }
        return out + "\r\n" + body;
    }
};

class RequestBuilder {
public:
    RequestBuilder(std::string method, std::string host, std::string path)
        : method_(std::move(method)), host_(std::move(host)), path_(std::move(path)) {}
    RequestBuilder& param(const std::string& key, const std::string& value) {
        params_.emplace_back(key, value);
        return *this;
    }
    RequestBuilder& header(const std::string& name, const std::string& value) {
        headers_.set(name, value);
        return *this;
    }
    /// application/x-www-form-urlencoded: like a query string, with '+' for spaces.
    RequestBuilder& form(const Params& fields) {
        body_ = query_string(fields, true);
        headers_.set("Content-Type", "application/x-www-form-urlencoded");
        return *this;
    }
    /// A JSON body; the caller passes serialised JSON text.
    RequestBuilder& json(std::string body) {
        body_ = std::move(body);
        headers_.set("Content-Type", "application/json");
        return *this;
    }
    PreparedRequest prepare() const {
        PreparedRequest request{method_, host_, path_, headers_, body_};
        if (!params_.empty()) request.target += "?" + query_string(params_);
        if (!body_.empty()) request.headers.set("Content-Length", std::to_string(body_.size()));
        if (!request.headers.get("Accept")) request.headers.set("Accept", "application/json");
        return request;
    }

private:
    std::string method_;
    std::string host_;
    std::string path_;
    Params params_;
    Headers headers_;
    std::string body_;
};

/// Hotel search: GET with filters as query parameters.
inline PreparedRequest search_request(const std::string& city, const std::string& check_in, int guests,
                                      const std::vector<std::string>& amenities) {
    RequestBuilder builder("GET", "api.hotels.example", "/v2/search");
    builder.param("city", city).param("check_in", check_in).param("guests", std::to_string(guests));
    for (const auto& amenity : amenities) builder.param("amenity", amenity);  // repeated key
    return builder.header("X-Client", "trip-planner/2.1").prepare();
}

/// The interactive demo: "city check_in guests [amenity…]" -> the prepared request, secrets masked.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 59 – Query Parameters, Headers & Payloads\n";
    while (auto line = prompt_line(in, out, "city check_in guests [amenities]> ")) {
        std::istringstream words(*line);
        std::string city;
        std::string check_in;
        int guests = 0;
        if (!(words >> city >> check_in >> guests)) break;
        std::vector<std::string> amenities;
        for (std::string a; words >> a;) amenities.push_back(a);
        std::replace(city.begin(), city.end(), '_', ' ');  // type São_Paulo for "São Paulo"
        PreparedRequest request = search_request(city, check_in, guests, amenities);
        request.headers.set("X-Api-Key", "sk_live_123456");
        out << request.wire(true) << "\n---\n";
    }
    const auto booking = RequestBuilder("POST", "api.hotels.example", "/v2/bookings")
                             .json(R"({"hotel":"H42","nights":2})")
                             .prepare();
    out << booking.wire() << '\n';
    return 0;
}

}  // namespace cppm::day59
