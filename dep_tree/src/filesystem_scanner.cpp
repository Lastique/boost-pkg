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
#include <iterator>
#include <stdexcept>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <filesystem_scanner.hpp>

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

//! The function detects if the file should be parsed as C++
bool is_cxx_file(boost::filesystem::path const& path, std::vector< std::string > const& cxx_wildcards)
{
    std::string filename = path.filename().string();
    for (std::vector< std::string >::const_iterator it = cxx_wildcards.begin(), end = cxx_wildcards.end(); it != end; ++it)
    {
        if (filename_match(filename, *it))
            return true;
    }

    if (path.has_parent_path())
    {
        // Consider everything in 'include' directory C++. This is needed for boost/compatibility and boost/tr1 - headers in there are difficult to match.
        // This hack should probably be a customization point.
        typedef std::reverse_iterator< boost::filesystem::path::const_iterator > iterator;

        boost::filesystem::path dir = path.parent_path();
        iterator begin(dir.end()), end(dir.begin());
        if (std::find(begin, end, boost::filesystem::path("include")) != end)
            return true;
    }

    return false;
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
        "*.h",
        "*.ipp"
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
void scan_filesystem_tree(scan_params const& params, dep_tree& root, std::vector< dep_node* >* sublibs)
{
    BOOST_ASSERT(params.boost_root.is_absolute());

    boost::filesystem::directory_iterator dir_it(params.boost_root), dir_end;
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
