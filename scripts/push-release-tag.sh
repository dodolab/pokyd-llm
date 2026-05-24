#!/usr/bin/env bash
# Create/update a release tag and push ONLY the tag (triggers .github/workflows/release.yml).
#
# Pushing main alone does not run the release workflow. Pushing main + tag in one
# command can also skip the tag webhook; push the tag in its own step.
#
# Usage:
#   ./scripts/push-release-tag.sh 1.0
#   ./scripts/push-release-tag.sh v1.0.0
#
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TAG="${1:?Usage: $0 <tag>}"

cd "$ROOT_DIR"

if ! git rev-parse "$TAG" >/dev/null 2>&1; then
  git tag "$TAG"
fi

echo "Pushing tag ${TAG} (current commit: $(git rev-parse --short HEAD))..."
git push origin "refs/tags/${TAG}"

REMOTE="$(git remote get-url origin 2>/dev/null || true)"
if [[ "$REMOTE" =~ github\.com[:/]([^/]+)/([^/.]+) ]]; then
  owner="${BASH_REMATCH[1]}"
  repo="${BASH_REMATCH[2]}"
  echo "Actions: https://github.com/${owner}/${repo}/actions/workflows/release.yml"
fi
