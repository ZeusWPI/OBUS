import glob
import os

# https://tex.stackexchange.com/questions/101717/converting-markdown-to-latex-in-latex/246871#246871

outfilename = '/tmp/out.md'
with open(outfilename, 'w') as outfile:
	for filepath in glob.iglob('../src/modules/*/doc/index.md'):
		with open(filepath) as infile:
			outfile.write(infile.read())
			outfile.write("\n\\newpage{}\n")
os.system(f"pandoc --from=markdown --output=docs.pdf {outfilename} --highlight-style=espresso")
