#ifndef _DIRECTORY_TREE__
#define _DIRECTORY_TREE__

#include <string>
#include <vector>

class DirectoryTree {
public:
    //DirectoryTree() = default;
    DirectoryTree(const std::string& id, const std::string& filename, DirectoryTree* parent = nullptr);
    DirectoryTree(const DirectoryTree&) = delete;
    DirectoryTree(DirectoryTree&&);

    DirectoryTree& operator = (const DirectoryTree&) = delete;
    DirectoryTree& operator = (DirectoryTree&&);

    static DirectoryTree buildTree(const std::string& encoding);
    std::string encodeTree() const;

    void addChild(const std::string& id, const std::string& filename);
    void addChild(DirectoryTree&& child);
    size_t childrenSize() const;
    DirectoryTree& child(size_t index);

    std::string path() const;
    std::string name() const;

    DirectoryTree* findID(const std::string& id);
    const DirectoryTree* findID(const std::string& id) const;

private:
    // db identifier
    std::string id;
    // aka alias, screen name
    std::string filename = "";
    DirectoryTree* parent = nullptr;
    std::vector<DirectoryTree> m_children = {};
};

#endif