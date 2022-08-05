#pragma once

#include <memory>
#include <string>

#include "misc/Platform.hpp"

class GnuplotController {
private:
    struct PtrDeleter {
        void operator()(FILE* p) const {
            PCLOSE(p);
            p = nullptr;
        }
    };

    std::unique_ptr<FILE, PtrDeleter> m_ptr = nullptr;

public:
    bool open() {
        m_ptr = std::unique_ptr<FILE, PtrDeleter>(POPEN("gnuplot", "w"));
        return m_ptr != nullptr;
    }

    void close() {
        flush();
        m_ptr.release();
    }

    void flush() {
        fflush(m_ptr.get());
    }

    template <class... Args>
    void send(std::string_view command, const Args&... args) {
        fprintf(m_ptr.get(), command.ends_with('\n') ? command.data() : (std::string(command) + '\n').c_str(), args...);
    }
};
