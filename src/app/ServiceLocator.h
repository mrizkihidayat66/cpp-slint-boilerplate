// ServiceLocator.h — minimal type-indexed dependency-injection container.

#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "services/IService.h"

namespace cpp_slint_boilerplate::app {

/// Owns the application's services. Registration order determines
/// initialization order; shutdown runs in reverse. Not thread-safe by
/// design: all registration happens on the main thread before the event
/// loop runs.
class ServiceLocator {
public:
    ServiceLocator() = default;
    ServiceLocator(const ServiceLocator &) = delete;
    ServiceLocator &operator=(const ServiceLocator &) = delete;
    ~ServiceLocator();

    /// Register `service` under its concrete type `T`. The locator takes
    /// shared ownership. Registering the same type twice is a logic error.
    template <typename T, typename... Args> void registerService(std::shared_ptr<T> service)
    {
        static_assert(std::is_base_of_v<services::IService, T>,
                      "Registered services must implement IService");
        const std::type_index key(typeid(T));
        if (m_services.contains(key)) {
            throw std::logic_error("Service already registered: " + std::string(key.name()));
        }
        m_order.push_back(key);
        m_services.emplace(key, std::move(service));
    }

    /// Construct-and-register convenience.
    template <typename T, typename... Args> std::shared_ptr<T> registerConstructed(Args &&...args)
    {
        auto service = std::make_shared<T>(std::forward<Args>(args)...);
        registerService<T>(service);
        return service;
    }

    /// Resolve a registered service; throws std::logic_error if absent.
    template <typename T> [[nodiscard]] std::shared_ptr<T> get() const
    {
        const std::type_index key(typeid(T));
        const auto it = m_services.find(key);
        if (it == m_services.end()) {
            throw std::logic_error("Service not registered: " + std::string(key.name()));
        }
        return std::static_pointer_cast<T>(it->second);
    }

    /// True when a service is registered under T.
    template <typename T> [[nodiscard]] bool has() const
    {
        return m_services.contains(std::type_index(typeid(T)));
    }

    /// Initialize all services in registration order.
    void initializeAll();

    /// Shut down all services in reverse registration order.
    void shutdownAll();

private:
    std::vector<std::type_index> m_order;
    std::unordered_map<std::type_index, std::shared_ptr<services::IService>> m_services;
};

} // namespace cpp_slint_boilerplate::app
