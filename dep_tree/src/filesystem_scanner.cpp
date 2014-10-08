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
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <filesystem_scanner.hpp>
#include <cxx_parser.hpp>
#include <filesystem_ext.hpp>

namespace {

//! Matches the filename to the wildcard
bool filename_match(std::string const& filename, std::string const& wildcard)
{
    const char* f = filename.c_str(), *fend = f + filename.size();
    const char* w = wildcard.c_str(), *wend = w + wildcard.size();
    while (f != fend && w != wend)
    {
        char wc = *w++;
        switch (wc)
        {
        case '?':
            ++f;
            break;

        case '*':
            if (w == wend)
            {
                // Match the rest of the filename
                f = fend;
            }
            else
            {
                char lookup_char = *w;
                if (lookup_char == '?')
                    BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect filename pattern specified: " + wildcard));
                f = std::strchr(f, lookup_char);
                if (!f)
                    f = fend;
            }
            break;

        default:
            if (wc != *f)
                return false;
            ++f;
            break;
        }
    }

    return f == fend && w == wend;
}

//! Checks if the file name matches a wildcard in the list
bool filename_match_any(std::string const& filename, std::vector< std::string > const& wildcards)
{
    for (std::vector< std::string >::const_iterator it = wildcards.begin(), end = wildcards.end(); it != end; ++it)
    {
        if (filename_match(filename, *it))
            return true;
    }
    return false;
}


//! The function detects if the file should be parsed as C++
bool is_cxx_file(boost::filesystem::path const& path, std::vector< std::string > const& cxx_wildcards)
{
    std::string filename = path.filename().string();
    for (std::vector< std::string >::const_iterator it = cxx_wildcards.begin(), end = cxx_wildcards.end(); it != end; ++it)
    {
        if (filename_match(filename, *it))
            return true;
    }

    // Consider everything in 'include' directory C++. This is needed for boost/compatibility and boost/tr1 - headers in there are difficult to match.
    // This hack should probably be a customization point.
    for (boost::filesystem::path dir = path.parent_path(), include_dir = "include"; dir.has_parent_path(); dir = dir.parent_path())
    {
        if (dir.filename() == include_dir)
            return true;
    }

    return false;
}

//! The function scans Boost directory tree and builds header dependency tree. The function optionally detects Boost sublibraries and returns the nodes that correspond to the sublib directories.
void scan_directory(boost::filesystem::path const& dir, scan_params const& params, cxx_parser_params const& cxx_params, dep_tree& root, dep_node& node, std::vector< dep_node* >* sublibs, bool top_level)
{
    boost::filesystem::directory_iterator dir_it(dir), dir_end;
    for (; dir_it != dir_end; ++dir_it)
    {
        boost::filesystem::path path = *dir_it;
        std::string filename = path.filename().string();
        boost::filesystem::file_status status = boost::filesystem::status(path);
        if (boost::filesystem::is_directory(status))
        {
            if (top_level && std::find(params.skip_root_dirs.begin(), params.skip_root_dirs.end(), filename) != params.skip_root_dirs.end())
                continue;

            scan_directory(path, params, cxx_params, root, *node.add_child(filename), sublibs, false);
        }
        else if (boost::filesystem::is_regular(status))
        {
            if (filename_match_any(filename, params.whitelist_wildcards) && !filename_match_any(filename, params.blacklist_wildcards))
            {
                if (is_cxx_file(path, params.cxx_wildcards))
                {
                    parse_cxx(path, cxx_params, root);
                }
                else
                {
                    node.add_child(filename);
                }
            }
        }
    }
}

} // namespace

//! The function returns a wildcard that matches all files
std::vector< std::string > all_files_wildcards()
{
    static const char* const wildcards[] =
    {
        "*"
    };

    return std::vector< std::string >(wildcards, wildcards + sizeof(wildcards) / sizeof(*wildcards));
}

//! The function returns the list of directories in Boost root that should normally be skipped from scanning
std::vector< boost::filesystem::path > default_skip_root_dirs()
{
    static const char* const dirs[] =
    {
        "boost",
        ".git",
        "bin.v2",
        "stage"
    };

    return std::vector< boost::filesystem::path >(dirs, dirs + sizeof(dirs) / sizeof(*dirs));
}

//! The function returns wildcards for the filenames that are typically used to store C++ code in Boost
std::vector< std::string > default_cxx_wildcards()
{
    static const char* const wildcards[] =
    {
        "*.hpp",
        "*.cpp",
        "*.ipp",
        "*.h",
        "*.c"
    };

    return std::vector< std::string >(wildcards, wildcards + sizeof(wildcards) / sizeof(*wildcards));
}

scan_params::scan_params() : create_reverse_dependencies(false)
{
}

//! Creates scaanning parameters that are typically used to create a full directory with dependencies
scan_params scan_params::typical(boost::filesystem::path const& boost_root)
{
    scan_params params;
    params.boost_root = boost_root;
    params.whitelist_wildcards = all_files_wildcards();
    params.cxx_wildcards = default_cxx_wildcards();
    params.skip_root_dirs = default_skip_root_dirs();

    params.include_dirs.push_back(boost_root);

    return params;
}

//! The function scans Boost directory tree and builds header dependency tree. The function optionally detects Boost sublibraries and returns the nodes that correspond to the sublib directories.
void scan_filesystem_tree(boost::filesystem::path const& dir, scan_params const& params, dep_tree& root, std::vector< dep_node* >* sublibs)
{
    BOOST_ASSERT(dir.is_absolute());

    cxx_parser_params cxx_params;
    cxx_params.root_dirs.push_back(params.boost_root);
    if (!is_descendant(params.boost_root, dir))
        cxx_params.root_dirs.push_back(dir);
    cxx_params.include_dirs = params.include_dirs;
    cxx_params.create_reverse_dependencies = params.create_reverse_dependencies;
    scan_directory(dir, params, cxx_params, root, root, sublibs, true);
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
