#!/usr/bin/env bash
set -euo pipefail

# Expected clang-format MAJOR version
REQUIRED_MAJOR="${CLANG_FORMAT_REQUIRED_MAJOR:-18}"

# Ensure clang-format is installed
if ! command -v clang-format >/dev/null 2>&1; then
  echo "Error: clang-format not found in PATH. Install clang-format-${REQUIRED_MAJOR}."
  exit 1
fi

# Parse installed version
RAW_VER="$(clang-format --version || true)"
# Try to capture full semver if present; fall back to first integer as major
FULL_VER="$(echo "$RAW_VER" | sed -E 's/.*version ([0-9]+\.[0-9]+\.[0-9]+).*/\1/;t; s/.*version ([0-9]+).*/\1/')"
MAJOR_VER="$(echo "$FULL_VER" | cut -d. -f1)"

echo "Using clang-format version: $RAW_VER"
if [[ "$MAJOR_VER" -lt "$REQUIRED_MAJOR" ]]; then
  echo "❌ Version mismatch: required major $REQUIRED_MAJOR, but found '$FULL_VER'"
  echo "   Tip (Ubuntu): sudo apt-get install clang-format-${REQUIRED_MAJOR}"
  echo "   Optionally set a symlink: sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-${REQUIRED_MAJOR} 100"
  exit 1
fi

# Collect all tracked C/C++/ObjC files
mapfile -t FILES < <(git ls-files \
  '*.[ch]' '*.cc' '*.cpp' '*.cxx' \
  '*.hh' '*.hpp' '*.hxx' '*.m' '*.mm')

if [ ${#FILES[@]} -eq 0 ]; then
  echo "No C/C++/ObjC files found."
  exit 0
fi

echo "Checking ${#FILES[@]} files with clang-format $FULL_VER ..."

# Create patch of what formatting would change
clang-format-"$MAJOR_VER" -i "${FILES[@]}"
git diff > clang-format.patch

if [ -s clang-format.patch ]; then
  echo
  echo "❌ Formatting issues found."
  echo "   Inspect/apply locally with:  git apply clang-format.patch"
  exit 1
else
  echo "✅ All files are properly formatted."
  rm -f clang-format.patch
fi
