#include <iostream>
#include "intermediary.h"


using namespace std;

int main(int argc, const char* argv[])
{
    Intermediary forwarder;

    // Setup
    forwarder.setName("Forwarder");
    forwarder.setStatusMessage("Tired");

    // Connect to network
    forwarder.bootstrapNode("biribiri.org", 33445,
        "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67");


    // Print address
    cout << "Address: " << uppercase << forwarder.getAddress() << nouppercase << endl << endl;

    // Main loop
    forwarder.run();

    return 0;
}
