// ServiceLocator.cpp — see ServiceLocator.h.

#include "app/ServiceLocator.h"

namespace cpp_slint_boilerplate::app {

ServiceLocator::~ServiceLocator()
{
    shutdownAll();
}

void ServiceLocator::initializeAll()
{
    for (const auto &key : m_order) {
        m_services.at(key)->initialize();
    }
}

void ServiceLocator::shutdownAll()
{
    for (auto it = m_order.rbegin(); it != m_order.rend(); ++it) {
        const auto found = m_services.find(*it);
        if (found != m_services.end()) {
            found->second->shutdown();
        }
    }
    m_order.clear();
    m_services.clear();
}

} // namespace cpp_slint_boilerplate::app
