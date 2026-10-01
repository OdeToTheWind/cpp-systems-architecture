/**
 * @file
 * Day 39 – Inheritance.
 *
 * Scenario: an *electric-vehicle charging network*. A base `Charger` defines how a charging
 * session is billed; AC posts, DC fast chargers and solar-canopy chargers specialise it, and
 * capability interfaces (remote reporting, maintenance) are mixed in with multiple inheritance.
 *
 * Deliverables (syllabus):
 * - Base and derived classes
 * - virtual and override
 * - final
 * - Virtual destructors
 * - Multiple inheritance of interfaces
 */
#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cppm/lesson.hpp"

namespace cppm::day39 {

inline constexpr Deliverable DELIVERABLES[] = {
    {"a base class with a non-virtual template method", "Charger::session_cost_cents"},
    {"virtual hooks overridden in derived classes", "AcCharger"},
    {"a final class that cannot be derived from further", "DcFastCharger"},
    {"multiple inheritance of pure interfaces", "SolarCanopyCharger"},
    {"virtual destructors observed through a base pointer", "Charger::~Charger"},
    {"asking for a capability at run time", "maintenance_due"},
};

/// Counts destructor calls so the effect of a virtual destructor can be observed.
struct DestructionLog {
    static inline int base = 0;
    static inline int derived = 0;
};

class Charger {
public:
    Charger(std::string id, double power_kw) : id_(std::move(id)), power_kw_(power_kw) {
        if (power_kw <= 0) {
            throw std::invalid_argument("power must be positive");
        }
    }
    /// Virtual: `delete charger_ptr` (or unique_ptr<Charger>) runs the derived destructor first.
    virtual ~Charger() { ++DestructionLog::base; }

    const std::string& id() const { return id_; }
    double power_kw() const { return power_kw_; }
    virtual std::string kind() const { return "generic charger"; }

    /// Template method: the billing *procedure* is fixed here; derived classes supply the details.
    long long session_cost_cents(double kwh, int minutes) const {
        if (kwh < 0 || minutes < 0) {
            throw std::invalid_argument("energy and time must not be negative");
        }
        const double max_kwh = power_kw_ * minutes / 60.0;
        const double delivered = kwh < max_kwh ? kwh : max_kwh;  // physics caps what the car can receive
        return static_cast<long long>(delivered * price_per_kwh_cents()) + idle_fee_cents(minutes);
    }

protected:
    virtual double price_per_kwh_cents() const { return 40.0; }
    virtual long long idle_fee_cents(int /*minutes*/) const { return 0; }

private:
    std::string id_;
    double power_kw_;
};

class AcCharger : public Charger {
public:
    explicit AcCharger(std::string id) : Charger(std::move(id), 11.0) {}
    ~AcCharger() override { ++DestructionLog::derived; }
    std::string kind() const override { return "AC 11 kW"; }

protected:
    double price_per_kwh_cents() const override { return 35.0; }
};

/// `final`: fast chargers are a leaf of the hierarchy; the compiler rejects any further derivation.
class DcFastCharger final : public Charger {
public:
    explicit DcFastCharger(std::string id) : Charger(std::move(id), 150.0) {}
    ~DcFastCharger() override { ++DestructionLog::derived; }
    std::string kind() const override { return "DC 150 kW"; }

protected:
    double price_per_kwh_cents() const override { return 59.0; }
    /// Blocking a fast charger is expensive: 0.40 per minute after the first 45 minutes.
    long long idle_fee_cents(int minutes) const override { return minutes > 45 ? 40LL * (minutes - 45) : 0; }
};

/// Interface: can send a status report to the operator's back office.
class Reportable {
public:
    virtual ~Reportable() = default;
    virtual std::string status_report() const = 0;
};

/// Interface: needs periodic maintenance.
class Maintainable {
public:
    virtual ~Maintainable() = default;
    virtual int days_until_service() const = 0;
};

/// One implementation inherited (AcCharger) plus two interfaces – the safe form of multiple inheritance.
class SolarCanopyCharger final : public AcCharger, public Reportable, public Maintainable {
public:
    SolarCanopyCharger(std::string id, double solar_share, int days_to_service)
        : AcCharger(std::move(id)), solar_share_(solar_share), days_to_service_(days_to_service) {}
    std::string kind() const override { return "AC 11 kW + solar canopy"; }
    std::string status_report() const override {
        return id() + ": " + std::to_string(static_cast<int>(solar_share_ * 100)) + "% solar today";
    }
    int days_until_service() const override { return days_to_service_; }

protected:
    /// Solar energy is cheaper, so the price blends with the AC price by the solar share.
    double price_per_kwh_cents() const override {
        return AcCharger::price_per_kwh_cents() * (1.0 - solar_share_) + 10.0 * solar_share_;
    }

private:
    double solar_share_;
    int days_to_service_;
};

/// Ask a Charger whether it also implements Maintainable (a cross-cast between unrelated bases).
inline bool maintenance_due(const Charger& charger, int within_days) {
    const auto* maintainable = dynamic_cast<const Maintainable*>(&charger);
    return maintainable != nullptr && maintainable->days_until_service() <= within_days;
}

inline std::vector<std::unique_ptr<Charger>> sample_site() {
    std::vector<std::unique_ptr<Charger>> site;
    site.push_back(std::make_unique<AcCharger>("AC-1"));
    site.push_back(std::make_unique<DcFastCharger>("DC-1"));
    site.push_back(std::make_unique<SolarCanopyCharger>("SOL-1", 0.5, 3));
    return site;
}

/// The interactive demo: "kwh minutes" – price the session on every charger at the site.
inline int run(std::istream& in, std::ostream& out) {
    out << "Day 39 – Inheritance\n";
    const auto site = sample_site();
    while (auto line = prompt_line(in, out, "kWh minutes> ")) {
        std::istringstream words(*line);
        double kwh = 0;
        int minutes = 0;
        if (!(words >> kwh >> minutes)) {
            break;
        }
        for (const auto& charger : site) {
            out << "  " << charger->id() << " (" << charger->kind() << "): " << charger->session_cost_cents(kwh, minutes)
                << " cents\n";
        }
    }
    for (const auto& charger : site) {
        if (const auto* reporter = dynamic_cast<const Reportable*>(charger.get())) {
            out << reporter->status_report() << '\n';
        }
        if (maintenance_due(*charger, 7)) {
            out << charger->id() << " needs service this week\n";
        }
    }
    return 0;
}

}  // namespace cppm::day39
