#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <utility>

class dir;

class file {
public:
    explicit file(std::string name, std::string path)
        : m_name(std::move(name)), m_path(std::move(path)), m_parent(nullptr) {}

    virtual ~file() {
        m_parent = nullptr;
    }

    [[nodiscard]] virtual std::string get_name() const {
        return m_name;
    }

    virtual void set_name(const std::string& name) {
        m_name = name;
    }

    [[nodiscard]] virtual const std::string& get_path() const {
        return m_path;
    }

    virtual void set_path(const std::string& path) {
        file::m_path = path;
    }

    [[nodiscard]] virtual dir* get_parent() const {
        return m_parent;
    }

    virtual void set_parent(dir* parent) {
        m_parent = parent;
    }

    [[nodiscard]] virtual const std::string& get_content() const {
        return m_content;
    }

    virtual void set_content(const std::string& content) {
        m_content = content;
    }

    virtual void append_content(const std::string& content) {
        m_content.append(content);
    }

    virtual void list() const {
        std::cout << m_name << " (in " << m_parent << ')' << std::endl;
    }

protected:
    std::string m_name;
    std::string m_path;
    dir* m_parent;

private:
    std::string m_content;
};

#endif // FILE_H
