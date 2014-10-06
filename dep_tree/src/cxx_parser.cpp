/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines implementation for the C++ files parser
 */

#include <cstring>
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/enable_error_info.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/filesystem/operations.hpp>
#include <cxx_parser.hpp>

namespace {

inline const char* skip_one_line_comment(const char* p, const char* end)
{
    while (true)
    {
        const char* q = static_cast< const char* >(std::memchr(p, '\n', end - p));
        if (!q)
            break;

        const char* res = q + 1;

        // Check if the comment extends to the next line
        if (q > p && *(q - 1) == '\r')
            --q;
        if (q > p && *(q - 1) == '\\')
            p = res;
        else
            return res;
    }

    return end;
}

inline const char* skip_multi_line_comment(const char* p, const char* end)
{
    while (true)
    {
        const char* res = static_cast< const char* >(std::memchr(p, '*', end - p));
        if (!res || (end - res) < 2)
            break;
        ++res;
        if (*res == '/')
            return ++res;
    }

    return end;
}

inline const char* skip_spaces(const char* p, const char* end)
{
    while (p != end)
    {
        char c = *p;
        if (c == ' ' || c == '\t')
            break;
        ++p;
    }

    return p;
}

inline const char* find_closing_quote(const char* p, const char* end, char quote_char)
{
    const char* res = static_cast< const char* >(std::memchr(p, quote_char, end - p));
    return res ? res : end;
}

inline const char* find_closing_quote_with_escapes(const char* p, const char* end, char quote_char)
{
    while (true)
    {
        const char* q = find_closing_quote(p, end, quote_char);
        if (q != end && q > p && (*q - 1) == '\\')
        {
            p = q + 1;
            continue;
        }

        return q;
    }
}

//! Obtains the status of the file, ignoring any level of symlinks
inline boost::filesystem::file_status peel_symlinks_status(boost::filesystem::path& path)
{
    boost::filesystem::file_status file_stat = boost::filesystem::status(path);
    while (boost::filesystem::is_symlink(file_stat))
    {
        path = boost::filesystem::read_symlink(path);
        file_stat = boost::filesystem::status(path);
    }
    return file_stat;
}

void add_include(boost::string_ref const& included_header, dep_tree& root, dep_node& node, boost::filesystem::path const& header_dir, std::vector< boost::filesystem::path > const& include_dirs, bool use_header_dir, bool create_reverse_dependencies)
{
    boost::filesystem::path path(included_header.to_string());
    boost::filesystem::path full_path;

    boost::filesystem::file_status file_stat;
    if (use_header_dir)
    {
        full_path = header_dir / path;
        file_stat = peel_symlinks_status(full_path);
    }

    std::vector< boost::filesystem::path >::const_iterator it = include_dirs.begin(), end = include_dirs.end();
    for (; it != end && !boost::filesystem::is_regular_file(file_stat); ++it)
    {
        full_path = *it / path;
        file_stat = peel_symlinks_status(full_path);
    }

    if (boost::filesystem::is_regular_file(file_stat))
    {
        dep_node* other = root.add_nested_child(included_header);
        node.add_dependency(other);
        if (create_reverse_dependencies)
        {
            other->add_dependent(&node);
        }
    }
}

//! The function creates a node for a header and fills its dependencies depending on the header contents
void parse_cxx_source(boost::string_ref const& source, dep_tree& root, dep_node& node, boost::filesystem::path const& header_dir, std::vector< boost::filesystem::path > const& include_dirs, bool create_reverse_dependencies)
{
    bool first_char_in_line = true;
    const char* p = source.data(), * const end = p + source.size();
    while (p != end)
    {
        p = skip_spaces(p, end);
        if (p == end)
            break;

        char c = *p;
        switch (c)
        {
        case '#':
            {
                ++p;
                if (first_char_in_line)
                {
                    p = skip_spaces(p, end);
                    if ((end - p) > sizeof("include<>") && std::memcmp(p, "include", sizeof("include") - 1) == 0)
                    {
                        p = skip_spaces(p + sizeof("include") - 1, end);
                        if (p != end)
                        {
                            const char* q;
                            c = *p++;
                            if (c == '<')
                                q = find_closing_quote(p, end, '>');
                            else if (c == '"')
                                q = find_closing_quote(p, end, '"');
                            else
                                break;

                            add_include(boost::string_ref(p, q - p), root, node, header_dir, include_dirs, c == '"', create_reverse_dependencies);
                            p = q + 1;
                        }
                    }
                }
            }
            break;

        case '/':
            {
                ++p;
                if (p != end)
                {
                    c = *p++;
                    if (c == '/')
                        p = skip_one_line_comment(p, end);
                    else if (c == '*')
                        p = skip_multi_line_comment(p, end);
                }
            }
            break;

        case '\\':
            {
                // Handle line continuation. Tolerate spaces after the backslash.
                p = skip_spaces(p + 1, end);
                if (p != end && *p == '\r')
                    ++p;
                if (p != end && *p == '\n')
                {
                    ++p;
                    continue;
                }
            }
            break;

        case '\n':
            ++p;
            first_char_in_line = true;
            continue;

        case '"':
        case '\'':
            p = find_closing_quote_with_escapes(p + 1, end, c);
            if (p != end)
                ++p;
            break;

        default:
            ++p;
            break;
        }

        first_char_in_line = false;
    }
}

} // namespace

//! The function creates a node for a header and fills its dependencies depending on the header contents
void parse_cxx(boost::filesystem::path const& path, dep_tree& root, std::vector< boost::filesystem::path > const& include_dirs, bool create_reverse_dependencies)
{
    std::string path_str = path.string();
    try
    {
        boost::interprocess::file_mapping file(path_str.c_str(), boost::interprocess::read_only);
        boost::interprocess::mapped_region region(file, boost::interprocess::read_only);

        dep_node* node = root.add_nested_child(path_str);

        parse_cxx_source(boost::string_ref(static_cast< const char* >(region.get_address()), region.get_size()), root, *node, path.parent_path(), include_dirs, create_reverse_dependencies);
    }
    catch (boost::interprocess::interprocess_exception& e)
    {
        BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(std::string("Failed to open file for parsing: ") + e.what())) << file_name_info(path_str));
    }
    catch (boost::exception& e)
    {
        e << file_name_info(path_str);
        throw;
    }
    catch (std::exception& e)
    {
        throw boost::enable_error_info(e) << file_name_info(path_str);
    }
}
