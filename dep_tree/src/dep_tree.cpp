#include <cstddef>
#include <utility>
#include <algorithm>
#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>
#include <boost/move/utility.hpp>
#include <dep_tree.hpp>
#include <path_iterator.hpp>

BOOST_CONSTEXPR_OR_CONST char dep_node::default_node_separator;

dep_node::dep_node() : m_parent(NULL)
{
}

dep_node::dep_node(dep_node* parent, boost::string_ref const& name) : m_parent(parent), m_name(name.data(), name.size())
{
}

dep_node::~dep_node()
{
    m_children.clear_and_dispose(boost::checked_deleter< dep_node >());
}

//! Returns the root node
dep_node* dep_node::get_root() const BOOST_NOEXCEPT
{
    dep_node* root = const_cast< dep_node* >(this);
    while (root->m_parent)
        root = root->m_parent;
    return root;
}

//! Returns the full node name
std::string dep_node::get_full_name(char separator) const
{
    std::string full_name = m_name;

    const dep_node* p = this;
    while (p->m_parent)
    {
        p = p->m_parent;
        full_name.insert(0, 1, separator);
        full_name.insert(0, p->m_name);
    }

    return boost::move(full_name);
}

//! Returns an immediate child node with the specified name
dep_node* dep_node::get_child(boost::string_ref const& name) BOOST_NOEXCEPT
{
    node_set::iterator it = m_children.find(name, order_by_name());
    if (it != m_children.end())
        return &*it;
    return NULL;
}

//! Returns a possibly nested child node by the specified path
dep_node* dep_node::navigate(boost::string_ref const& path, char separator) BOOST_NOEXCEPT
{
    dep_node* node = this;
    path_iterator p(path, separator);
    boost::string_ref name = *p;
    while (!name.empty())
    {
        node_set::iterator it = node->m_children.find(name, order_by_name());
        if (it == node->m_children.end())
            return NULL;

        node = &*it;

        ++p;
        name = *p;
    }

    return node;
}

//! Adds a child node or returns the existing node if one exists
dep_node* dep_node::add_child(boost::string_ref const& name)
{
    BOOST_ASSERT(!name.empty());

    node_set::insert_commit_data commit_data;
    std::pair< node_set::iterator, bool > res = m_children.insert_check(name, order_by_name(), commit_data);
    if (res.second)
    {
        dep_node* node = new dep_node(this, name);
        res.first = m_children.insert_commit(*node, commit_data);
    }
    return &*res.first;
}

//! Adds a possibly nested child node or returns the existing node if one exists
dep_node* dep_node::add_nested_child(boost::string_ref const& path, char separator)
{
    BOOST_ASSERT(!path.empty());

    dep_node* node = this;
    path_iterator p(path, separator);
    boost::string_ref name = *p;
    while (!name.empty())
    {
        node = node->add_child(name);
        ++p;
        name = *p;
    }

    return node;
}

//! Adds a dependency node
void dep_node::add_dependency(dep_node* node)
{
    BOOST_ASSERT(node != NULL);

    nodes::iterator it = std::lower_bound(m_dependencies.begin(), m_dependencies.end(), node);
    if (it == m_dependencies.end() || node != *it)
        m_dependencies.insert(it, node);
}

//! Adds a dependency node identified by path from root node. The node is created, if needed.
void dep_node::add_dependency(boost::string_ref const& path, char separator)
{
    add_dependency(get_root()->add_nested_child(path, separator));
}

//! Adds a dependent node
void dep_node::add_dependent(dep_node* node)
{
    BOOST_ASSERT(node != NULL);

    nodes::iterator it = std::lower_bound(m_dependents.begin(), m_dependents.end(), node);
    if (it == m_dependents.end() || node != *it)
        m_dependents.insert(it, node);
}

//! Adds a dependent node identified by path from root node. The node is created, if needed.
void dep_node::add_dependent(boost::string_ref const& path, char separator)
{
    add_dependent(get_root()->add_nested_child(path, separator));
}

//! The function reconstructs reverse dependencies between the tree nodes
void reconstruct_reverse_dependencies(dep_tree& root)
{
    for (dep_node::node_set::const_iterator it = root.get_children().begin(), end = root.get_children().end(); it != end; ++it)
    {
        reconstruct_reverse_dependencies(const_cast< dep_node& >(*it));
    }

    for (dep_node::nodes::const_iterator it = root.get_dependencies().begin(), end = root.get_dependencies().end(); it != end; ++it)
    {
        (*it)->add_dependent(&root);
    }
}
