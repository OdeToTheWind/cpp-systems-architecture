/**
 * @file
 * A uniquely named scratch directory that deletes itself – the C++ counterpart of pytest's
 * `tmp_path`. Tests and demos that touch the file system work only inside one of these, so
 * they can never damage real files.
 */
#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

namespace cppm {

class TempDir {
  public:
    explicit TempDir(std::string_view prefix = "cppm") {
        static std::atomic<unsigned> counter{0};
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                (std::string(prefix) + "-" + std::to_string(stamp) + "-" + std::to_string(counter++));
        std::filesystem::create_directories(path_);
    }
    ~TempDir() {
        std::error_code ignored;  // a destructor must not throw
        std::filesystem::remove_all(path_, ignored);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    const std::filesystem::path& path() const { return path_; }

    /// Create (or overwrite) @p relative with @p content, creating parent folders as needed.
    std::filesystem::path write(const std::filesystem::path& relative, std::string_view content) const {
        const auto target = path_ / relative;
        std::filesystem::create_directories(target.parent_path());
        std::ofstream(target, std::ios::binary) << content;
        return target;
    }

  private:
    std::filesystem::path path_;
};

}  // namespace cppm
