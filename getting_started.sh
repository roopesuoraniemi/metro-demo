#!/usr/bin/env bash
set -euo pipefail

echo "[1/6] Updating apt package lists..."
sudo apt update

echo "[2/6] Installing build tools and libraries..."
sudo apt install -y build-essential pkg-config git cmake libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev

# Find usable compiler binaries even if /usr/bin/g++ symlink is broken.
find_compiler() {
  local primary="$1"
  shift
  if command -v "$primary" >/dev/null 2>&1; then
    command -v "$primary"
    return 0
  fi
  for c in "$@"; do
    if command -v "$c" >/dev/null 2>&1; then
      command -v "$c"
      return 0
    fi
  done
  return 1
}

if ! CC_BIN="$(find_compiler gcc gcc-15 gcc-14 gcc-13 gcc-12 gcc-11 x86_64-linux-gnu-gcc)"; then
  echo "No gcc binary found. Installing gcc..."
  sudo apt install -y gcc
  CC_BIN="$(find_compiler gcc gcc-15 gcc-14 gcc-13 gcc-12 gcc-11 x86_64-linux-gnu-gcc)"
fi

if ! CXX_BIN="$(find_compiler g++ g++-15 g++-14 g++-13 g++-12 g++-11 x86_64-linux-gnu-g++)"; then
  echo "No g++ binary found. Installing g++..."
  sudo apt install -y g++
  CXX_BIN="$(find_compiler g++ g++-15 g++-14 g++-13 g++-12 g++-11 x86_64-linux-gnu-g++)"
fi

echo "Using CC=$CC_BIN"
echo "Using CXX=$CXX_BIN"

if pkg-config --exists raylib; then
  echo "[3/6] raylib found via pkg-config."
elif [ -f /usr/local/lib/libraylib.a ] || [ -f /usr/local/lib/libraylib.so ]; then
  echo "[3/6] raylib found in /usr/local/lib."
else
  echo "[3/6] Building raylib from source..."
  TMP_DIR="$(mktemp -d)"
  git clone --depth 1 https://github.com/raysan5/raylib.git "$TMP_DIR/raylib"
  CC="$CC_BIN" CXX="$CXX_BIN" \
    cmake -S "$TMP_DIR/raylib" -B "$TMP_DIR/raylib/build" -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF
  cmake --build "$TMP_DIR/raylib/build" -j"$(nproc)"
  sudo cmake --install "$TMP_DIR/raylib/build"
  rm -rf "$TMP_DIR"
fi

echo "[4/6] Refreshing linker cache..."
sudo ldconfig

echo "[5/6] Building demo..."
make CXX="$CXX_BIN"

echo "[6/6] Setup complete."
echo "Run the demo with: make run"
