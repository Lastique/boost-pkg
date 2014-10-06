/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines interface for Boost filesystem scanner
 */

#ifndef BOOST_PKG_DEP_TREE_FILESYSTEM_SCANNER_HPP_INCLUDED_
#define BOOST_PKG_DEP_TREE_FILESYSTEM_SCANNER_HPP_INCLUDED_

#include <cstddef>
#include <vector>
#include <dep_tree.hpp>
#include <boost/filesystem/path.hpp>

//! The function finds Boost root directory
boost::filesystem::path find_boost_root();

//! The function scans Boost directory tree and builds header dependency tree. The function optionally detects Boost sublibraries and returns the nodes that correspond to the sublib directories.
void scan_filesystem_tree
(
    boost::filesystem::path const& boost_root,
    dep_tree& root,
    std::vector< dep_node* >* sublibs = NULL,
    std::vector< boost::filesystem::path > const& include_dirs = std::vector< boost::filesystem::path >(),
    bool create_reverse_dependencies = true
);

#endif // BOOST_PKG_DEP_TREE_FILESYSTEM_SCANNER_HPP_INCLUDED_
