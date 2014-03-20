all : html pdf

dist: html
	rm -f jazz.aux jazz.con jazz.dvi jazz.idx jazz.ilg jazz.ind jazz.log jazz.ref jazz.toc

pdf: jazz.pdf
jazz.pdf: jazz.tex
	pdflatex $<
	pdflatex $<
	pdflatex $<

#ps: jazz.ps
#jazz.ps : jazz.dvi
#	dvips jazz.dvi -o jazz.ps

jazz.dvi: jazz.tex
	latex jazz
	latex jazz
	latex jazz

html: jazz_contents.html
jazz_contents.html : jazz.tex
	tex2rtf jazz.tex jazz.html -html -twice

clean:
	rm -f jazz.aux jazz.dvi jazz.idx jazz.ilg jazz.ind jazz.log jazz.ref jazz.toc
	rm -f jazz.htx jazz.hhc jazz.hhk jazz.hhp *.con *.html
	rm -f jazz.pdf
