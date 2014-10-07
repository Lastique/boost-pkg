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
#include <string>
#include <vector>
#include <dep_tree.hpp>
#include <boost/filesystem/path.hpp>

//! The function finds Boost root directory
boost::filesystem::path find_boost_root();

//! The function returns a wildcard that matches all files
std::vector< std::string > all_files_wildcards();

//! The function returns the list of directories in Boost root that should normally be skipped from scanning
std::vector< boost::filesystem::path > default_skip_root_dirs();

//! The function returns wildcards for the filenames that are typically used to store C++ code in Boost
std::vector< std::string > default_cxx_wildcards();

//! Filesystem scanner parameters
struct scan_params
{
    boost::filesystem::path boost_root;
    std::vector< std::string > whitelist_wildcards;
    std::vector< std::string > blacklist_wildcards;
    std::vector< std::string > cxx_wildcards;
    std::vector< boost::filesystem::path > skip_root_dirs;
    std::vector< boost::filesystem::path > include_dirs;
    bool create_reverse_dependencies;

    scan_params();

    //! Creates scaanning parameters that are typically used to create a full directory with dependencies
    static scan_params typical(boost::filesystem::path const& boost_root = find_boost_root());
};

//! The function scans Boost directory tree and builds header dependency tree. The function optionally detects Boost sublibraries and returns the nodes that correspond to the sublib directories.
void scan_filesystem_tree(scan_params const& params, dep_tree& root, std::vector< dep_node* >* sublibs = NULL);

#endif // BOOST_PKG_DEP_TREE_FILESYSTEM_SCANNER_HPP_INCLUDED_
