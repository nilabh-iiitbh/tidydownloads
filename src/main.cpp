#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path source;
    fs::path destination;
    std::string category;
};

struct Stats {
    size_t moved = 0;
    size_t skipped = 0;
    size_t failed = 0;
};

class TidyDownloads {
public:
    explicit TidyDownloads(const fs::path& folder) : target_folder_(folder) {
        initialize_categories();
    }

    void preview() {
        scan_files();
        print_preview();
    }

    void apply() {
        scan_files();
        print_preview();

        if (proposed_moves_.empty()) {
            std::cout << "\nNo files to organize.\n";
            return;
        }

        std::cout << "\nType 'APPLY' to confirm and execute moves: ";
        std::string confirmation;
        std::getline(std::cin, confirmation);

        if (confirmation != "APPLY") {
            std::cout << "Cancelled.\n";
            return;
        }

        execute_moves();
        print_summary();
    }

private:
    fs::path target_folder_;
    std::unordered_map<std::string, std::string> extension_to_category_;
    std::vector<FileInfo> proposed_moves_;
    Stats stats_;

    void initialize_categories() {
        // Images
        add_extensions({"jpg", "jpeg", "png", "gif", "bmp", "tiff", "tif", "webp", "svg", "ico", "heic", "heif", "raw", "cr2", "nef", "arw", "dng"}, "Images");

        // Documents
        add_extensions({"pdf", "doc", "docx", "txt", "rtf", "odt", "xls", "xlsx", "ppt", "pptx", "csv", "md", "tex", "epub", "mobi"}, "Documents");

        // Videos
        add_extensions({"mp4", "mkv", "avi", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "ts", "mts", "vob"}, "Videos");

        // Audio
        add_extensions({"mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "opus", "aiff", "aif", "mid", "midi"}, "Audio");

        // Archives
        add_extensions({"zip", "rar", "7z", "tar", "gz", "bz2", "xz", "tgz", "tbz2", "txz", "iso", "dmg", "pkg"}, "Archives");

        // Code
        add_extensions({"cpp", "hpp", "h", "c", "cc", "cxx", "py", "js", "ts", "jsx", "tsx", "java", "cs", "go", "rs", "rb", "php", "swift", "kt", "scala", "html", "css", "scss", "sass", "less", "json", "xml", "yaml", "yml", "toml", "ini", "cfg", "conf", "sh", "bat", "ps1", "sql", "dockerfile", "makefile", "cmake", "gradle", "vue", "svelte"}, "Code");
    }

    void add_extensions(const std::vector<std::string>& extensions, const std::string& category) {
        for (const auto& ext : extensions) {
            extension_to_category_[to_lower(ext)] = category;
        }
    }

    std::string to_lower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    std::string get_extension(const fs::path& path) {
        auto ext = path.extension().string();
        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
        }
        return to_lower(ext);
    }

    std::string get_category(const fs::path& path) {
        auto ext = get_extension(path);
        auto it = extension_to_category_.find(ext);
        if (it != extension_to_category_.end()) {
            return it->second;
        }
        return "Other";
    }

    fs::path get_unique_destination(const fs::path& base_dir, const fs::path& filename) {
        fs::path dest = base_dir / filename;
        if (!fs::exists(dest)) {
            return dest;
        }

        std::string stem = filename.stem().string();
        std::string ext = filename.extension().string();

        for (int i = 1; i < 10000; ++i) {
            fs::path new_name = stem + " (" + std::to_string(i) + ")" + ext;
            dest = base_dir / new_name;
            if (!fs::exists(dest)) {
                return dest;
            }
        }
        return dest; // fallback
    }

    void scan_files() {
        proposed_moves_.clear();
        stats_ = Stats{};

        try {
            for (const auto& entry : fs::directory_iterator(target_folder_)) {
                if (!entry.is_regular_file()) {
                    continue; // Skip subdirectories and non-regular files
                }

                const auto& path = entry.path();
                std::string category = get_category(path);
                fs::path category_dir = target_folder_ / category;
                fs::path dest = get_unique_destination(category_dir, path.filename());

                proposed_moves_.push_back({path, dest, category});
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error scanning folder: " << e.what() << "\n";
        }
    }

    public:
    bool has_proposed_moves() const {
        return !proposed_moves_.empty();
    }

    void print_preview() {
        if (proposed_moves_.empty()) {
            std::cout << "No files to organize in: " << target_folder_ << "\n";
            return;
        }

        // Group by category for display
        std::map<std::string, std::vector<FileInfo>> by_category;
        for (const auto& move : proposed_moves_) {
            by_category[move.category].push_back(move);
        }

        std::cout << "Preview of proposed moves in: " << target_folder_ << "\n";
        std::cout << std::string(60, '=') << "\n\n";

        for (const auto& [category, moves] : by_category) {
            std::cout << category << "/ (" << moves.size() << " file(s))\n";
            for (const auto& move : moves) {
                std::cout << "  " << move.source.filename().string()
                          << " -> " << category << "/" << move.destination.filename().string() << "\n";
            }
            std::cout << "\n";
        }

        std::cout << std::string(60, '-') << "\n";
        std::cout << "Total: " << proposed_moves_.size() << " file(s) to organize\n";
    }

    void execute_moves() {
        for (const auto& move : proposed_moves_) {
            try {
                fs::path category_dir = move.destination.parent_path();
                if (!fs::exists(category_dir)) {
                    fs::create_directories(category_dir);
                }

                fs::rename(move.source, move.destination);
                ++stats_.moved;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to move " << move.source.filename().string()
                          << ": " << e.what() << "\n";
                ++stats_.failed;
            }
        }
    }

    void print_summary() {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Summary:\n";
        std::cout << "  Moved:   " << stats_.moved << "\n";
        std::cout << "  Skipped: " << stats_.skipped << "\n";
        std::cout << "  Failed:  " << stats_.failed << "\n";
        std::cout << std::string(60, '=') << "\n";
    }
};

void print_centered(const std::string& text, int width = 80) {
    int padding = (width - static_cast<int>(text.length())) / 2;
    if (padding < 0) padding = 0;
    std::cout << std::string(padding, ' ') << text << "\n";
}

void print_banner() {
    std::cout << "\n";
    std::string header = "T I D Y   D O W N L O A D S";
    int padding = 8; // extra width on each side
    std::string outline(header.length() + padding * 2, '=');
    print_centered(outline);
    print_centered(header);
    print_centered(outline);
    std::cout << "\n";
    print_centered("by Nilabh");
    std::cout << "\n";
    print_centered("Effortlessly declutter and sort your chaotic files in milliseconds!");
    print_centered("Sorts into: Images | Documents | Videos | Audio | Archives | Code | Other");
    print_centered(std::string(60, '-'));
}

void print_help() {
    print_banner();
    std::cout << "\nUsage:\n";
    std::cout << "  tidydownloads                    Interactive mode (prompts for folder)\n";
    std::cout << "  tidydownloads --folder <path>    Preview proposed moves\n";
    std::cout << "  tidydownloads --folder <path> --apply  Preview, then move after confirmation\n";
    std::cout << "  tidydownloads --help             Show this help\n\n";
    std::cout << "Categories: Images, Documents, Videos, Audio, Archives, Code, Other\n";
}

int main(int argc, char* argv[]) {
    std::string folder;
    bool apply = false;
    bool interactive = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        } else if (arg == "--folder" || arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --folder requires a path argument\n";
                return 1;
            }
            folder = argv[++i];
        } else if (arg == "--apply" || arg == "-a") {
            apply = true;
        } else {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            std::cerr << "Run 'tidydownloads --help' for usage\n";
            return 1;
        }
    }

    // Interactive mode if no folder provided
    if (folder.empty()) {
        print_banner();
        std::cout << "\n  [!] Effortlessly declutter and sort your chaotic files in milliseconds!\n";
        std::cout << "  -------------------------------------------------------------------------\n";
        std::cout << "\n  Enter folder path to organize (or press Enter for current directory): ";
        std::getline(std::cin, folder);

        if (folder.empty()) {
            folder = ".";
        }
        interactive = true;
    }

    fs::path target(folder);

    if (!fs::exists(target)) {
        std::cerr << "Error: Folder does not exist: " << target << "\n";
        return 1;
    }

    if (!fs::is_directory(target)) {
        std::cerr << "Error: Path is not a directory: " << target << "\n";
        return 1;
    }

    try {
        TidyDownloads app(target);

        if (interactive) {
            // In interactive mode, always preview first, then ask to apply
            app.preview();

            if (!app.has_proposed_moves()) {
                return 0;
            }

            std::cout << "\nProceed with organizing these files? (y/N): ";
            std::string confirm;
            std::getline(std::cin, confirm);

            if (confirm == "y" || confirm == "Y" || confirm == "yes" || confirm == "YES") {
                app.apply();
            } else {
                std::cout << "Cancelled.\n";
            }
        } else if (apply) {
            app.apply();
        } else {
            app.preview();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}