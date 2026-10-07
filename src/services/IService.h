// IService.h — service contract used by the ServiceLocator.

#pragma once

namespace cpp_slint_boilerplate::services {

/// Lifecycle contract for all registered services. Services are initialized
/// in registration order and shut down in reverse order.
class IService
{
public:
    virtual ~IService() = default;

    /// Called once at startup, after construction and before the UI runs.
    virtual void initialize() = 0;

    /// Called once at shutdown, before the service is destroyed.
    virtual void shutdown() = 0;
};

} // namespace cpp_slint_boilerplate::services
