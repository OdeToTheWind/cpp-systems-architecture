/**
 * @file
 * Day 55 – Hosting C++ Online.
 *
 * Scenario: a *"Word of the Day" web service* for a language school, written so it can be
 * hosted anywhere: as a CGI program behind Apache or nginx today, or behind an embedded HTTP
 * server later. The request comes from CGI environment variables, a router picks the handler,
 * the response is written in CGI format, and settings come from the environment with safe defaults.
 *
 * Deliverables (syllabus):
 * - Request/response handlers
 * - CGI-style environment parsing
 * - Routing
 * - Deployment-ready configuration
 */
#pragma once

#include <cstdlib>
#include <functional>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day55 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a request built from CGI environment variables", "request_from_cgi"},
    {"decoding the query string", "parse_query"},
    {"a router mapping method and path to handlers", "Router"},
    {"responses rendered in CGI format", "render_cgi"},
    {"configuration from the environment with defaults", "load_config"},
};

using Environment = std::map<std::string, std::string>;

/// The process environment as a map – tests pass their own map instead.
inline Environment process_environment(const std::vector<std::string>& names) {
    Environment env;
    for (const auto& name : names) {
        if (const char* value = std::getenv(name.c_str())) env[name] = value;
    }
    return env;
}

struct Request {
    std::string method;
    std::string path;
    std::map<std::string, std::string> query;
};

struct Response {
    int status{200};
    std::string content_type{"text/plain; charset=utf-8"};
    std::string body;
};

/// "word=hello%20world&lang=fr" -> {word: "hello world", lang: "fr"}; '+' also means space.
inline std::map<std::string, std::string> parse_query(const std::string& query) {
    const auto decode = [](const std::string& text) {
        std::string out;
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (text[i] == '+') {
                out += ' ';
            } else if (text[i] == '%' && i + 2 < text.size()) {
                const std::string hex = text.substr(i + 1, 2);
                char* end = nullptr;
                const long value = std::strtol(hex.c_str(), &end, 16);
                if (end == hex.c_str() + 2) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    out += '%';
                }
            } else {
                out += text[i];
            }
        }
        return out;
    };
    std::map<std::string, std::string> result;
    std::istringstream pairs(query);
    std::string pair;
    while (std::getline(pairs, pair, '&')) {
        if (pair.empty()) continue;
        const auto eq = pair.find('=');
        result[decode(pair.substr(0, eq))] = eq == std::string::npos ? "" : decode(pair.substr(eq + 1));
    }
    return result;
}

/// CGI passes the request in REQUEST_METHOD, PATH_INFO and QUERY_STRING.
inline Request request_from_cgi(const Environment& env) {
    const auto get = [&](const std::string& key, const std::string& fallback) {
        const auto it = env.find(key);
        return it == env.end() || it->second.empty() ? fallback : it->second;
    };
    return {get("REQUEST_METHOD", "GET"), get("PATH_INFO", "/"), parse_query(get("QUERY_STRING", ""))};
}

/// Exact (method, path) routing with 404 and 405 answers.
class Router {
public:
    using Handler = std::function<Response(const Request&)>;
    void add(const std::string& method, const std::string& path, Handler handler) {
        routes_[path][method] = std::move(handler);
    }
    Response handle(const Request& request) const {
        const auto path = routes_.find(request.path);
        if (path == routes_.end()) return {404, "text/plain; charset=utf-8", "not found"};
        const auto handler = path->second.find(request.method);
        if (handler == path->second.end()) return {405, "text/plain; charset=utf-8", "method not allowed"};
        try {
            return handler->second(request);
        } catch (const std::exception&) {
            return {500, "text/plain; charset=utf-8", "internal error"};  // never leak details to visitors
        }
    }

private:
    std::map<std::string, std::map<std::string, Handler>> routes_;
};

inline std::string reason_phrase(int status) {
    switch (status) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        default: return "Internal Server Error";
    }
}

/// CGI output: a Status header, a Content-Type header, a blank line, then the body.
inline std::string render_cgi(const Response& response) {
    return "Status: " + std::to_string(response.status) + " " + reason_phrase(response.status) +
           "\r\nContent-Type: " + response.content_type + "\r\nContent-Length: " + std::to_string(response.body.size()) +
           "\r\n\r\n" + response.body;
}

struct Config {
    std::string school_name{"Lingua Lab"};
    std::string default_language{"fr"};
    bool debug{false};
};

/// Read WOTD_* variables; unknown values fall back to safe defaults, invalid ones throw at start-up.
inline Config load_config(const Environment& env) {
    Config config;
    if (auto it = env.find("WOTD_SCHOOL"); it != env.end() && !it->second.empty()) config.school_name = it->second;
    if (auto it = env.find("WOTD_LANGUAGE"); it != env.end()) {
        if (it->second != "fr" && it->second != "es" && it->second != "de") {
            throw std::invalid_argument("WOTD_LANGUAGE must be fr, es or de");
        }
        config.default_language = it->second;
    }
    if (auto it = env.find("WOTD_DEBUG"); it != env.end()) config.debug = it->second == "1" || it->second == "true";
    return config;
}

/// The application: routes for /, /word and /health.
inline Router make_app(const Config& config) {
    static const std::map<std::string, std::vector<std::pair<std::string, std::string>>> words{
        {"fr", {{"bonjour", "hello"}, {"merci", "thank you"}}},
        {"es", {{"hola", "hello"}, {"gracias", "thank you"}}},
        {"de", {{"hallo", "hello"}, {"danke", "thank you"}}},
    };
    Router router;
    router.add("GET", "/", [config](const Request&) {
        return Response{200, "text/html; charset=utf-8", "<h1>" + config.school_name + "</h1><p>Try /word?lang=es&day=1</p>"};
    });
    router.add("GET", "/word", [config](const Request& request) {
        const auto lang_it = request.query.find("lang");
        const std::string lang = lang_it == request.query.end() ? config.default_language : lang_it->second;
        const auto list = words.find(lang);
        if (list == words.end()) return Response{400, "text/plain; charset=utf-8", "unknown language"};
        std::size_t day = 0;
        if (const auto day_it = request.query.find("day"); day_it != request.query.end()) {
            std::istringstream number(day_it->second);
            if (!(number >> day) || !(number >> std::ws).eof() || day_it->second.front() == '-') {
                return Response{400, "text/plain; charset=utf-8", "day must be a non-negative number"};
            }
        }
        const auto& [word, meaning] = list->second[day % list->second.size()];
        return Response{200, "application/json", "{\"word\":\"" + word + "\",\"meaning\":\"" + meaning + "\"}"};
    });
    router.add("GET", "/health", [](const Request&) { return Response{200, "text/plain; charset=utf-8", "ok"}; });
    return router;
}

/// The demo simulates CGI requests: type "METHOD /path?query".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 55 – Hosting C++ Online\n";
    const Router app = make_app(load_config(process_environment({"WOTD_SCHOOL", "WOTD_LANGUAGE", "WOTD_DEBUG"})));
    while (auto line = prompt_line(in, out, "METHOD /path?query> ")) {
        std::istringstream words(*line);
        std::string method;
        std::string target;
        if (!(words >> method >> target)) break;
        const auto question = target.find('?');
        const Environment env{{"REQUEST_METHOD", method},
                              {"PATH_INFO", target.substr(0, question)},
                              {"QUERY_STRING", question == std::string::npos ? "" : target.substr(question + 1)}};
        out << render_cgi(app.handle(request_from_cgi(env))) << "\n\n";
    }
    return 0;
}

}  // namespace cppm::day55
