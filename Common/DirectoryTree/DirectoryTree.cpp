#include "DirectoryTree.hpp"

DirectoryTree::DirectoryTree(const std::string& _id, const size_t _size, const std::string& _filename, DirectoryTree* _parent) :
    m_id(_id), m_size(_size), filename(_filename), m_parent(_parent), m_children{} {}

DirectoryTree::DirectoryTree(DirectoryTree&& other) {
    *this = std::move(other);
}

// current pointer must be destroyed
DirectoryTree& DirectoryTree::operator = (DirectoryTree&& other) {
    m_id = std::move(other.m_id);
    m_size = other.m_size;
    filename = std::move(other.filename);
    m_parent = other.m_parent;
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
    if (m_parent) {
        encoding = '/' + encoding;
    }
    for (const auto& child : m_children) {
        encoding += child->encodeTree();
    }
    encoding += '\0';
    return  encoding;
}

void DirectoryTree::addChild(const std::string& id, const size_t size, const std::string& filename) {
    m_children.emplace_back(std::make_unique<DirectoryTree>(id, size, filename, this));
}

#include <iostream>

void DirectoryTree::addChild(DirectoryTree&& child) {
    child.m_parent = this;
    m_children.push_back(std::make_unique<DirectoryTree>(std::move(child)));
}

void DirectoryTree::erase(const size_t index) {
    m_children.erase(m_children.begin() + index);
}

void DirectoryTree::rename(const std::string& name) {
    filename = name;
}

size_t DirectoryTree::childrenSize() const {
    return m_children.size();
}

DirectoryTree& DirectoryTree::child(const size_t index) {
    return *(m_children[index]);
}

const DirectoryTree& DirectoryTree::child(const size_t index) const {
    return *(m_children[index]);
}

std::string DirectoryTree::path() const {
    if (m_parent == nullptr) {
        return filename + "/";
    }
    return m_parent->path() + filename + "/";
}

std::string DirectoryTree::name() const {
    return filename;
}

DirectoryTree* DirectoryTree::findID(const std::string& target) {
    if (m_id == target) {
        return this;
    }
    for (auto& child : m_children) {
        DirectoryTree* ptr = child->findID(target);
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
    for (const auto& child : m_children) {
        const DirectoryTree* ptr = child->findID(target);
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
    // file is directory
    if (m_size == 0) {
        size_t res = 0;
        for (auto& child : m_children) {
            res += child->size();
        }
        return res;
    }
    return m_size;
}

bool DirectoryTree::isDirectory() const {
    return m_size == 0;
}

DirectoryTree* DirectoryTree::parent() {
    return m_parent;
}

DirectoryTree DirectoryTree::clone() const {
    DirectoryTree cloneFile(id(), size(), name(), m_parent);
    for (const auto& child : m_children) {
        cloneFile.addChild(child->clone());
    }
    return cloneFile;
}