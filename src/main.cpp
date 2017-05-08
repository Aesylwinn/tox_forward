#include <iostream>

#include "toxwrapper.h"


using namespace std;


class Parrot : public ToxWrapper
{
public:
    void onFriendRequestRecieved(const string& publicKey, const string& message)
    {
        cout << "Incoming friend!" << endl;
        cout << "public key: " << publicKey << endl;
        cout << "message: " << message << endl << endl;

        addFriendNoRequest(publicKey);
    }

    void onMessageRecieved(uint32_t friendAlias, const string& message, bool)
    {
        cout << "Recieved message!" << endl;
        cout << "friend #: " << friendAlias << endl;
        cout << "message: " << message << endl << endl;

        if (message == "quit")
        {
            stop();
        }
        else
        {
            sendMessage(friendAlias, message);
        }
    }
};

int main(int argc, const char* argv[])
{
    Parrot parrot;

    // Setup
    parrot.setName("Polly");
    parrot.setStatusMessage("Polly wanna cracker!");

    // Connect to network
    parrot.bootstrapNode("biribiri.org", 33445,
        "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67");

    // Print address
    cout << "Address: " << parrot.getAddress() << endl << endl;

    // Main loop
    parrot.run();

    return 0;
}
