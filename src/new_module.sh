#!/bin/bash

# "Bash strict mode", see http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

# Go to the current working directory so things work if people are in a different one and e.g. use ../src/new_module.sh
cd "$(dirname "$0")"

# Make sure the template dir exists so we don't let people enter details unnecessarily
if [ ! -d ./template_module ]; then
	echo "template_module doesn't exist" >&2
	exit 1
fi

# Ask for module name
read -p "Name of module (e.g. Oil gauge): " module_name
if [[ $module_name == *%* ]]; then
	echo "Module name must not contain %" >&2
	exit 1
fi

# Determine a "clean" module name: lowercase, no spaces
module="$(tr [A-Z] [a-z] <<< "$module_name")"
module="${module// /_}"
module="${module//\'/}"

# Make sure `modules` directory exists and target directory doesn't
mkdir -p modules
module_dir="modules/$module"
if [[ -e "$module_dir" ]]; then
	echo "$module_dir already exists" >&2
	exit 1
fi

# Ask for author name
read -p "How would you like to be credited? Your name: " author
if [[ $author == *%* ]]; then
	echo "Author name must not contain %" >&2
	exit 1
fi

# Copy the template directory
cp -r template_module "$module_dir"
cd "$module_dir"

# Fill in the blanks in the template
if [ "$(uname)" == "Darwin" ]; then
	 sed -i '' -e "
                s/{YEAR}/$(date +%Y)/
                s%{AUTHOR}%$author%
                s%{MODULE_NAME}%$module_name%
                s%{MODULE}%$module%
                " $(find . -type f)
else
	sed -i "
		s/{YEAR}/$(date +%Y)/
		s%{AUTHOR}%$author%
		s%{MODULE_NAME}%$module_name%
		s%{MODULE}%$module%
		" $(find -type f)
fi
# Arduino IDE requires .ino sketches to have the same name as their directory
mv main.ino "$module.ino"

echo "The basic structure for your module is now ready in $module_dir"
