#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <boost/filesystem.hpp>
#include <windows.h>

namespace fs = boost::filesystem;

class Node {
public:
    explicit Node(const std::string& name, bool is_directory = false)
        : name(name), is_directory(is_directory) {}

    void addChild(std::unique_ptr<Node> node) {
        children.push_back(std::move(node));
    }

    void print(int depth = 0) const {
        for (int i = 0; i < depth; ++i) std::cout << "| ";
        std::cout << (is_directory ? "[D] " : "[F] ") << name << "\n";
        for (const auto& child : children) {
            child->print(depth + 1);
        }
    }

    const std::string& getName() const { return name; }
    bool isDirectory() const { return is_directory; }
    const std::vector<std::unique_ptr<Node>>& getChildren() const { return children; }

private:
    std::string name;
    bool is_directory;
    std::vector<std::unique_ptr<Node>> children;
};

class DirectoryTree {
public:
    explicit DirectoryTree(const std::string& root_path) {
        root = std::make_unique<Node>(fs::path(root_path).filename().string(), true);
        buildTree(root_path, root);
    }

    void print() const {
        root->print();
    }

private:
    void buildTree(const std::string& current_path, std::unique_ptr<Node>& current_node) {
        try {
            for (const auto& entry : fs::directory_iterator(current_path)) {
                auto node = std::make_unique<Node>(
                    entry.path().filename().string(),
                    entry.is_directory()
                );

                if (entry.is_directory()) {
                    buildTree(entry.path().string(), node);
                }

                current_node->addChild(std::move(node));
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing: " << current_path << " - " << e.what() << "\n";
        }
    }

    std::unique_ptr<Node> root;
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::string path;
    std::cout << "Введите путь к директории: ";
    std::getline(std::cin, path);

    if (!fs::exists(path)) {
        std::cerr << "Директория не существует!\n";
        return 1;
    }

    DirectoryTree tree(path);
    std::cout << "\nСтруктура каталогов:\n";
    tree.print();

    return 0;
}