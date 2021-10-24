#ifndef DIR_H
#define DIR_H

#include "file.h"

#include <vector>
#include <algorithm>

/**
 * Class that represents a directory in VSFS.
 */
class dir : public file
{
public:
    explicit dir(const std::string& name, const std::string& path) : file(name, path)
    {}

    ~dir() override
    {
        std::for_each(m_children.begin(), m_children.end(), [](file* child)
        {
            delete child;
        });
    }

    void add_child(file* child)
    {
        // Detach child's parent
        dir* parent = child->get_parent();
        if (parent && parent != this)
            parent->remove_child(child);

        // Add the child file
        m_children.push_back(child);
        m_children.back()->set_parent(this);
    }

    void remove_child(file* child)
    {
        size_t index = find_by_name(child);
        if (index != m_children.size())
            m_children.erase(m_children.begin() + (int) index);
    }

    size_t find_by_name(file* child)
    {
        return find_by_name(child->get_name());
    }

    size_t find_by_path(file* child)
    {
        return find_by_path(child->get_path());
    }

    size_t find_by_name(const std::string& child_name)
    {
        size_t index = 0;
        for (; index < m_children.size(); index++)
            if (m_children.at(index)->get_name() == child_name)
                break;

        return index;
    }

    size_t find_by_path(const std::string& child_path)
    {
        size_t index = 0;
        for (; index < m_children.size(); index++)
            if (m_children.at(index)->get_path() == child_path)
                break;

        return index;
    }

    [[nodiscard]] std::vector<file*>& get_children()
    {
        return m_children;
    }

private:
    std::vector<file*> m_children;

    // Disable meaningless functions
    using file::get_content;
    using file::set_content;
    using file::append_content;

};

#endif // DIR_H
