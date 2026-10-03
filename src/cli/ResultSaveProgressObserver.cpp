// ------------------------------------------------
// ResultSaveProgressObserver.hpp implementation
// ------------------------------------------------

#include "ResultSaveProgressObserver.hpp"

#include <boost/progress.hpp>

namespace cli {
    struct ResultSaveProgressObserver::Impl {
        std::unique_ptr<boost::progress_display> display;
        size_t displayedCount = 0;
    };

    ResultSaveProgressObserver::ResultSaveProgressObserver() : m_impl(std::make_unique<Impl>()) {}

    ResultSaveProgressObserver::~ResultSaveProgressObserver() = default;

    void ResultSaveProgressObserver::onBodyStarted(size_t, size_t rowCount) {
        m_impl->display = std::make_unique<boost::progress_display>(
            static_cast<unsigned long>(rowCount));
        m_impl->displayedCount = 0;
    }

    void ResultSaveProgressObserver::onBodyProgress(size_t,
                                                    size_t completedRowCount,
                                                    size_t) {
        while (m_impl->displayedCount < completedRowCount) {
            ++(*m_impl->display);
            ++m_impl->displayedCount;
        }
    }
}
