// The plugin registry: plugins add themselves; the host looks them up by name.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "plugin_api.hpp"

namespace cppm::day87 {

class PluginRegistry {
public:
    /// One registry per program. A function-local static is created on first use, which avoids
    /// the "static initialisation order fiasco" when plugins register during start-up.
    static PluginRegistry& instance() {
        static PluginRegistry registry;
        return registry;
    }

    /// Accept a plugin if the host can run it: same major version (breaking changes), and the
    /// host's minor version at least the plugin's (it may use newer features). Returns whether accepted.
    bool add(PluginInfo info) {
        const auto& need = info.requires_api;
        std::string problem;
        if (need.major != HOST_API.major) {
            problem = "needs API " + std::to_string(need.major) + ".x";
        } else if (need.minor > HOST_API.minor) {
            problem = "needs API " + std::to_string(need.major) + "." + std::to_string(need.minor) + " or newer";
        } else if (plugins_.contains(info.name)) {
            problem = "duplicate name";
        }
        if (!problem.empty()) {
            rejected_[info.name] = problem;
            return false;
        }
        plugins_.emplace(info.name, std::move(info));
        return true;
    }

    std::unique_ptr<Filter> create(const std::string& name, const Options& options) const {
        const auto it = plugins_.find(name);
        if (it == plugins_.end()) {
            const auto why = rejected_.find(name);
            throw std::invalid_argument("no plugin '" + name + "'" + (why == rejected_.end() ? "" : " (rejected: " + why->second + ")"));
        }
        return it->second.create(options);
    }
    std::vector<const PluginInfo*> list() const {
        std::vector<const PluginInfo*> out;
        for (const auto& [name, info] : plugins_) out.push_back(&info);
        return out;
    }
    const std::map<std::string, std::string>& rejected() const { return rejected_; }

private:
    PluginRegistry() = default;
    std::map<std::string, PluginInfo> plugins_;
    std::map<std::string, std::string> rejected_;
};

/// Self-registration helper: `inline const bool registered = register_plugin({...});` in a plugin
/// header runs add() during static initialisation of any program that includes the header.
inline bool register_plugin(PluginInfo info) { return PluginRegistry::instance().add(std::move(info)); }

}  // namespace cppm::day87
