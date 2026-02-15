#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKILLS_SRC="${SCRIPT_DIR}/skills"
SKILLS_DST="${HOME}/.claude/skills"

echo "Installing Firefly Pixie skills for Claude Code..."

if [ ! -d "${SKILLS_SRC}" ]; then
  echo "Error: skills/ directory not found. Run this from the repo root." >&2
  exit 1
fi

mkdir -p "${SKILLS_DST}"

for skill_dir in "${SKILLS_SRC}"/firefly-*; do
  skill_name="$(basename "${skill_dir}")"
  echo "  Installing ${skill_name}..."
  rm -rf "${SKILLS_DST}/${skill_name}"
  cp -r "${skill_dir}" "${SKILLS_DST}/${skill_name}"
done

echo ""
echo "Done! Installed skills:"
for skill_dir in "${SKILLS_SRC}"/firefly-*; do
  echo "  - $(basename "${skill_dir}")"
done
echo ""
echo "Restart Claude Code to activate the new skills."
echo "Then try: \"build and flash the template app\""
