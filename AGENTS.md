# AGENTS


This repository is a 6-DoF rocket flight simulator (Prologue). Follow these rules:

- Numerical correctness is paramount.
- Preserve numerical behavior and, within the same supported toolchain, bitwise results unless a numerical change is intentional.
- Do not unintentionally change floating-point operation order, integration behavior, constants, units, coordinate conventions, or tolerances.
- For intentional numerical changes, describe the expected effect and add or update regression tests. Explain when testing is infeasible.
- Prefer clarity over cleverness or micro-optimizations.
- Keep changes small and focused; avoid drive-by formatting or whitespace churn.
- Maintain backward compatibility for the legacy user base unless explicitly requested.
- If numerical behavior might change, call it out and add validation or tests when possible.
- Consult `docs/DYNAMICS.md` and `docs/INPUT.md` before changing physical models, units, coordinate systems, or input semantics.
- Use the repository's C++20 and CMake configuration.
- Run relevant tests with `ctest --test-dir build/test -C Debug --output-on-failure`.
- Treat `deps/` as vendored code; do not modify it unless explicitly required.
- Preserve the documented input formats and command-line behavior unless a breaking change is explicitly requested.
