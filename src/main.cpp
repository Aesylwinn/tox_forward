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
    forwarder.setName("Forwarder");
    forwarder.setStatusMessage("Greetings stranger!");

    // Connect to network
    forwarder.bootstrapNode("biribiri.org", 33445,
        "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67");


    // Print address
    cout << "Address: " << forwarder.getAddress() << endl << endl;

    // Main loop
    forwarder.run();

    return 0;
}
