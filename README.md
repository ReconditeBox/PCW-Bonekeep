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

The current development target is an Amstrad PCW / compatible emulator
running CP/M Plus with Hi-Tech C 3.09-21.

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

The map utilities are built by the commands in:

    BUILDM.SUB

This produces:

- `MENCODE.COM`
- `MDECODE.COM`
- `MEDIT.COM`

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

- `BONEKEEP.COM`
- `BONEKEEP.DAT`
- the encoded `LEVEL.xxx` files used by the dungeon

`BONEKEEP.DAT` is stored under `runtime/` in this repository.

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
