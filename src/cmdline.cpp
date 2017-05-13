#include "cmdline.h"
#include <iostream>
#include <getopt.h>

using namespace std;

void parseCmdLineArgs(int argc, char* argv[], std::string& cfgFile,
                      std::string& dataDir)
{
    const char* shortOptions = "c:";

    option longOptions[] =
    {
        { "config",  required_argument, 0, 'c' },
        { "datadir", required_argument, 0, 'd' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c=getopt_long(argc, argv, shortOptions, longOptions, nullptr))!=-1)
    {
        switch (c)
        {
            case 'c':
                cfgFile = optarg;
                break;

            case 'd':
                dataDir = optarg;
                break;

            case '?':
                cout << "Unexpected option " << argv[optind-1] << endl;
                break;

            case ':':
                cout << "Expected argument for " << argv[optind - 1] << endl;
                exit(1);
                break;
        }
    }

    if (cfgFile.empty())
    {
        cout << "error: --config option was not specified." << endl;
        exit(1);
    }
    if (dataDir.empty())
    {
        cout << "error: --datadir options was not specified." << endl;
        exit(1);
    }

    // Ensure dataDir is a directory
    if (dataDir.back() != '/')
    {
        dataDir.push_back('/');
    }
}
