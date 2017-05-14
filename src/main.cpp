#include <fstream>
#include <iostream>
#include <libconfig.h++>
#include "cmdline.h"
#include "intermediary.h"


using namespace std;
using namespace libconfig;

int main(int argc, char* argv[])
{
    ToxOptionsWrapper options;
    bool newInstance = true;
    string instFileName = "instance.tox";
    string cfgFileName;
    string dataDirName;
    fstream saveFile;

    // Process cmd arguments
    parseCmdLineArgs(argc, argv, cfgFileName, dataDirName);
    instFileName.insert(0, dataDirName);

    // Use saved data if it exists
    saveFile.open(instFileName.c_str(), ios_base::in);
    if (saveFile.is_open())
    {
        cout << "Found saved data." << endl;
        // Save data exists
        newInstance = false;
        options.loadSaveData(saveFile);
    }
    saveFile.close();


    // Start
    Intermediary forwarder(options);

    // Save to file
    if (newInstance)
    {
        cout << "Saving to file." << endl;
        saveFile.open(instFileName.c_str(), ios_base::out);
        forwarder.save(saveFile);
        saveFile.close();
    }

    // Setup
    Config cfg;

    try
    {
        cfg.readFile(cfgFileName.c_str());
    }
    catch (ParseException& pe)
    {
        cout << "parse error: " << pe.getFile() << ":" << pe.getLine() << " ";
        cout << pe.getError() << endl;
        exit(1);
    }
    catch (FileIOException& fe)
    {
        cout << "file error: failed to read " << cfgFileName << endl;
        exit(1);
    }

    string name;
    if (cfg.lookupValue("name", name))
    {
        forwarder.setName(name);
    }

    string statusMessage;
    if (cfg.lookupValue("status", statusMessage))
    {
        forwarder.setStatusMessage(statusMessage);
    }

    if (cfg.exists("friends"))
    {
        Setting& friends = cfg.lookup("friends");
        for (auto it = friends.begin(); it != friends.end(); ++it)
        {
            forwarder.addAllowedFriend(it->c_str());
        }
    }

    if (cfg.exists("nodes"))
    {
        Setting& nodes = cfg.lookup("nodes");
        for (auto node = nodes.begin(); node != nodes.end(); ++node)
        {
            bool valid = true;
            string address;
            unsigned port;
            string key;

            valid &= node->lookupValue("address", address);
            valid &= node->lookupValue("port", port);
            valid &= node->lookupValue("key", key);

            if (valid)
            {
                forwarder.bootstrapNode(address, (uint16_t)port, key);
            }
        }
    }

    // Print address
    cout << "Address: " << forwarder.getAddress() << endl << endl;

    // Main loop
    forwarder.run();

    return 0;
}
