#include "toxwrapper.h"

#include <cassert>
#include <sodium.h>
#include <vector>

ToxWrapper::ToxWrapper()
    : mTox(nullptr)
{
    // Defaults for now
    Tox_Options* options = tox_options_new(nullptr);
    assert(options);

    // Create tox instance
    mTox = tox_new(options, nullptr);
    assert(mTox);

    // Clean up resources
    tox_options_free(options);
}

ToxWrapper::~ToxWrapper()
{
    // Destroy tox instance
    tox_kill(mTox);
}

bool ToxWrapper::bootstrapNode(const std::string& address, uint16_t port,
                               const std::string& publicKeyHex)
{
    // Convert the hexadecimal key to binary
    std::vector<uint8_t> publicKeyBin(tox_public_key_size(), 0);
    assert(!sodium_hex2bin(&publicKeyBin[0], tox_public_key_size(),
                           publicKeyHex.data(), publicKeyHex.size(), nullptr,
                           nullptr, nullptr));

    // Attempt to bootstrap the node
    return tox_bootstrap(mTox, address.c_str(), port, &publicKeyBin[0],
                         nullptr);
}

bool ToxWrapper::isConnected()
{
    TOX_CONNECTION status = tox_self_get_connection_status(mTox);
    return (status != TOX_CONNECTION_NONE);
}

std::string ToxWrapper::getAddress()
{
    // Retrieve the binary address
    std::vector<uint8_t> addressBin(tox_address_size(), 0);
    tox_self_get_address(mTox, &addressBin[0]);

    // Convert from binary to hexadecimal
    std::string addressHex;
    addressHex.resize(tox_address_size() * 2 + 1); // +1 for null character
    assert(!sodium_bin2hex(&addressHex[0], addressHex.size(), &addressBin[0],
                           tox_address_size()));
    addressHex.pop_back(); // remove null character

    return addressHex;
}

std::string ToxWrapper::getName()
{
    // Retrieve the name in utf8
    std::vector<uint8_t> rawName(tox_self_get_name_size(mTox), 0);
    tox_self_get_name(mTox, &rawName[0]);

    // Return as a string
    return std::string(rawName.begin(), rawName.end());
}

bool ToxWrapper::setName(const std::string& name)
{
    // Convert to uint8_t format
    std::vector<uint8_t> rawName(name.begin(), name.end());

    // Set name
    return tox_self_set_name(mTox, &rawName[0], rawName.size(), nullptr);
}

std::string ToxWrapper::getStatusMessage()
{
    // Retrieve the message
    std::vector<uint8_t> rawMessage(tox_self_get_status_message_size(mTox));
    tox_self_get_status_message(mTox, &rawMessage[0]);

    // Return as a string
    return std::string(rawMessage.begin(), rawMessage.end());
}

bool ToxWrapper::setStatusMessage(const std::string& message)
{
    // Convert to uint8_t format
    std::vector<uint8_t> rawMessage(message.begin(), message.end());

    // Set message
    return tox_self_set_status_message(mTox, &rawMessage[0], rawMessage.size(),
                                       nullptr);
}

uint32_t ToxWrapper::addFriend(const std::string& address,
                               const std::string& message)
{
    // Convert hex to binary
    std::vector<uint8_t> addressBinary(tox_address_size(), 0);
    assert(!sodium_hex2bin(&addressBinary[0], tox_address_size(),
                           address.data(), address.size(), nullptr,
                           nullptr, nullptr));

    // Convert message to uint8_t form
    std::vector<uint8_t> rawMessage(message.begin(), message.end());

    // Add friend
    return tox_friend_add(mTox, &addressBinary[0], &rawMessage[0],
                          rawMessage.size(), nullptr);
}

uint32_t ToxWrapper::addFriendNoRequest(const std::string& publicKey)
{
    // Convert hex to binary
    std::vector<uint8_t> publicKeyBin(tox_public_key_size(), 0);
    assert(!sodium_hex2bin(&publicKeyBin[0], tox_public_key_size(),
                           publicKey.data(), publicKey.size(), nullptr,
                           nullptr, nullptr));

    // Add friend
    return tox_friend_add_norequest(mTox, &publicKeyBin[0], nullptr);
}

bool ToxWrapper::deleteFriend(uint32_t alias)
{
    // They must be DESTROYED!!!
    return tox_friend_delete(mTox, alias, nullptr);
}
