P=	mchck

SOURCES=	$(wildcard $P*.ps) $(wildcard $(SCHEMATIC:.sch=*.ps))
PDFS=		${SOURCES:.ps=-crop.pdf}
PNGS=            ${COMPOSITE_PNGS} ${SOURCES:.ps=.png}
PNG_PREVIEWS=	${PNGS:.png=-small.png}
SCHEMATIC=	$(wildcard *.sch)
BOARD=		$P.brd
GERBERS=	$(wildcard $P-*.g[bt][lso] $P-*.gbr $P*.drl)
LICENSE_FILES=	LICENSE LICENSE.HOWTO README.md
COMPOSITE_PNGS=	$P-F_Composite.png $P-B_Composite.png

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

# ImageMagick crashes when specifying the Palette type and writing to png... do it two-step instead
%.png: %-crop.pdf
	convert -density 1200 -alpha off -resize '25%' -resize '2000x2000>' -type optimize -quality 95 $^ $@
	-[ $$(du -k $@ | cut -f 1) -gt 80 ] && convert $@ -colors 255 -define png:format=png8 -quality 95 $@

%-small.png: %.png
	convert -type TrueColorMatte -resize '300x300' -quality 95 -type optimize $^ $@

# Wouldn't it be great if we didn't need this rule?
$P-Composite_%-small.png: $P-Composite_%.png
	convert -type TrueColorMatte -resize '300x300' -quality 95 -type optimize $^ $@

$P-%_Composite.png: $P-%_Cu.png $P-%_Mask.png $P-%_SilkS.png $P-Edge_Cuts.png
	sh composite-board.sh $^ $$(case "$@" in *B_*) printf -- "-flop";;esac) -type optimize -quality 95 $@


${GERBERS}: ${BOARD}
	@echo "Fabrication outputs older than board file!" >&2
	@exit 1


FAB_BRANCH=	fab
FAB_INDEX=	$(shell git rev-parse --git-dir || echo ".git")/fab-index
e=	GIT_INDEX_FILE=${FAB_INDEX}

ci-fab: ${GERBERS} ${PDFS} ${PNGS} ${PNG_PREVIEWS} ${LICENSE_FILES}
	@if ! git diff --quiet HEAD -- . ${LICENSE_FILES}; then \
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

update-wiki:
	cd ../wiki && git pull
	cp ${PNGS} ${PNG_PREVIEWS} ../wiki/gen-images
	cd ../wiki/gen-images && git add ${PNGS} ${PNG_PREVIEWS} && git commit -m 'update generated images' && git push
	git commit -m 'update wiki' ../wiki

.PHONY: update-wiki

zip: ${P}.zip

.PHONY: zip

${P}.zip: ${GERBERS}
	-rm $@
	zip $@ $^
