# codem37 - GClient Configuration Template
# Explicitly excludes internal repositories and Google-internal test dependencies.

solutions = [
  {
    "name": "src",
    "url": "https://chromium.googlesource.com/chromium/src.git",
    "deps_file": "DEPS",
    "managed": False,
    "custom_deps": {
      # Exclude internal / non-public components
      "src/chrome/browser/internal": None,
      "src/chrome/installer/mac/internal": None,
      "src/internal": None,
    },
    "custom_vars": {
      "checkout_pgo_profiles": True,
      "checkout_telemetry_dependencies": False,
      "checkout_clang_tidy": False,
    },
  },
]
target_os = ["linux", "win"]
