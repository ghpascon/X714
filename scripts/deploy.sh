#!/usr/bin/env bash
set -euo pipefail

echo "Selecione o tipo de versao:"
echo "1 - Patch (correcoes pequenas)"
echo "2 - Minor (novas funcionalidades)"
echo "3 - Major (mudancas incompativeis)"
read -rp "Digite o numero (1/2/3): " type

case "$type" in
  1) bump="patch" ;;
  2) bump="minor" ;;
  3) bump="major" ;;
  *) echo "Tipo invalido. Abortando."; exit 1 ;;
esac

read -rp "Digite a mensagem do commit: " msg

# determina a raiz do repo e o arquivo version.h
repo_root=$(git rev-parse --show-toplevel 2>/dev/null || true)
if [[ -z "$repo_root" ]]; then
  echo "Nao parece ser um repositorio git. Abortando."
  exit 1
fi
cd "$repo_root"

version_file="$repo_root/program_version.h"
if [[ ! -f "$version_file" ]]; then
  echo "Arquivo $version_file nao encontrado. Abortando."
  exit 1
fi

# ler versao atual
current_version=$(sed -n 's/.*#define PROGRAM_VERSION "\([^"]*\)".*/\1/p' "$version_file" | head -n1)
if [[ -z "$current_version" ]]; then
  echo "Nao foi possivel ler a versao atual em $version_file"
  exit 1
fi

IFS='.' read -r major minor patch <<< "$current_version"
major=${major:-0}
minor=${minor:-0}
patch=${patch:-0}

re='^[0-9]+$'
if ! [[ "$major" =~ $re && "$minor" =~ $re && "$patch" =~ $re ]]; then
  echo "Versao atual com formato invalido: $current_version"
  exit 1
fi

case "$bump" in
  patch) patch=$((patch + 1)) ;;
  minor) minor=$((minor + 1)); patch=0 ;;
  major) major=$((major + 1)); minor=0; patch=0 ;;
  *) echo "Tipo de bump invalido"; exit 1 ;;
esac

new_version="${major}.${minor}.${patch}"

echo "Atualizando $version_file: $current_version -> $new_version"
tmpfile=$(mktemp)
cat > "$tmpfile" <<EOF
#define PROGRAM_VERSION "$new_version"
EOF
mv "$tmpfile" "$version_file"

# commitar e push
git add .
if git diff --cached --quiet; then
  echo "Nenhuma mudanca para commitar depois do bump. Abortando."
  exit 1
fi

git commit -m "${new_version} - ${msg}"
git push

# criar e enviar tag
tag="v${new_version}"
if git rev-parse --verify "$tag" >/dev/null 2>&1; then
  echo "Tag $tag ja existe; nao sera criada."
else
  git tag -a "$tag" -m "${new_version} - ${msg}"
  git push origin "$tag"
fi

echo "Concluido: versao $new_version (tag: $tag)"
