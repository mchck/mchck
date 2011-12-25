SOURCES=	$(wildcard *.ps)
PDFS=		${SOURCES:.ps=-crop.pdf}
PNGS=		${SOURCES:.ps=.png}

all: clean pdf png

clean:
	-rm -f *.pdf *.png

realclean:
	-rm -f *.pdf *.png *.ps *.gbr *.gbl *.gto *.gts *.gbs *.gtl *.dsn *.ses *.svg *.svgz *.drl *.csv *.lst

pdf: ${PDFS}
png: ${PNGS}


%.pdf: %.ps
	ps2pdf $^

%-crop.pdf: %.pdf
	pdfcrop $^

%.png: %.pdf
	convert -density 600 -alpha off $^ $@
