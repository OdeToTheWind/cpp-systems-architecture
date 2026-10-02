/**
 * @file
 * Day 35 – Event Listeners & Callbacks.
 *
 * Scenario: a *smart-home hub*. Devices publish events – the doorbell rang, motion in the
 * hallway, the smoke alarm went off – and any number of independent listeners react, without
 * the devices knowing who is listening. Listeners can unsubscribe safely, even while an event
 * is being delivered.
 *
 * Deliverables (syllabus):
 * - Observer pattern
 * - Subscription handles
 * - Unsubscribing safely
 * - std::function listeners
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day35 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"an event bus: the subject of the observer pattern", "EventBus"},
    {"std::function listeners for any callable", "EventBus::Listener"},
    {"subscribe returns a handle", "EventBus::subscribe"},
    {"unsubscribing safely, even during delivery", "EventBus::unsubscribe"},
    {"an RAII subscription that unsubscribes itself", "ScopedSubscription"},
};

/// Something that happened in the house.
struct Event {
    std::string topic;   // "doorbell", "motion", "smoke"
    std::string detail;  // "front door", "hallway", …
};

class EventBus {
  public:
    using Listener = std::function<void(const Event&)>;
    using Handle = std::size_t;

    /// Register @p listener for @p topic ("*" = every topic). Keep the handle to unsubscribe later.
    Handle subscribe(const std::string& topic, Listener listener) {
        const Handle handle = next_handle_++;
        subscriptions_.push_back({handle, topic, std::move(listener), true});
        return handle;
    }

    /// Safe at any time. During delivery the entry is only marked inactive and removed afterwards,
    /// so the loop that is delivering the event never sees its vector change under it.
    bool unsubscribe(Handle handle) {
        for (auto& s : subscriptions_) {
            if (s.handle == handle && s.active) {
                s.active = false;
                if (delivering_ == 0) {
                    remove_inactive();
                }
                return true;
            }
        }
        return false;
    }

    /// Deliver @p event to every active listener of its topic (and "*"), in subscription order.
    int publish(const Event& event) {
        ++delivering_;
        int delivered = 0;
        const std::size_t count = subscriptions_.size();  // listeners added during delivery wait for the next event
        for (std::size_t i = 0; i < count; ++i) {
            if (subscriptions_[i].active &&
                (subscriptions_[i].topic == event.topic || subscriptions_[i].topic == "*")) {
                const Listener listener = subscriptions_[i].listener;  // copy: the listener may unsubscribe itself
                listener(event);
                ++delivered;
            }
        }
        if (--delivering_ == 0) {
            remove_inactive();
        }
        return delivered;
    }

    std::size_t listener_count() const {
        return static_cast<std::size_t>(
            std::count_if(subscriptions_.begin(), subscriptions_.end(), [](const auto& s) { return s.active; }));
    }

  private:
    struct Subscription {
        Handle handle;
        std::string topic;
        Listener listener;
        bool active;
    };
    void remove_inactive() {
        std::erase_if(subscriptions_, [](const Subscription& s) { return !s.active; });
    }

    std::vector<Subscription> subscriptions_;
    Handle next_handle_{1};
    int delivering_{0};
};

/// Unsubscribes in its destructor, so a listener can never outlive the object it captures.
class ScopedSubscription {
  public:
    ScopedSubscription(EventBus& bus, const std::string& topic, EventBus::Listener listener)
        : bus_(&bus), handle_(bus.subscribe(topic, std::move(listener))) {}
    ~ScopedSubscription() { bus_->unsubscribe(handle_); }
    ScopedSubscription(const ScopedSubscription&) = delete;
    ScopedSubscription& operator=(const ScopedSubscription&) = delete;

  private:
    EventBus* bus_;
    EventBus::Handle handle_;
};

/// The interactive demo: type "topic detail" events; the hub's listeners react.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 35 – Event Listeners & Callbacks\n";
    EventBus bus;
    std::map<std::string, int> counts;
    bus.subscribe("*", [&](const Event& e) { ++counts[e.topic]; });
    bus.subscribe("doorbell", [&](const Event& e) { out << "  phone: someone is at the " << e.detail << '\n'; });
    bus.subscribe("smoke",
                  [&](const Event& e) { out << "  siren on, calling the fire service (" << e.detail << ")\n"; });
    EventBus::Handle lights = 0;
    lights = bus.subscribe("motion", [&](const Event& e) {
        out << "  lights on in the " << e.detail << '\n';
        if (e.detail == "garden") {
            bus.unsubscribe(lights);  // safe even though we are inside publish()
            out << "  (garden sensor disabled after first trigger)\n";
        }
    });
    while (auto line = prompt_line(in, out, "topic detail> ")) {
        std::istringstream words(*line);
        Event event;
        if (!(words >> event.topic)) {
            break;
        }
        std::getline(words >> std::ws, event.detail);
        out << "  -> " << bus.publish(event) << " listener(s)\n";
    }
    for (const auto& [topic, count] : counts) {
        out << topic << ": " << count << " event(s)\n";
    }
    return 0;
}

}  // namespace cppm::day35
