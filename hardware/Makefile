SOURCES=	$(wildcard *.ps)
PDFS=		${SOURCES:.ps=-crop.pdf}
PNGS=		${SOURCES:.ps=.png}
PNG_PREVIEWS=	${PNGS:.png=-small.png}

all: clean png png_preview

clean:
	-rm -f *.pdf *.png

realclean:
	-rm -f *.pdf *.png *.ps *.gbr *.gbl *.gto *.gts *.gbs *.gtl *.dsn *.ses *.svg *.svgz *.drl *.csv *.lst

pdf: ${PDFS}
png: ${PNGS}
png_preview: ${PNG_PREVIEWS}


%.pdf: %.ps
	ps2pdf $^

%-crop.pdf: %.pdf
	pdfcrop $^

%.png: %-crop.pdf
	convert -antialias -density 600 -alpha off $^ $@

%-small.png: %.png
	convert -resize 10% $^ $@
