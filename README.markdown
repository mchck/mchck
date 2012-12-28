# The MC HCK!

The goal is to have a small, cheap, and powerful development board
that supports USB for easy programming, with a target price of $5.
This allows us to use the board for all silly purposes
where spending >$20 for an Arduino is just too much.

Find up-to-date information and get involved at <http://mchck.org>.

## Cross build environment

% nix-env -p cross -iA nixos.pkgs.gccCrossStageStatic -f default.nix
