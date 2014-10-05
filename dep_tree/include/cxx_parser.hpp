/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines interface for the C++ files parser
 */

#ifndef BOOST_PKG_DEP_TREE_CXX_PARSER_HPP_INCLUDED_
#define BOOST_PKG_DEP_TREE_CXX_PARSER_HPP_INCLUDED_

#include <string>
#include <dep_tree.hpp>
#include <boost/exception/error_info.hpp>

//! Error info used to communicate the file name that failed to be parsed
typedef boost::error_info< struct file_name, std::string > file_name_info;

//! The function creates a node for a header and fills its dependencies depending on the header contents
void parse_cxx(const char* path, dep_tree& root, bool create_reverse_dependencies = true);

#endif // BOOST_PKG_DEP_TREE_CXX_PARSER_HPP_INCLUDED_
