#!/bin/bash

echo "Selecione o tipo de commit:"
echo "1 - Patch (correções pequenas)"
echo "2 - Minor (novas funcionalidades)"
echo "3 - Major (mudanças incompatíveis)"
read -p "Digite o número (1/2/3): " type

case $type in
	1)
		state="patch"
		;;
	2)
		state="minor"
		;;
	3)
		state="major"
		;;
	*)
		echo "Tipo inválido. Abortando."
		exit 1
		;;
esac

read -p "Digite a mensagem do commit: " msg

git add .
git commit -m "$state - $msg"
git push
