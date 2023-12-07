#include "DirectoryTree.hpp"

DirectoryTree::DirectoryTree(const std::string& _filename, DirectoryTree* _parent) :
    filename(_filename), parent(_parent), m_children() {}

DirectoryTree::DirectoryTree(DirectoryTree&& other) {
    *this = std::move(other);
}

// current pointer must be destroyed
DirectoryTree& DirectoryTree::operator = (DirectoryTree&& other) {
    filename = std::move(other.filename);
    parent = other.parent;
    other.parent = nullptr;
    m_children = std::move(other.m_children);
    return *this;
}

void buildTree_util(const std::string& encoding, size_t& index, DirectoryTree& parent) {
    if (encoding[index] == '\0') {
        return;
    }
    std::size_t filenameSeparator = encoding.find_first_of(std::string("/\\"), index + 1);
    std::string filename = encoding.substr(index + 1, filenameSeparator - index - 1);
    index = filenameSeparator;
    parent.addChild(filename);
    while (index != std::string::npos) {
        if (encoding[index] == '\0') {
            ++index;
            return;
        }
        buildTree_util(encoding, index, parent.child(parent.childrenSize() - 1));
    }
}

DirectoryTree DirectoryTree::buildTree(const std::string& encoding) {
    size_t index = 0;
    DirectoryTree root("");
    buildTree_util(encoding, index, root);
    return root;
}

std::string DirectoryTree::encodeTree() const {
    std::string encoding = filename;
    if (parent != nullptr) {
        encoding = '/' + filename;
    }
    for (const DirectoryTree& child : m_children) {
        encoding += child.encodeTree();
    }
    encoding += '\0';
    return  encoding;
}

void DirectoryTree::addChild(const std::string& filename) {
    m_children.emplace_back(filename, this);
}

void DirectoryTree::addChild(DirectoryTree&& child) {
    m_children.push_back(std::move(child));
}

size_t DirectoryTree::childrenSize() const {
    return m_children.size();
}

DirectoryTree& DirectoryTree::child(const size_t index) {
    return m_children[index];
}

std::string DirectoryTree::path() const {
    if (parent == nullptr) {
        return filename + "/";
    }
    return parent->path() + filename + "/";
}

std::string DirectoryTree::name() const {
    return filename;
}