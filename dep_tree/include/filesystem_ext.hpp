/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines several extensions for Boost.Filesystem
 */

#ifndef BOOST_PKG_DEP_TREE_FILESYSTEM_EXT_HPP_INCLUDED_
#define BOOST_PKG_DEP_TREE_FILESYSTEM_EXT_HPP_INCLUDED_

#include <cstddef>
#include <string>
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

//! Follows all symlinks and returns the path that is referred to by the symlinks
inline boost::filesystem::path peel_symlinks(boost::filesystem::path path, boost::filesystem::file_status* status = NULL)
{
    boost::filesystem::file_status file_stat = boost::filesystem::status(path);
    while (boost::filesystem::is_symlink(file_stat))
    {
        path = boost::filesystem::read_symlink(path);
        file_stat = boost::filesystem::status(path);
    }
    if (status)
        *status = file_stat;
    return path;
}

//! Tests is one path is a descendant to the other
inline bool is_descendant(boost::filesystem::path const& parent, boost::filesystem::path const& descendant)
{
    std::string p = boost::filesystem::system_complete(parent).string();
    std::string d = boost::filesystem::system_complete(descendant).string();
    std::size_t parent_len = p.size();
    return parent_len <= d.size() && p.compare(0, parent_len, d.c_str(), parent_len) == 0;
}

//! Makes one path relative to the other
inline boost::filesystem::path make_relative(boost::filesystem::path const& parent, boost::filesystem::path const& descendant)
{
    std::string p = boost::filesystem::system_complete(parent).string();
    std::string d = boost::filesystem::system_complete(descendant).string();
    std::size_t parent_len = p.size(), descendant_len = d.size();
    if (parent_len <= d.size() && p.compare(0, parent_len, d.c_str(), parent_len) == 0)
    {
        std::size_t i = parent_len;
        while (i < descendant_len && (d[i] == '/' || d[i] == '\\'))
        {
            ++i;
        }
        return boost::filesystem::path(d.c_str() + i);
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::invalid_argument("Path \"" + d + "\" is not descendant of \"" + p + "\""));
    }
}

#endif // BOOST_PKG_DEP_TREE_FILESYSTEM_EXT_HPP_INCLUDED_
