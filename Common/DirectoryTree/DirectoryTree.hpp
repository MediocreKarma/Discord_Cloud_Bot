#ifndef _DIRECTORY_TREE__
#define _DIRECTORY_TREE__

#include <string>
#include <vector>
#include <memory>
#include <cstring>

class DirectoryTree {
public:
    static constexpr size_t ID_LEN = 8;

    //DirectoryTree() = default;
    DirectoryTree(const std::string& id, size_t size, const std::string& filename, DirectoryTree* parent = nullptr);
    DirectoryTree(DirectoryTree&&);

    DirectoryTree& operator = (const DirectoryTree&) = delete;
    DirectoryTree& operator = (DirectoryTree&&);

    static DirectoryTree buildTree(const std::string& encoding);
    std::string encodeTree() const;

    void addChild(const std::string& id, size_t size, const std::string& filename);
    void addChild(DirectoryTree&& child);
    void erase(size_t index);

    void rename(const std::string& name);
    
    size_t childrenSize() const;
    DirectoryTree& child(size_t index);
    const DirectoryTree& child(size_t index) const;

    std::string path() const;
    std::string name() const;

    DirectoryTree* findID(const std::string& id);
    const DirectoryTree* findID(const std::string& id) const;
    DirectoryTree* parent();

    std::string id() const;
    size_t size() const;

    bool isDirectory() const;

    DirectoryTree clone() const;

private:
    // db identifier
    std::string m_id;
    size_t m_size;
    // aka alias, screen name
    std::string filename = "";
    DirectoryTree* m_parent = nullptr;
    std::vector<std::unique_ptr<DirectoryTree>> m_children = {};
};

#endif