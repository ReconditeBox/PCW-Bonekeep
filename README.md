# BONEKEEP

BONEKEEP is a dungeon game for the Amstrad PCW under CP/M Plus.

It is written in old-style K&R C for Hi-Tech C 3.09-21 and is currently
under active playtesting and balance work.

## Repository layout

- `src/` - game source, map tools, CP/M build scripts and manuals
- `maps/` - raw source maps, `LEVEL000.MAP` through `LEVEL010.MAP`
- `runtime/` - runtime data file required by the game
- `LICENSE` - MIT License

## Target environment

BONEKEEP now has two build targets:

- Amstrad PCW / compatible emulator running CP/M Plus, using the native PCW/Joyce terminal controls at 90 x 30.
- Generic CP/M 2.2 using an 80 x 24 VT100-compatible terminal.

Both targets are built with Hi-Tech C 3.09-21 and use the same 75 x 24 dungeon files and runtime data.

BONEKEEP is sensitive to available TPA. The final game link deliberately uses
Hi-Tech C's `-N` option to keep startup memory small enough for the current
build.

## Building BONEKEEP

Copy the contents of `src/` to the CP/M working directory.

The game build commands are in:

    BUILDBK.SUB

They compile the modules individually and finish with:

    CC -V -N GAME.OBJ VNAME.OBJ PLAYER.OBJ INV.OBJ LEVEL.OBJ STATE.OBJ MONSTER.OBJ SCREEN.OBJ VIS.OBJ

The linker produces `GAME.COM`; the build script renames it to
`BONEKEEP.COM`.

### Generic CP/M 2.2 / VT100 build

The alternate 80-column build commands are in:

    BUILD80.SUB

This build defines `VT100_80`, links `SCREEN80.C` instead of `SCREEN.C`,
and renames the result to:

    BONE80.COM

It expects a VT100-compatible 80 x 24 terminal. The logical dungeon remains
75 x 24; the terminal edition displays a scrolling 64 x 18 viewport with the
status and inventory panel on the right and messages at the bottom.

The VT100 build uses standard CP/M 2.2 console services for its startup random
seed rather than the CP/M Plus clock BDOS call used by the PCW build.

The PCW map utilities are built by the commands in:

    BUILDM.SUB

This produces:

- `MENCODE.COM`
- `MDECODE.COM`
- `MEDIT.COM`

For generic CP/M 2.2 with an 80 x 24 VT100 terminal, use:

    BUILDM80.SUB

This produces the same terminal-independent `MENCODE.COM` and
`MDECODE.COM`, plus `MEDIT80.COM`. The VT100 editor keeps the logical
75 x 24 map size and presents it through a scrolling 64 x 18 viewport.

## Building level files

The raw map sources are in `maps/`.

Encode each map with `MENCODE`, for example:

    MENCODE LEVEL000.MAP LEVEL.000
    MENCODE LEVEL001.MAP LEVEL.001

Continue through `LEVEL010.MAP`.

`LEVEL.000` is the starting level. Only level 000 may contain the player start
`@` and dungeon exit `X`.

Adjacent stair locations must use matching coordinates between levels.

See `src/MAPEDIT.TXT` for the complete map-source format and tool
documentation.

## Runtime files

A playable directory requires at least:

- `BONEKEEP.COM` (PCW build) or `BONE80.COM` (generic CP/M 2.2 / VT100 build)
- `BONEKEEP.DAT`
- the encoded `LEVEL.xxx` files used by the dungeon

`BONEKEEP.DAT` is stored under `runtime/` in this repository. The VT100 build
uses the same file and renders its randomized ending text in an 80-column-safe
victory screen.

The player's manual is `src/BONEKEEP.TXT`.

## Current gameplay

The current dungeon uses Skeletons, Zombies and the Crown Vampire.

The character begins with a deliberately narrower random SKILL and STAMINA
range than earlier development versions. Regular undead become stronger as the
player descends, and Sword, Shield and Armour quality also rises with dungeon
depth.

The Crown Vampire is found on level 010.

This is still a work in progress. Combat balance and level design should be
considered subject to further playtesting.

## Source-format note

Files intended for CP/M are deliberately stored without Git line-ending
conversion. Several text files use CR/LF and a final CP/M Ctrl-Z EOF byte.
The repository's `.gitattributes` file preserves those bytes.

## License

BONEKEEP is licensed under the MIT License.

Copyright (c) 2026 Tony Blews
