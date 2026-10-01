/**
 * @file
 * Day 57 – REST APIs & JSON.
 *
 * Scenario: a *library-book REST API*, simulated in memory. No network is involved: the goal
 * is to understand what each HTTP method means, which status code each outcome deserves, and
 * how JSON request and response bodies are produced and consumed by a resource-oriented API.
 *
 * Deliverables (syllabus):
 * - HTTP methods and their meaning
 * - Status codes
 * - Resource routing
 * - JSON request and response bodies
 */
#pragma once

#include <cctype>
#include <cstddef>
#include <istream>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day57 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"which methods are safe and which are idempotent", "method_semantics"},
    {"parsing a flat JSON object body", "parse_object"},
    {"writing JSON objects and arrays", "to_json"},
    {"routing /books and /books/{id} to handlers", "BookApi::handle"},
    {"choosing the right status code for every outcome", "BookApi::create"},
};

struct MethodInfo {
    bool safe;        // never changes server state (GET, HEAD)
    bool idempotent;  // repeating it has the same effect as doing it once (GET, PUT, DELETE)
};

inline std::optional<MethodInfo> method_semantics(std::string_view method) {
    if (method == "GET" || method == "HEAD") return MethodInfo{true, true};
    if (method == "PUT" || method == "DELETE") return MethodInfo{false, true};
    if (method == "POST" || method == "PATCH") return MethodInfo{false, false};
    return std::nullopt;
}

using Fields = std::map<std::string, std::string>;  // values kept as text; numbers are validated by the handlers

/// Parse `{"key": "text", "n": 3}` – one level, string and integer values only. std::nullopt if malformed.
inline std::optional<Fields> parse_object(std::string_view text) {
    std::size_t i = 0;
    const auto skip = [&] { while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) ++i; };
    const auto read_string = [&]() -> std::optional<std::string> {
        if (i >= text.size() || text[i] != '"') return std::nullopt;
        std::string out;
        for (++i; i < text.size() && text[i] != '"'; ++i) {
            if (text[i] == '\\' && i + 1 < text.size()) ++i;
            out += text[i];
        }
        if (i >= text.size()) return std::nullopt;
        ++i;
        return out;
    };
    Fields fields;
    skip();
    if (i >= text.size() || text[i++] != '{') return std::nullopt;
    skip();
    if (i < text.size() && text[i] == '}') return fields;
    while (true) {
        skip();
        auto key = read_string();
        skip();
        if (!key || i >= text.size() || text[i++] != ':') return std::nullopt;
        skip();
        std::string value;
        if (i < text.size() && text[i] == '"') {
            auto s = read_string();
            if (!s) return std::nullopt;
            value = *s;
        } else {
            const std::size_t start = i;
            if (i < text.size() && text[i] == '-') ++i;
            while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) ++i;
            if (i == start) return std::nullopt;
            value = std::string(text.substr(start, i - start));
        }
        fields[*key] = value;
        skip();
        if (i < text.size() && text[i] == ',') {
            ++i;
            continue;
        }
        if (i < text.size() && text[i] == '}') {
            ++i;
            skip();
            return i == text.size() ? std::optional<Fields>(fields) : std::nullopt;
        }
        return std::nullopt;
    }
}

inline std::string quote(const std::string& text) {
    std::string out = "\"";
    for (const char c : text) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + '"';
}

struct Book {
    int id;
    std::string title;
    std::string author;
    int year;
};

/// `{"id":1,"title":"…","author":"…","year":1990}`
inline std::string to_json(const Book& book) {
    return "{\"id\":" + std::to_string(book.id) + ",\"title\":" + quote(book.title) + ",\"author\":" + quote(book.author) +
           ",\"year\":" + std::to_string(book.year) + "}";
}
inline std::string to_json(const std::vector<Book>& books) {
    std::string out = "[";
    for (std::size_t i = 0; i < books.size(); ++i) out += (i ? "," : "") + to_json(books[i]);
    return out + "]";
}

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
};

struct HttpResponse {
    int status;
    std::string body;
    std::map<std::string, std::string> headers;
};

inline HttpResponse error(int status, const std::string& message) { return {status, "{\"error\":" + quote(message) + "}", {}}; }

