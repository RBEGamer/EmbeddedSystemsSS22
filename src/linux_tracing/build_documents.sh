#!/bin/bash
cd .
# REMOVE TEMP FILES
rm -f *.acn
rm -f *.acr
rm -f *.alg
rm -f *.ist
rm -f *.glg
rm -f *.glo
rm -f *.gls
rm -f *.aux
rm -f *.lof
rm -f *.lot
rm -f *.aux
rm -f *.out
rm -f *.toc
rm -f *.log
rm -f *.synctex.gz
rm -f *.bbl
rm -f *.blg
rm -f *.fls
rm -f *.fdb_latexmk
rm -f *.run.xml

# REMOVE GENERATED TEX
rm -f ./declaration.tex
rm -f ./document.tex
rm -f ./abstract.tex
rm -f ./attachments.tex
# REMOVE GENERATED PDF
rm -f ./main.pdf


echo "-- STARTING BUILDING DOCUMENT --"
pandoc --version




pandoc ./document.md -o ./document.tex --from markdown --biblatex --template ./pandoc_template.tex --listings --top-level-division=chapter --lua-filter ./pandoc-gls.lua
# NOW THE HACKY PART WE WANT TO USE THE STANDART cite command instead the from pandoc used cite to we use sed to hard replace the stuff
sed -i 's/\\autocite{/\\cite{/g' ./document.tex
# python3 ./fix-table-color-bleed.py ./_document.tex > ./document.tex

pandoc ./declaration.md -o ./declaration.tex --from markdown --top-level-division=chapter --listings
pandoc ./abstract.md -o ./abstract.tex --from markdown --top-level-division=chapter --listings
pandoc ./attachments.md -o ./attachments.tex --from markdown --top-level-division=chapter --listings

echo "------------- PANDOC GENERATION FINISHED -----------"

# BUILD FIRST TIME GENERATE .AUX and .TOC FILE
pdflatex ./main.tex ./main.pdf
# GENERATE BIBTEX INDEX
makeglossaries main # ACRONYM
bibtex main # REFERENCES
pdflatex ./main.tex ./main.pdf
# BUILD FINAL DOCUMENT
pdflatex ./main.tex ./main.pdf

echo "------------- PDF EXPORT FINISHED -----------"



# EXPORT AS HTML

#pandoc -s thesis.tex -o thesis.md
#pandoc thesis.tex -f latex -t html -s -o thesis.html --bibliography thesis_references.bib

# REMOVE TEMP FILES
rm -f *.acn
rm -f *.acr
rm -f *.alg
rm -f *.ist
rm -f *.glg
rm -f *.glo
rm -f *.gls
rm -f *.aux
rm -f *.lof
rm -f *.lot
rm -f *.aux
rm -f *.out
rm -f *.toc
rm -f *.log
rm -f *.synctex.gz
rm -f *.bbl
rm -f *.blg
rm -f *.fls
rm -f *.fdb_latexmk
rm -f *.run.xml


exit 0
