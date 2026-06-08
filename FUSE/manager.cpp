#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void list_files(const std::string& mount_path) {
    std::cout << "\n--- Files in FUSE System ---\n";
    bool empty = true;
    if (fs::exists(mount_path) && fs::is_directory(mount_path)) {
        for (const auto& entry : fs::directory_iterator(mount_path)) {
            std::cout << "- " << entry.path().filename().string() << " (" << fs::file_size(entry) << " bytes)\n";
            empty = false;
        }
    }
    if (empty) std::cout << "(No files found)\n";
}

int main() {
    std::string mount_point = "./mount_point";
    int choice;

    while (true) {
        std::cout << "\n--- Linux FS Manager ---\n";
        std::cout << "1. List Files\n";
        std::cout << "2. Create File & Write Initial Data\n";
        std::cout << "3. Append Text to Existing File\n"; // אופציה חדשה
        std::cout << "4. Append Content of One File to Another\n"; // אופציה חדשה
        std::cout << "5. Read File Content\n";
        std::cout << "6. Delete File\n";
        std::cout << "7. Exit\n";
        std::cout << "Choice: ";
        std::cin >> choice;

        if (choice == 7) break;

        std::string filename, content, src, dest;
        switch(choice) {
            case 1:
                list_files(mount_point);
                break;
            case 2: // יצירה עם כתיבה
                std::cout << "Enter filename to create: ";
                std::cin >> filename;
                std::cout << "Enter initial data: ";
                std::cin.ignore();
                std::getline(std::cin, content);
                {
                    std::ofstream f(mount_point + "/" + filename);
                    if (f) { f << content; std::cout << "File created with data.\n"; }
                    else std::cout << "Error creating file.\n";
                }
                break;
            case 3: // הוספת טקסט ידני לקובץ קיים
                std::cout << "Enter filename to append to: ";
                std::cin >> filename;
                std::cout << "Enter text to add: ";
                std::cin.ignore();
                std::getline(std::cin, content);
                {
                    std::ofstream f(mount_point + "/" + filename, std::ios::app);
                    if (f) { f << content << "\n"; std::cout << "Text appended.\n"; }
                    else std::cout << "Error: File not found.\n";
                }
                break;
            case 4: // שרשור קובץ לקובץ
                std::cout << "Source file: "; std::cin >> src;
                std::cout << "Destination file: "; std::cin >> dest;
                {
                    std::ifstream s_file(mount_point + "/" + src);
                    std::ofstream d_file(mount_point + "/" + dest, std::ios::app);
                    if (s_file && d_file) {
                        d_file << s_file.rdbuf();
                        std::cout << "Successfully appended " << src << " to " << dest << ".\n";
                    } else std::cout << "Error opening files.\n";
                }
                break;
            case 5: // קריאה
                std::cout << "Filename to read: "; std::cin >> filename;
                {
                    std::ifstream f(mount_point + "/" + filename);
                    if (f) {
                        std::string line;
                        std::cout << "\n--- Content ---\n";
                        while (std::getline(f, line)) std::cout << line << std::endl;
                    } else std::cout << "Error: File not found.\n";
                }
                break;
            case 6: // מחיקה
                std::cout << "Filename to delete: "; std::cin >> filename;
                if (fs::remove(mount_point + "/" + filename)) std::cout << "File deleted.\n";
                else std::cout << "Error: File not found.\n";
                break;
            default:
                std::cout << "Invalid choice.\n";
        }
    }
    return 0;
}

