/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines implementation for Boost filesystem scanner
 */

#include <cstdlib>
#include <stdexcept>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <filesystem_scanner.hpp>

//! The function scans Boost directory tree and builds header dependency tree. The function optionally detects Boost sublibraries and returns the nodes that correspond to the sublib directories.
void scan_filesystem_tree
(
    boost::filesystem::path const& boost_root,
    dep_tree& root,
    std::vector< dep_node* >* sublibs,
    std::vector< boost::filesystem::path > const& include_dirs,
    bool create_reverse_dependencies
)
{
    BOOST_ASSERT(boost_root.is_absolute());

    boost::filesystem::directory_iterator dir_it(boost_root), dir_end;
    for (; dir_it != dir_end; ++dir_it)
    {

    }
}

//! The function finds Boost root directory
boost::filesystem::path find_boost_root()
{
    const char* env_boost_root = std::getenv("BOOST_ROOT");
    if (env_boost_root)
        return boost::filesystem::path(env_boost_root);

    boost::filesystem::path dir = boost::filesystem::current_path();
    while (!(boost::filesystem::is_regular_file(dir / "Jamroot") && boost::filesystem::is_directory(dir / "libs")))
    {
        if (!dir.has_parent_path())
            BOOST_THROW_EXCEPTION(std::runtime_error("Could not determine Boost root directory"));

        dir = dir.parent_path();
    }

    return dir;
}
