#!/usr/bin/env python3
"""
TidyDownloads - Organize files by type
Usage:
  python tidydownloads.py --folder <path>        # Preview
  python tidydownloads.py --folder <path> --apply  # Move after confirmation
"""

import argparse
import os
import shutil
import sys
from pathlib import Path
from typing import Dict, List, Tuple


CATEGORIES: Dict[str, List[str]] = {
    "Images": ["jpg", "jpeg", "png", "gif", "bmp", "tiff", "tif", "webp", "svg", "ico", "heic", "heif", "raw", "cr2", "nef", "arw", "dng"],
    "Documents": ["pdf", "doc", "docx", "txt", "rtf", "odt", "xls", "xlsx", "ppt", "pptx", "csv", "md", "tex", "epub", "mobi"],
    "Videos": ["mp4", "mkv", "avi", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "ts", "mts", "vob"],
    "Audio": ["mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "opus", "aiff", "aif", "mid", "midi"],
    "Archives": ["zip", "rar", "7z", "tar", "gz", "bz2", "xz", "tgz", "tbz2", "txz", "iso", "dmg", "pkg"],
    "Code": ["cpp", "hpp", "h", "c", "cc", "cxx", "py", "js", "ts", "jsx", "tsx", "java", "cs", "go", "rs", "rb", "php", "swift", "kt", "scala", "html", "css", "scss", "sass", "less", "json", "xml", "yaml", "yml", "toml", "ini", "cfg", "conf", "sh", "bat", "ps1", "sql", "dockerfile", "makefile", "cmake", "gradle", "vue", "svelte"],
}

EXT_TO_CAT: Dict[str, str] = {}
for cat, exts in CATEGORIES.items():
    for ext in exts:
        EXT_TO_CAT[ext.lower()] = cat


def get_category(path: Path) -> str:
    ext = path.suffix.lower()
    if ext.startswith("."):
        ext = ext[1:]
    return EXT_TO_CAT.get(ext, "Other")


def get_unique_dest(dest_dir: Path, filename: str) -> Path:
    """Return a destination path that doesn't exist by adding (1), (2), etc."""
    filename_path = Path(filename)
    dest = dest_dir / filename_path
    if not dest.exists():
        return dest

    stem = filename_path.stem
    suffix = filename_path.suffix
    for i in range(1, 10000):
        new_name = f"{stem} ({i}){suffix}"
        dest = dest_dir / new_name
        if not dest.exists():
            return dest
    return dest  # fallback


def scan_files(folder: Path) -> List[Tuple[Path, Path, str]]:
    """Return list of (source, destination, category) for files to organize."""
    moves = []
    for entry in folder.iterdir():
        if not entry.is_file():
            continue
        category = get_category(entry)
        cat_dir = folder / category
        dest = get_unique_dest(cat_dir, entry.name)
        moves.append((entry, dest, category))
    return moves


def print_preview(moves: List[Tuple[Path, Path, str]], folder: Path) -> None:
    if not moves:
        print(f"No files to organize in: {folder}")
        return

    from collections import defaultdict
    by_cat = defaultdict(list)
    for src, dst, cat in moves:
        by_cat[cat].append((src, dst))

    print(f"Preview of proposed moves in: {folder}")
    print("=" * 60)
    print()

    for cat, items in sorted(by_cat.items()):
        print(f"{cat}/ ({len(items)} file(s))")
        for src, dst in items:
            print(f"  {src.name} -> {cat}/{dst.name}")
        print()

    print("-" * 60)
    print(f"Total: {len(moves)} file(s) to organize")


def apply_moves(moves: List[Tuple[Path, Path, str]]) -> Tuple[int, int]:
    """Execute moves. Returns (moved_count, failed_count)."""
    moved = 0
    failed = 0
    for src, dst, cat in moves:
        try:
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.move(str(src), str(dst))
            moved += 1
        except Exception as e:
            print(f"Failed to move {src.name}: {e}", file=sys.stderr)
            failed += 1
    return moved, failed


def main():
    parser = argparse.ArgumentParser(
        description="TidyDownloads - Organize files by type",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python tidydownloads.py --folder ~/Downloads
  python tidydownloads.py --folder ~/Downloads --apply
        """
    )
    parser.add_argument("--folder", "-f", required=True, help="Target folder to organize")
    parser.add_argument("--apply", "-a", action="store_true", help="Actually move files after confirmation")
    args = parser.parse_args()

    target = Path(args.folder).expanduser().resolve()

    if not target.exists():
        print(f"Error: Folder does not exist: {target}", file=sys.stderr)
        return 1
    if not target.is_dir():
        print(f"Error: Path is not a directory: {target}", file=sys.stderr)
        return 1

    moves = scan_files(target)
    print_preview(moves, target)

    if not moves:
        return 0

    if args.apply:
        print("\nType 'APPLY' to confirm and execute moves: ", end="")
        try:
            confirm = input().strip()
        except EOFError:
            print("\nCancelled.")
            return 0

        if confirm != "APPLY":
            print("Cancelled.")
            return 0

        moved, failed = apply_moves(moves)
        print("\n" + "=" * 60)
        print("Summary:")
        print(f"  Moved:  {moved}")
        print(f"  Failed: {failed}")
        print("=" * 60)

    return 0


if __name__ == "__main__":
    sys.exit(main())