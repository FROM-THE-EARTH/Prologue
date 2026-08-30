// ------------------------------------------------
// ProgressObserver.hppの実装
// ------------------------------------------------

#include "ProgressObserver.hpp"

#include <boost/progress.hpp>
#include <memory>

namespace cli {
    struct ProgressObserver::Impl {
        std::unique_ptr<boost::progress_display> display;
        size_t displayedCount = 0;
    };

    ProgressObserver::ProgressObserver() : m_impl(std::make_unique<Impl>()) {}

    ProgressObserver::~ProgressObserver() = default;

    void ProgressObserver::onStarted(size_t totalSimulationCount) {
        m_impl->display = std::make_unique<boost::progress_display>(
            static_cast<unsigned long>(totalSimulationCount));
        m_impl->displayedCount = 0;
    }

    void ProgressObserver::onProgress(size_t completedSimulationCount, size_t) {
        while (m_impl->displayedCount < completedSimulationCount) {
            ++(*m_impl->display);
            ++m_impl->displayedCount;
        }
    }
}
