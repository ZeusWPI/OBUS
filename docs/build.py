import glob
import os
from pathlib import PurePath
import sys

# https://tex.stackexchange.com/questions/101717/converting-markdown-to-latex-in-latex/246871#246871

remove_testmodules = True
if len(sys.argv) >= 2:
	remove_testmodules = False

outfilename = '/tmp/out.md'
with open(outfilename, 'w') as outfile:
	all_modules = sorted(glob.glob('../src/modules/*/doc/index.md'))
	if remove_testmodules:
		all_modules = [p for p in all_modules if 'testmodule_' not in p]
	without_info = [p for p in all_modules if "info_" not in p]
	only_info = [p for p in all_modules if "info_" in p]
	for filepath in without_info:
		with open(filepath) as infile:
			outfile.write(infile.read())
			outfile.write("\n\\newpage{}\n")
	outfile.write("\n\\part{Appendix}\\newpage\n")
	for filepath in only_info:
		with open(filepath) as infile:
			outfile.write(infile.read())
			outfile.write("\n\\newpage{}\n")
os.system(f"pandoc --from=markdown --template=template.tex --output=docs.pdf prefix.yaml {outfilename} --highlight-style=espresso")
