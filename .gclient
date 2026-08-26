# codem37 - Pinned Upstream Chromium GClient Configuration
# Pinned to exact Chromium milestone release: 134.0.6998.88

solutions = [
  {
    "name": "src",
    "url": "https://chromium.googlesource.com/chromium/src.git@134.0.6998.88",
    "deps_file": "DEPS",
    "managed": False,
    "custom_deps": {},
  },
]
target_os = ["win"]
