#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <windows.h>
#include <locale>
#include <codecvt>

class Node {
public:
    explicit Node(const std::wstring& name, bool is_dir = false)
        : name(name), is_directory(is_dir) {}

    void addChild(std::unique_ptr<Node> node) {
        children.push_back(std::move(node));
    }

    void print(int depth = 0) const {
        for (int i = 0; i < depth; ++i) std::cout << "| ";
        
        // Преобразование wide string в UTF-8 для вывода
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::cout << (is_directory ? "[D] " : "[F] ") 
                  << converter.to_bytes(name) << "\n";
        
        for (const auto& child : children) {
            child->print(depth + 1);
        }
    }

private:
    std::wstring name;
    bool is_directory;
    std::vector<std::unique_ptr<Node>> children;
};

class DirectoryTree {
public:
    explicit DirectoryTree(const std::wstring& root_path) {
        root = std::make_unique<Node>(getFileName(root_path), true);
        buildTree(root_path, root);
    }

    void print() const {
        root->print();
    }

private:
    void buildTree(const std::wstring& path, std::unique_ptr<Node>& node) {
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW((path + L"\\*").c_str(), &findData);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Cannot open directory: " << path << L" - Error: " << GetLastError() << std::endl;
            return;
        }

        do {
            // Пропускаем служебные записи
            if (wcscmp(findData.cFileName, L".") == 0 || 
                wcscmp(findData.cFileName, L"..") == 0) {
                continue;
            }

            std::wstring full_path = path + L"\\" + findData.cFileName;
            bool is_dir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

            auto child = std::make_unique<Node>(findData.cFileName, is_dir);
            
            if (is_dir) {
                buildTree(full_path, child);
            }
            
            node->addChild(std::move(child));
        } while (FindNextFileW(hFind, &findData) != 0);

        FindClose(hFind);
    }

    std::wstring getFileName(const std::wstring& path) {
        size_t pos = path.find_last_of(L"\\/");
        return (pos != std::wstring::npos) ? path.substr(pos + 1) : path;
    }

    std::unique_ptr<Node> root;
};

int main() {
    // Настройка кодировки консоли
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // Установка локали для широких символов
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());

    // Конвертер для преобразования строк
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    
    // Получение аргументов командной строки в Unicode
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    
    // Определение пути для сканирования
    std::wstring path;
    if (argc > 1) {
        path = argv[1];
    } else {
        // Если аргумент не указан - используем текущую директорию
        wchar_t currentDir[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, currentDir);
        path = currentDir;
    }
    
    // Проверка существования пути
    DWORD attribs = GetFileAttributesW(path.c_str());
    if (attribs == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "Ошибка: Путь не существует или недоступен\n";
        LocalFree(argv);
        return 1;
    }
    if (!(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
        std::cerr << "Ошибка: Указанный путь не является директорией\n";
        LocalFree(argv);
        return 1;
    }

    // Вывод информации о сканируемой директории
    std::cout << "Структура каталогов для: " 
              << converter.to_bytes(path) << "\n\n";
    
    DirectoryTree tree(path);
    tree.print();
    
    // Освобождение ресурсов
    LocalFree(argv);
    return 0;
}