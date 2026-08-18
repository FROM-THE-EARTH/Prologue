---
name: prologue-release
description: Prepare and verify Prologue releases. Use for version bumps, release readiness, tags, and GitHub Releases; not for ordinary builds or development.
---

# Prologue Release

Follow `AGENTS.md`; treat `docs/DEVELOPMENT.md`, `.github/workflows/ci.yml`, and `.github/workflows/release.yml` as the current release process.

1. Inspect the working tree, branch/upstream, tags, CI, `README.md`, and `src/main.cpp` before changing anything.
2. Record the unchanged revision as `BASE_COMMIT`; record `RELEASE_COMMIT` only after release metadata is finalized.
3. Keep the version, README date, tag, and release notes consistent with repository conventions.
4. Run the relevant CMake build and tests, and distinguish local, CI-verified, and unverified platforms.
5. Before publication, verify a clean tree, `RELEASE_COMMIT == HEAD`, passing required checks, and a new matching tag.
6. Never push commits or tags, create a GitHub Release, or upload assets unless the user explicitly requests publication.
7. For publication, push only the intended tag and let `release.yml` create the Release and three platform archives; do not duplicate them manually.
8. After publication, verify the remote tag target, Actions result, release metadata/notes, and all expected assets.
9. Stop on failing checks, inconsistent versions, an existing tag, an ambiguous commit, or missing artifacts; report the exact blocker.
10. Treat release candidates as blocked unless the workflow correctly marks them as prereleases.
