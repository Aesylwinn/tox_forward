#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <tox/tox.h>
#include <sodium.h>


using namespace std;
using namespace std::chrono;
using namespace std::this_thread;


// Forward Declarations

void handle_friend_request(Tox*, const uint8_t*, const uint8_t*, size_t, void*);
void handle_friend_message(Tox*, uint32_t, TOX_MESSAGE_TYPE, const uint8_t*,
		           size_t, void*);

struct DHTNode
{
    string ip;
    uint16_t port;
    const char hexKey[TOX_PUBLIC_KEY_SIZE*2 + 1];
    unsigned char binKey[TOX_PUBLIC_KEY_SIZE];
};

bool g_exit = false;

int main(int argc, const char* argv[])
{

    // Create tox instance

    TOX_ERR_NEW error;
    Tox* tox = tox_new(NULL, &error);
    if (error != TOX_ERR_NEW_OK)
    {
        cout << "Failed to initialize tox." << endl;
	return -1;
    }

    // Callbacks

    tox_callback_friend_request(tox, handle_friend_request);
    tox_callback_friend_message(tox, handle_friend_message);

    // Add Bootstrap nodes

    DHTNode nodes[] =
    {
        { "biribiri.org", 33445, "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67", {0} }
    };

    for (size_t i = 0; i < sizeof(nodes)/sizeof(DHTNode); ++i)
    {
        sodium_hex2bin(nodes[i].binKey, sizeof(nodes[i].binKey),
                       nodes[i].hexKey, sizeof(nodes[i].hexKey)-1, NULL, NULL, NULL);
        tox_bootstrap(tox, nodes[i].ip.c_str(), nodes[i].port, nodes[i].binKey, NULL);
    }

    // Print address

    uint8_t selfAddress[TOX_ADDRESS_SIZE];
    tox_self_get_address(tox, selfAddress);

    for (size_t i = 0; i < TOX_ADDRESS_SIZE; ++i)
    {
        cout << hex << setfill('0') << setw(2) << (int) selfAddress[i];
    }
    cout << dec << endl;

    // Main loop

    while (!g_exit)
    {
        sleep_for(milliseconds(tox_iteration_interval(tox)));
	tox_iterate(tox, NULL);
    }


    return 0;
}

void handle_friend_request(Tox* tox, const uint8_t* publicKey, const uint8_t* message,
                                  size_t length, void* userData)
{
    // Add friend

    cout << "Adding friend!" << endl;

    TOX_ERR_FRIEND_ADD error;
    tox_friend_add_norequest(tox, publicKey, &error);
    if (error != TOX_ERR_FRIEND_ADD_OK)
    {
        cout << "Failed to add friend :(" << endl;
    }
}

void handle_friend_message(Tox* tox, uint32_t friendNum,
                                  TOX_MESSAGE_TYPE type,
		                  const uint8_t* message,
				  size_t length, void* userData)
{
    // Check for quit message
    if (length == 4 && string(message, message + length) == "quit")
    {
        g_exit = true;
	return;
    }

    // Echo message

    cout << "Echoing!" << endl;

    TOX_ERR_FRIEND_SEND_MESSAGE error;
    tox_friend_send_message(tox, friendNum, type, message, length, &error);
    if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK)
    {
        cout << "Unable to parrot message X(" << endl;
    }
}
