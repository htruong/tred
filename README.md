## tred - TRuong's EDitor

A Vietnamese UTF-8 text editor for MS-DOS (formerly vicat, a Vietnamese file viewer).

![tred](https://github.com/htruong/vicat/blob/master/docs/screenshot.png?raw=true)

## Features

- Edits Vietnamese UTF-8 text files on MS-DOS with a VGA card, in an edit.com-style interface.
- Types Vietnamese with Telex or VNI like Unikey; F9 switches between Telex, VNI and off (also in the Options menu).
- Uses a BKED-style 8x14 font in 350-line text mode: Vietnamese letters live in the second VGA font, so CP437 line drawing and your normal font are left alone, and the normal text mode comes back on exit.
- Wraps long lines at word boundaries by default (Options > Word Wrap turns it off); wrapping is only on screen, the file keeps its lines.
- Keeps the file's line endings (CRLF or LF), BOM and final newline when saving.

## Usage

    tred [file]

| Key | Action |
| --- | --- |
| F9 | Switch input method: Telex, VNI, off |
| F10, Alt+F/E/S/O/H | Menus |
| Ctrl+S | Save |
| Alt+X | Exit |
| Ctrl+Y | Delete line |
| Ins | Insert / overwrite |

Telex: `aa` â, `aw` ă, `ee` ê, `oo` ô, `ow` ơ, `uw` ư, `dd` đ, then `s f r x j` for the tones and `z` to remove one; typing a key twice undoes it (`ass` gives `as`). VNI uses `1`-`5` for tones, `6` ^, `7` horn, `8` breve, `9` đ, `0` to remove the tone.

## Building and testing

Build with Open Watcom's `wmake` in `src/`. Inside DOSBox the DOS-hosted tools in `binw` work.

`tred -k keys.txt file` replays keystrokes from `keys.txt` (characters plus tokens like `{Enter}`, `{Down}`, `{C-S}`, `{F9}`), then exits and leaves the screen as it is, for automated checks.

The input method and text buffer have host tests:

    cc -Isrc -o t_ime tests/t_ime.c src/ime.c src/utf8.c && ./t_ime
    cc -Isrc -o t_textbuf tests/t_textbuf.c src/textbuf.c && ./t_textbuf

The font is generated from `tools/bked_glyphs.txt` by `tools/mkfont.py`.

## Why?

... why not? Programming for MS-DOS is fun.

## Acknowledgements

- [TECH Help Manual](http://www.techhelpmanual.com/)
- [Code page 437](https://en.wikipedia.org/wiki/Code_page_437)
- [VileR's FRAPT](https://int10h.org/blog/2019/05/fontraption-vga-text-mode-font-editor/)
- [OpenWatcom-VSCode](https://github.com/intbeam/watcom-vscode)
- [Code page 437 to Unicode - Lookup table](https://stackoverflow.com/questions/45375804/converting-char-from-cp437-encoding-to-utf-8-encoding-always-yields-the-same-cha)
- [VileR's int10h page](https://int10h.org/)
