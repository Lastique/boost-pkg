/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines interface for the dependency tree
 */

#ifndef BOOST_PKG_DEP_TREE_DEP_TREE_HPP_INCLUDED_
#define BOOST_PKG_DEP_TREE_DEP_TREE_HPP_INCLUDED_

#include <string>
#include <vector>
#include <boost/config.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/set_hook.hpp>
#include <boost/utility/string_ref.hpp>

typedef boost::intrusive::set_base_hook<
    boost::intrusive::tag< struct for_dep_node_tree >,
    boost::intrusive::link_mode< boost::intrusive::safe_link >,
    boost::intrusive::optimize_size< true >
> dep_node_set_hook_t;

class dep_node :
    public dep_node_set_hook_t
{
public:
    //! Ordering predicate for lookup by name
    struct order_by_name
    {
        typedef bool result_type;

        result_type operator() (dep_node const& left, dep_node const& right) const BOOST_NOEXCEPT
        {
            return order(left.get_name(), right.get_name());
        }
        result_type operator() (dep_node const& left, boost::string_ref const& right) const BOOST_NOEXCEPT
        {
            return order(left.get_name(), right);
        }
        result_type operator() (boost::string_ref const& left, dep_node const& right) const BOOST_NOEXCEPT
        {
            return order(left, right.get_name());
        }

    private:
        static result_type order(boost::string_ref const& left, boost::string_ref const& right) BOOST_NOEXCEPT
        {
            return left < right;
        }
    };

    //! List of nodes for tracking dependencies
    typedef std::vector< dep_node* > nodes;

    //! Set of nodes for lookup by name
    typedef boost::intrusive::set<
        dep_node,
        boost::intrusive::base_hook< dep_node_set_hook_t >,
        boost::intrusive::compare< order_by_name >,
        boost::intrusive::constant_time_size< false >
    > node_set;

private:
    dep_node* m_parent;
    node_set m_children;
    const std::string m_name;
    nodes m_dependencies;
    nodes m_dependents;

public:
    static BOOST_CONSTEXPR_OR_CONST char default_node_separator = '/';

public:
    //! Creates a root node
    dep_node();
    //! Creates a child node
    dep_node(dep_node* parent, boost::string_ref const& name);
    //! Destructor
    ~dep_node();

    //! Returns the parent node
    dep_node* get_parent() const BOOST_NOEXCEPT { return m_parent; }
    //! Returns the root node
    dep_node* get_root() const BOOST_NOEXCEPT;
    //! Returns the set of children nodes
    node_set const& get_children() const BOOST_NOEXCEPT { return m_children; }
    //! Returns an immediate child node with the specified name
    dep_node* get_child(boost::string_ref const& name) BOOST_NOEXCEPT;
    //! Returns a possibly nested child node by the specified path
    dep_node* navigate(boost::string_ref const& path, char separator = default_node_separator) BOOST_NOEXCEPT;

    //! Returns the node name
    std::string const& get_name() const BOOST_NOEXCEPT { return m_name; }
    //! Returns the full node name
    std::string get_full_name(char separator = default_node_separator) const;
    //! Returns the nodes that this node depend on
    nodes const& get_dependencies() const BOOST_NOEXCEPT { return m_dependencies; }
    //! Returns the nodes that depend on this node
    nodes const& get_dependents() const BOOST_NOEXCEPT { return m_dependents; }

    //! Adds an immediate child node or returns the existing node if one exists
    dep_node* add_child(boost::string_ref const& name);
    //! Adds a possibly nested child node or returns the existing node if one exists
    dep_node* add_nested_child(boost::string_ref const& path, char separator = default_node_separator);

    //! Adds a dependency node
    void add_dependency(dep_node* node);
    //! Adds a dependency node identified by path from root node. The node is created, if needed.
    void add_dependency(boost::string_ref const& path, char separator = default_node_separator);

    //! Adds a dependent node
    void add_dependent(dep_node* node);
    //! Adds a dependent node identified by path from root node. The node is created, if needed.
    void add_dependent(boost::string_ref const& path, char separator = default_node_separator);

    BOOST_DELETED_FUNCTION(dep_node(dep_node const&))
    BOOST_DELETED_FUNCTION(dep_node& operator=(dep_node const&))
};

typedef dep_node dep_tree;

#endif // BOOST_PKG_DEP_TREE_DEP_TREE_HPP_INCLUDED_
