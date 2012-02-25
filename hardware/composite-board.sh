#!/bin/sh

set -e

if [ $# -lt 4 ]
then
	echo "usage: composite-board.sh <copper-pattern> <mask-pattern> <silkscreen> <edges> [<imagemagick-options> ...] <output-file>" >&2
	exit 1
fi

# lets see what is going on in that black box there...
set -x;

# pop input file names off the parameter list and give them nice names
SRC_COPPER="$1";
SRC_MASK="$2";
SRC_SILKS="$3";
SRC_EDGES="$4";
shift 4;

# FIXME: sanity check that all input files exist and we have at least one 
# more parameter left (a non-empty destination name) in the parameter list!

# create temporary working directory
TMPDIR="$(mktemp -d)";

# make working copies of all source files
cp "$SRC_COPPER"	"$TMPDIR/copper.png";
cp "$SRC_MASK"		"$TMPDIR/mask.png";
cp "$SRC_SILKS"		"$TMPDIR/silks.png";
cp "$SRC_EDGES"		"$TMPDIR/edges.png";

(
	# enter temporary directory in subshell for easy processing
	cd "$TMPDIR";

	# preprocess images: invert colors, define transparency and colorize as appropriate

	# copper layer
	convert \
		"copper.png" \
		-negate \
		-background gold \
		-alpha Shape \
		"MIFF:copper.miff";

	# solder stop layer
	convert \
		"mask.png" \
		-background 'rgb(0,100,0)' \
		-alpha Shape \
		"MIFF:mask.miff";

	# silk screen layer
	convert \
		"silks.png" \
		-negate \
		-background white \
		-alpha Shape \
		"MIFF:silks.miff";
	# clip silk screen with mask
	convert "silks.miff" "mask.miff" -channel A -compose multiply -composite "silks.miff";

	# board edges layer (no colorize; board edges are already black)
	convert \
		"edges.png" \
		-negate \
		-background black \
		-alpha Shape \
		"MIFF:edges.miff";

	# get board shape
	convert \
		"edges.png" \
		-fill black \
		-bordercolor black \
		-threshold '50%' \
		-draw 'color 50%,50% floodfill' \
		-negate \
		-alpha Shape \
		"MIFF:shape.miff";

	# generate overlay image

	# create empty slightly tinted PCB base
	convert \
		"copper.png" \
		+matte \
		-fill 'rgb(170, 200, 110)' \
		-draw 'color 0,0 reset' \
		"MIFF:pcb.miff";

	# stack and merge layers in order: copper, mask, silkscreen, board edges
	composite "mask.miff"	"copper.miff"  -compose divide "overlay.miff";
        composite "pcb.miff"    "overlay.miff" -compose dst-atop "overlay.miff";
        composite "silks.miff"  "overlay.miff" -compose src-atop "overlay.miff";
	composite "edges.miff"	"overlay.miff" -compose src-atop "overlay.miff";
	composite "shape.miff"  "overlay.miff" -compose copy-opacity "overlay.miff";

	# output is handed back as "overlay.miff"
);

# move result to final destination and convert to desired format
convert "$TMPDIR/overlay.miff" -background white -alpha Background "$@";

# delete the working directory and stale temporary files
rm -rf "$TMPDIR";
