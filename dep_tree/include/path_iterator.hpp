/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This header defines an iterator for parsing dependency tree path
 */

#ifndef BOOST_PKG_DEP_TREE_PATH_ITERATOR_HPP_INCLUDED_
#define BOOST_PKG_DEP_TREE_PATH_ITERATOR_HPP_INCLUDED_

#include <iterator>
#include <algorithm>
#include <boost/config.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/string_ref.hpp>

class path_iterator :
    public boost::iterator_facade<
        path_iterator,
        boost::string_ref,
        std::forward_iterator_tag,
        boost::string_ref
    >
{
    friend class boost::iterator_core_access;

private:
    const char* m_pos;
    const char* m_sep;
    const char* m_end;
    char m_separator;

public:
    path_iterator(boost::string_ref const& str, char separator) BOOST_NOEXCEPT :
        m_pos(str.data()), m_end(str.data() + str.size()), m_separator(separator)
    {
        m_sep = std::find(m_pos, m_end, separator);
    }

    path_iterator(path_iterator const& that) BOOST_NOEXCEPT :
        m_pos(that.m_pos), m_sep(that.m_sep), m_end(that.m_end), m_separator(that.m_separator)
    {
    }

    path_iterator& operator= (path_iterator const& that) BOOST_NOEXCEPT
    {
        m_pos = that.m_pos;
        m_sep = that.m_sep;
        m_end = that.m_sep;
        m_separator = that.m_separator;
    }

    static path_iterator make_end(boost::string_ref const& str, char separator) BOOST_NOEXCEPT
    {
        path_iterator p;
        p.m_pos = p.m_sep = p.m_end = str.data() + str.size();
        p.m_separator = separator;
        return p;
    }

private:
    BOOST_DEFAULTED_FUNCTION(path_iterator(), {})

    boost::string_ref dereference() const BOOST_NOEXCEPT
    {
        return boost::string_ref(m_pos, m_sep - m_pos);
    }

    void increment() BOOST_NOEXCEPT
    {
        m_pos = m_sep;
        if (m_pos != m_end)
            ++m_pos;
        m_sep = std::find(m_pos, m_end, m_separator);
    }

    bool equal(path_iterator const& that) const BOOST_NOEXCEPT
    {
        return m_pos == that.m_pos;
    }
};

#endif // BOOST_PKG_DEP_TREE_PATH_ITERATOR_HPP_INCLUDED_
