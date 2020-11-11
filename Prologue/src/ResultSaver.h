#pragma once

struct SolvedResult;

namespace ResultSaver {

	void OpenFile(const char* filepath, size_t rows);

	void WriteLine(const SolvedResult& result, size_t rocketIndex);

	void Close();
}