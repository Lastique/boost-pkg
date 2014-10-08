/*
 *             Copyright Andrey Semashev 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * This file contains implementation of the boost-dep utility
 */

#include <cstddef>
#include <vector>
#include <string>
#include <locale>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <json.hpp>
#include <dep_tree.hpp>
#include <filesystem_scanner.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    try
    {
        std::locale::global(std::locale::classic());

        // Command line parameters definition
        po::options_description general_options("General options");
        general_options.add_options()
            ("help", "produce this help message");

        po::options_description input_options("Input options");
        input_options.add_options()
            ("scan-dir,s", po::value< std::string >(), "directory to scan")
            ("include,I", po::value< std::vector< std::string > >()->composing(), "directories to search included headers in")
            ("boost-root", po::value< std::string >(), "Boost root directory");

        po::options_description output_options("Output options");
        output_options.add_options()
            ("output,o", po::value< std::string >(), "output file (stdout by default)")
            ("format,f", po::value< std::string >()->default_value("json"), "output format (json by default)");

        po::options_description options("boost-dep options");
        options.add(general_options).add(input_options).add(output_options);

        po::positional_options_description positional_options;
        positional_options.add("scan-dir", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(options).positional(positional_options).run(), vm);
        po::notify(vm);

        // Command line arguments processing
        if (vm.count("help"))
        {
            std::cout << options << std::endl;
            return 0;
        }

        boost::filesystem::path scan_dir;
        const po::variable_value* arg = &vm["scan-dir"];
        if (!arg->empty())
        {
            scan_dir = arg->as< std::string >();
            scan_dir = boost::filesystem::system_complete(scan_dir);
        }
        else
        {
            scan_dir = boost::filesystem::current_path();
        }

        boost::filesystem::path boost_root;
        arg = &vm["boost-root"];
        if (!arg->empty())
        {
            boost_root = arg->as< std::string >();
            boost_root = boost::filesystem::system_complete(boost_root);
        }
        else
        {
            boost_root = find_boost_root();
        }

        scan_params params = scan_params::typical(boost_root);

        arg = &vm["include"];
        if (!arg->empty())
        {
            std::vector< std::string > includes = arg->as< std::vector< std::string > >();
            std::vector< boost::filesystem::path > include_dirs;
            std::copy(includes.begin(), includes.end(), std::back_inserter(include_dirs));
            include_dirs.insert(include_dirs.end(), params.include_dirs.begin(), params.include_dirs.end());
            params.include_dirs.swap(include_dirs);
        }

        std::ofstream file;
        std::ostream* output = &std::cout;
        arg = &vm["output"];
        if (!arg->empty())
        {
            std::string out_fname = arg->as< std::string >();
            file.open(out_fname.c_str(), std::ios::out | std::ios::trunc);
            if (!file.is_open())
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open output file: " + out_fname));
            output = &file;
        }

        std::string out_format = vm["format"].as< std::string >();
        if (out_format != "json")
            BOOST_THROW_EXCEPTION(std::invalid_argument("Unsupported output format: " + out_format));

        // Filesystem scanning
        dep_tree root;

        scan_filesystem_tree(scan_dir, params, root);

        // Saving the result
        if (out_format == "json")
            serialize_json(root, *output);
    }
    catch (std::exception& e)
    {
        std::cerr << "Failure: " << boost::diagnostic_information(e) << std::endl;
        return 1;
    }

    return 0;
}
