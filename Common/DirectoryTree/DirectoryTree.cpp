#include "DirectoryTree.hpp"

DirectoryTree::DirectoryTree(const std::string& _id, const size_t _size, const std::string& _filename, DirectoryTree* _parent) :
    m_id(_id), m_size(_size), filename(_filename), parent(_parent), m_children() {}

DirectoryTree::DirectoryTree(DirectoryTree&& other) : DirectoryTree("", 0, "") {
    *this = std::move(other);
}

// current pointer must be destroyed
DirectoryTree& DirectoryTree::operator = (DirectoryTree&& other) {
    filename = std::move(other.filename);
    m_id = other.m_id;
    m_size = other.m_size;
    parent = other.parent;
    other.parent = nullptr;
    m_children = std::move(other.m_children);
    return *this;
}

void buildTree_util(const std::string& encoding, size_t& index, DirectoryTree& parent) {
    // explicit null string instantiation (to prevent c-string related problems)
    const std::string SEP = std::string(1, '\0') + std::string(1, '/');
    while (index < encoding.size()) {
        if (encoding[index] == '\0') {
            ++index;
            return;
        }
        else if (encoding[index] == '/') {
            size_t filenameSeparator = encoding.find_first_of(SEP, index + 1 + DirectoryTree::ID_LEN + 8);
            std::string id = encoding.substr(index + 1, DirectoryTree::ID_LEN);
            std::string sizeString = encoding.substr(index + 1 + DirectoryTree::ID_LEN, 8);
            size_t size;
            memcpy(&size, sizeString.c_str(), 8);
            std::string filename = encoding.substr(
                index + 1 + DirectoryTree::ID_LEN + 8, 
                filenameSeparator - index - 1 - DirectoryTree::ID_LEN - 8
            );
            index = filenameSeparator;
            parent.addChild(id, size, filename);
            buildTree_util(encoding, index, parent.child(parent.childrenSize() - 1));
        }
    }
}

DirectoryTree DirectoryTree::buildTree(const std::string& encoding) {
    DirectoryTree root("", 0, "");
    size_t index = 8;
    buildTree_util(encoding, index, root);
    return root;
}

std::string DirectoryTree::encodeTree() const {
    std::string sizeString(8, '\0');
    memcpy(sizeString.data(), &m_size, 8);
    std::string encoding = m_id + sizeString + filename;
    if (parent) {
        encoding = '/' + encoding;
    }
    for (const DirectoryTree& child : m_children) {
        encoding += child.encodeTree();
    }
    encoding += '\0';
    return  encoding;
}

void DirectoryTree::addChild(const std::string& id, const size_t size, const std::string& filename) {
    m_children.emplace_back(id, size, filename, this);
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

DirectoryTree* DirectoryTree::findID(const std::string& target) {
    if (m_id == target) {
        return this;
    }
    for (DirectoryTree& child : m_children) {
        DirectoryTree* ptr = child.findID(target);
        if (ptr) {
            return ptr;
        }
    }
    return nullptr;
}

const DirectoryTree* DirectoryTree::findID(const std::string& target) const {
    if (m_id == target) {
        return this;
    }
    for (const DirectoryTree& child : m_children) {
        const DirectoryTree* ptr = child.findID(target);
        if (ptr) {
            return ptr;
        }
    }
    return nullptr;
}

std::string DirectoryTree::id() const {
    return m_id;
}

size_t DirectoryTree::size() const {
    return m_size;
}