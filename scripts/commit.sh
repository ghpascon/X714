#!/usr/bin/env bash
set -euo pipefail

echo "Selecione o tipo de commit:"
echo "1 - Patch (correcoes pequenas)"
echo "2 - Minor (novas funcionalidades)"
echo "3 - Major (mudancas incompativeis)"
read -rp "Digite o numero (1/2/3): " type

state=""
if [[ "$type" == "1" ]]; then state="patch"; fi
if [[ "$type" == "2" ]]; then state="minor"; fi
if [[ "$type" == "3" ]]; then state="major"; fi

if [[ -z "$state" ]]; then
  echo "Tipo invalido. Abortando."
  exit 1
fi

read -rp "Digite a mensagem do commit: " msg

git add .
git commit -m "$state - $msg"
git push
