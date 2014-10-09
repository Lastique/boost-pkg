/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines implementation of serialization/deserialization the dependency tree in JSON format
 */

#include <cstddef>
#include <string>
#include <boost/assert.hpp>
#include <json.hpp>

namespace {

const char meta_tag[] = "$meta";
const char deps_tag[] = "deps";
const char rdeps_tag[] = "rdeps";

void serialize_meta(dep_node const& node, std::string const& newline_indent, std::string const& indent, bool with_rdeps, std::ostream& strm)
{
    std::string nested_newline_indent = newline_indent + indent;
    std::string nested_nested_newline_indent = nested_newline_indent + indent;

    strm << newline_indent << '"' << meta_tag << "\":" << newline_indent << '{';

    bool is_first = true;
    dep_node::nodes const& deps = node.get_dependencies();
    if (!deps.empty())
    {
        strm << nested_newline_indent << '"' << deps_tag << "\":" << nested_newline_indent << '[';

        for (dep_node::nodes::const_iterator it = deps.begin(), end = deps.end(); it != end; ++it)
        {
            if (!is_first)
                strm << ',';
            else
                is_first = false;
            strm << nested_nested_newline_indent << '"' << (*it)->get_full_name() << '"';
        }

        strm << nested_newline_indent << ']';
    }

    if (with_rdeps)
    {
        dep_node::nodes const& rdeps = node.get_dependents();
        if (!rdeps.empty())
        {
            if (!is_first)
                strm << ',';
            strm << nested_newline_indent << '"' << rdeps_tag << "\":" << nested_newline_indent << '[';

            is_first = true;
            for (dep_node::nodes::const_iterator it = rdeps.begin(), end = rdeps.end(); it != end; ++it)
            {
                if (!is_first)
                    strm << ',';
                else
                    is_first = false;
                strm << nested_nested_newline_indent << '"' << (*it)->get_full_name() << '"';
            }

            strm << nested_newline_indent << ']';
        }
    }

    strm << newline_indent << '}';
}

void serialize_node(dep_node const& node, std::string const& newline_indent, std::string const& indent, bool with_rdeps, std::ostream& strm)
{
    std::string nested_newline_indent = newline_indent + indent;

    strm << newline_indent << '"' << node.get_name() << "\":";

    if (!node.get_children().empty() || !node.get_dependencies().empty() || !(with_rdeps && node.get_dependents().empty()))
    {
        strm << newline_indent << '{';

        bool is_first = true;
        for (dep_node::node_set::const_iterator it = node.get_children().begin(), end = node.get_children().end(); it != end; ++it)
        {
            if (!is_first)
                strm << ',';
            else
                is_first = false;
            serialize_node(*it, nested_newline_indent, indent, with_rdeps, strm);
        }

        if (!node.get_dependencies().empty() || !(with_rdeps && node.get_dependents().empty()))
        {
            if (!is_first)
                strm << ',';
            else
                is_first = false;
            serialize_meta(node, nested_newline_indent, indent, with_rdeps, strm);
        }

        strm << newline_indent << '}';
    }
    else
    {
        // Conserve some space for empty nodes
        strm << " {}";
    }
}

} // namespace

//! Serializes the tree into JSON format
void serialize_json(dep_tree const& root, std::ostream& strm, bool with_rdeps, bool pretty_print, const char* indent)
{
    BOOST_ASSERT(root.get_parent() == NULL);

    std::string nl_ind, ind;
    if (pretty_print)
    {
        ind = indent;
        nl_ind = "\n" + ind;
    }

    strm << '{';

    for (dep_node::node_set::const_iterator it = root.get_children().begin(), end = root.get_children().end(); it != end; ++it)
    {
        serialize_node(*it, nl_ind, ind, with_rdeps, strm);
    }

    if (pretty_print)
        strm << '\n';
    strm << '}';
    if (pretty_print)
        strm << '\n';
    strm << std::flush;
}