/// The resource-oriented API: the URL names the thing, the method says what to do with it.
class BookApi {
public:
    HttpResponse handle(const HttpRequest& request) {
        if (!method_semantics(request.method)) return error(405, "unsupported method");
        if (request.path == "/books") {
            if (request.method == "GET") return list();
            if (request.method == "POST") return create(request.body);
            return error(405, "use GET or POST on /books");
        }
        if (request.path.rfind("/books/", 0) == 0) {
            const auto id = parse_id(request.path.substr(7));
            if (!id) return error(400, "book ids are positive integers");
            if (request.method == "GET") return get(*id);
            if (request.method == "PUT") return replace(*id, request.body);
            if (request.method == "DELETE") return remove(*id);
            return error(405, "use GET, PUT or DELETE on /books/{id}");
        }
        return error(404, "no such resource");
    }

    /// POST creates a new resource: 201 Created with a Location header, 400 for unreadable JSON,
    /// 422 for valid JSON with invalid content, 409 if the same title and author already exist.
    HttpResponse create(const std::string& body) {
        auto book = validate(body);
        if (!book.first) return book.second;
        for (const auto& [id, existing] : books_) {
            if (existing.title == book.first->title && existing.author == book.first->author) {
                return error(409, "this book is already in the catalogue");
            }
        }
        Book created = *book.first;
        created.id = next_id_++;
        books_[created.id] = created;
        return {201, to_json(created), {{"Location", "/books/" + std::to_string(created.id)}}};
    }

private:
    static std::optional<int> parse_id(const std::string& text) {
        if (text.empty() || text.size() > 9) return std::nullopt;
        for (const char c : text) {
            if (!std::isdigit(static_cast<unsigned char>(c))) return std::nullopt;
        }
        const int id = std::stoi(text);
        return id > 0 ? std::optional<int>(id) : std::nullopt;
    }

    std::pair<std::optional<Book>, HttpResponse> validate(const std::string& body) const {
        const auto fields = parse_object(body);
        if (!fields) return {std::nullopt, error(400, "body must be a JSON object")};
        const auto title = fields->find("title");
        const auto author = fields->find("author");
        const auto year = fields->find("year");
        if (title == fields->end() || author == fields->end() || year == fields->end() || title->second.empty()) {
            return {std::nullopt, error(422, "title, author and year are required")};
        }
        int y = 0;
        std::istringstream number(year->second);
        if (!(number >> y) || !(number >> std::ws).eof() || y < 1450 || y > 2100) {
            return {std::nullopt, error(422, "year must be a number 1450-2100")};
        }
        return {Book{0, title->second, author->second, y}, HttpResponse{}};
    }

    HttpResponse list() const {
        std::vector<Book> all;
        for (const auto& [id, book] : books_) all.push_back(book);
        return {200, to_json(all), {}};
    }
    HttpResponse get(int id) const {
        const auto it = books_.find(id);
        return it == books_.end() ? error(404, "book not found") : HttpResponse{200, to_json(it->second), {}};
    }
    /// PUT replaces the whole resource and is idempotent: sending it twice changes nothing more.
    HttpResponse replace(int id, const std::string& body) {
        if (!books_.contains(id)) return error(404, "book not found");
        auto book = validate(body);
        if (!book.first) return book.second;
        book.first->id = id;
        books_[id] = *book.first;
        return {200, to_json(*book.first), {}};
    }
    /// DELETE is idempotent too: the second call finds nothing and says so with 404.
    HttpResponse remove(int id) {
        return books_.erase(id) == 1 ? HttpResponse{204, "", {}} : error(404, "book not found");
    }

    std::map<int, Book> books_;
    int next_id_{1};
};

/// The interactive demo: "METHOD /path [json body]".
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 57 – REST APIs & JSON\nTry: POST /books {\"title\":\"Dune\",\"author\":\"Herbert\",\"year\":1965}\n";
    BookApi api;
    while (auto line = prompt_line(in, out, "request> ")) {
        std::istringstream words(*line);
        HttpRequest request;
        if (!(words >> request.method >> request.path)) break;
        std::getline(words >> std::ws, request.body);
        const HttpResponse response = api.handle(request);
        out << "  " << response.status;
        for (const auto& [name, value] : response.headers) out << "  " << name << ": " << value;
        out << (response.body.empty() ? "" : "  " + response.body) << '\n';
    }
    return 0;
}

}  // namespace cppm::day57
