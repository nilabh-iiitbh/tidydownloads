# TidyDownloads

A C++17 command-line tool that safely organizes files in a folder by type.

## Features

- Organizes files into categories: **Images**, **Documents**, **Videos**, **Audio**, **Archives**, **Code**, **Other**
- Preview mode by default (read-only, no modifications)
- Apply mode requires explicit `APPLY` confirmation
- Never overwrites files — adds numbered suffixes (e.g., `photo (1).jpg`)
- Skips subdirectories
- Clear summary of moved, skipped, and failed files

## Building

Requires a C++17 compiler and CMake 3.10+.

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be at `build/Release/tidydownloads.exe` (Windows) or `build/tidydownloads` (Linux/macOS).

## Usage

```
tidydownloads --folder <path>        # Preview proposed moves
tidydownloads --folder <path> --apply  # Preview, then move after typing APPLY
tidydownloads --help                 # Show help
```

### Examples

```bash
# Preview what would be organized in Downloads
tidydownloads --folder ~/Downloads

# Actually organize after confirmation
tidydownloads --folder ~/Downloads --apply
```

### Categories and Extensions

| Category | Extensions |
|----------|------------|
| **Images** | jpg, jpeg, png, gif, bmp, tiff, tif, webp, svg, ico, heic, heif, raw, cr2, nef, arw, dng |
| **Documents** | pdf, doc, docx, txt, rtf, odt, xls, xlsx, ppt, pptx, csv, md, tex, epub, mobi |
| **Videos** | mp4, mkv, avi, mov, wmv, flv, webm, m4v, mpg, mpeg, 3gp, ogv, ts, mts, vob |
| **Audio** | mp3, wav, flac, aac, ogg, m4a, wma, opus, aiff, aif, mid, midi |
| **Archives** | zip, rar, 7z, tar, gz, bz2, xz, tgz, tbz2, txz, iso, dmg, pkg |
| **Code** | cpp, hpp, h, c, cc, cxx, py, js, ts, jsx, tsx, java, cs, go, rs, rb, php, swift, kt, scala, html, css, scss, sass, less, json, xml, yaml, yml, toml, ini, cfg, conf, sh, bat, ps1, sql, dockerfile, makefile, cmake, gradle, vue, svelte |
| **Other** | Everything else |

## Safety

- **Preview mode is read-only** — no files are modified unless you use `--apply` and confirm
- **No overwrites** — existing files get a numbered suffix
- **No network access** — completely local operation
- **No deletion** — files are only moved, never deleted
- **Category folders created only on apply**

## License

Public domain / Unlicense. Use freely.