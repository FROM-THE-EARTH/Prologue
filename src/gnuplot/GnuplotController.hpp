// ------------------------------------------------
// Gnuplotはpopenでプロセスを開いて操作する
// そのGnuplotプロセス操作用のクラス
// ------------------------------------------------

#pragma once

#include <memory>
#include <string>

#include "misc/Platform.hpp"

class GnuplotController {
private:
    // std::unique_ptr のデリータ
    // デストラクタが呼ばれたタイミングでoperator()が実行される
    struct PtrDeleter {
        void operator()(FILE* p) const {
            PCLOSE(p);
            p = nullptr;
        }
    };

    std::unique_ptr<FILE, PtrDeleter> m_ptr = nullptr;

public:
    // 新たにプロセスを開く
    bool open() {
        m_ptr = std::unique_ptr<FILE, PtrDeleter>(POPEN("gnuplot", "w"));
        return m_ptr != nullptr;
    }

    // プロセスを閉じる
    void close() {
        flush();
        m_ptr.release();
    }

    // 現時点までのバッファを書き込み
    void flush() {
        fflush(m_ptr.get());
    }

    // コマンドを入力
    template <class... Args>
    void send(std::string_view command, const Args&... args) {
        fprintf(m_ptr.get(), command.ends_with('\n') ? command.data() : (std::string(command) + '\n').c_str(), args...);
    }
};
