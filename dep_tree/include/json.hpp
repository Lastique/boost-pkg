/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines interface for serializing/deserializing the dependency tree in JSON format
 */

#ifndef BOOST_PKG_DEP_TREE_JSON_HPP_INCLUDED_
#define BOOST_PKG_DEP_TREE_JSON_HPP_INCLUDED_

#include <ostream>
#include <dep_tree.hpp>

//! Serializes the tree into JSON format
void serialize_json(dep_tree const& root, std::ostream& strm, bool with_rdeps = true, bool pretty_print = true, const char* indent = "\t");

#endif // BOOST_PKG_DEP_TREE_JSON_HPP_INCLUDED_
