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

inline void add_include(boost::string_ref const& header, dep_tree& root, dep_node& node, bool create_reverse_dependencies)
{
    if (header.starts_with("boost"))
    {
        dep_node* other = root.add_nested_child(header);
        node.add_dependency(other);
        if (create_reverse_dependencies)
        {
            other->add_dependent(&node);
        }
    }
}

//! The function creates a node for a header and fills its dependencies depending on the header contents
void parse_cxx_source(boost::string_ref const& source, dep_tree& root, dep_node& node, bool create_reverse_dependencies)
{
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
                            continue;

                        add_include(boost::string_ref(p, q - p), root, node, create_reverse_dependencies);
                        p = q + 1;
                    }
                }
            }
            continue;

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
            continue;

        case '"':
        case '\'':
            p = find_closing_quote_with_escapes(p + 1, end, c);
            if (p != end)
                ++p;
            continue;

        default:
            ++p;
            continue;
        }
    }
}

} // namespace

//! The function creates a node for a header and fills its dependencies depending on the header contents
void parse_cxx(const char* path, dep_tree& root, bool create_reverse_dependencies)
{
    try
    {
        boost::interprocess::file_mapping file(path, boost::interprocess::read_only);
        boost::interprocess::mapped_region region(file, boost::interprocess::read_only);

        dep_node* node = root.add_nested_child(path);

        parse_cxx_source(boost::string_ref(static_cast< const char* >(region.get_address()), region.get_size()), root, *node, create_reverse_dependencies);
    }
    catch (boost::interprocess::interprocess_exception& e)
    {
        BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error(std::string("Failed to open file for parsing: ") + e.what())) << file_name_info(path));
    }
    catch (boost::exception& e)
    {
        e << file_name_info(path);
        throw;
    }
    catch (std::exception& e)
    {
        throw boost::enable_error_info(e) << file_name_info(path);
    }
}
