#ifndef CMDLINE_H
#define CMDLINE_H

#include <string>

void parseCmdLineArgs(int argc, char* argv[], std::string& cfgFile,
                      std::string& dataDir);

#endif
