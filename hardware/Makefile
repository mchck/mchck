SOURCES=	$(wildcard *.ps)
PDFS=		${SOURCES:.ps=-crop.pdf}
PNGS=		${SOURCES:.ps=.png}
PNG_PREVIEWS=	${PNGS:.png=-small.png}
BOARD=		$(wildcard *.brd)
GERBERS=	$(wildcard *.g[bt][lso] *.gbr *.drl)
LICENSE_FILES=	LICENSE LICENSE.HOWTO README.md

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


${GERBERS}: ${BOARD}
	@echo "Fabrication outputs older than board file!" >&2
	@exit 1


FAB_BRANCH=	fab
FAB_INDEX=	$(shell git rev-parse --git-dir || echo ".git")/fab-index
e=	GIT_INDEX_FILE=${FAB_INDEX}

ci-fab: ${GERBERS} ${PDFS} ${PNGS} ${PNG_PREVIEWS} ${LICENSE_FILES}
	@if ! git diff --quiet HEAD; then \
		echo "tree is dirty.  please commit first." >&2; \
		exit 1; \
	fi
	git branch ${FAB_BRANCH} origin/${FAB_BRANCH} || true
	rm -f ${FAB_INDEX}
	$e git update-index --add $^
	@if $e git diff-index --quiet --cached ${FAB_BRANCH}; then \
		echo "LISTEN: no changes to commit" >&2; \
		exit 1; \
	fi
	git update-ref refs/heads/${FAB_BRANCH} \
		$$((printf "fab outputs from "; git log --pretty=oneline --abbrev-commit -n1) | \
		$e git commit-tree $$($e git write-tree) \
			$$(git show-ref -q ${FAB_BRANCH} && echo "-p ${FAB_BRANCH}") -p HEAD)

.PHONY: ci-fab
