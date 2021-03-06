#include "toxwrapper.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <map>
#include <thread>
#include <sodium.h>
#include <vector>


// Utility functions

std::string convertToHex(const uint8_t* binary, size_t length)
{
    // Setup
    std::string hex(length * 2 + 1, '0');

    // Convert. sodium_bin2hex returns nullptr on failure.
    assert(sodium_bin2hex(&hex[0], hex.size(), &binary[0], length));
    hex.pop_back(); // remove null character

    return hex;
}

std::vector<uint8_t> convertToBinary(const std::string& hex)
{
    // Setup
    std::vector<uint8_t> binary(hex.size() / 2, 0);

    // Convert. sodium_hex2bin returns 0 on success.
    size_t hexSize = hex.size() - (hex.size() % 2);
    assert(!sodium_hex2bin(&binary[0], binary.size(), &hex[0], hexSize,
                           nullptr, nullptr, nullptr));

    return binary;
}


/*! @brief Used to map callbacks to specific instances.
 */
class ToxWrapperRegistry
{
public:
    /*! @brief Registers a specific wrapper tied to a specific instance.
     *  @param instance The Tox instance.
     *  @param wrapper The wrapper.
     */
    void registerWrapper(Tox* instance, ToxWrapper* wrapper);

    /*! @brief Unregisters the wrapper tied to a specific instance.
     *  @param instance The Tox instance to be unmapped.
     */
    void unregisterWrapper(Tox* instance);

    /*! @brief Returns the wrapper tied to a specific instance. Assumes it
     *         exists.
     *  @param instance The Tox instance the wrapper has been tied to.
     */
    ToxWrapper* lookup(Tox* instance);

    /*! @brief Returns the global instance of the ToxWrapperRegistry class
     */
    static ToxWrapperRegistry& get();

private:
    ToxWrapperRegistry() = default;
    ~ToxWrapperRegistry() = default;

    std::map<Tox*, ToxWrapper*> mRegistrations;
};


// A bunch of callbacks

void self_connection_status_changed(Tox* tox, TOX_CONNECTION status,
                                    void* user_data)
{
    bool online = (status != TOX_CONNECTION_NONE);
    ToxWrapperRegistry::get().lookup(tox)->onConnectionStatusChanged(online);
}

void friend_request(Tox* tox, const uint8_t* publicKeyBin,
                    const uint8_t* rawMessage, size_t length, void* userData)
{
    ToxKey publicKey(ToxKey::Public, std::vector<uint8_t>(publicKeyBin,
                     publicKeyBin+length));
    std::string message(rawMessage, rawMessage+length);
    ToxWrapperRegistry::get().lookup(tox)->onFriendRequestRecieved(publicKey,
                                                                   message);
}

void friend_name_changed(Tox* tox, uint32_t alias, const uint8_t* rawName,
                         size_t length, void* user_data)
{
    std::string name(rawName, rawName+length);
    ToxWrapperRegistry::get().lookup(tox)->onFriendNameChanged(alias, name);
}

void friend_status_message_changed(Tox* tox, uint32_t alias,
                                   const uint8_t* rawMessage, size_t length,
                                   void *userData)
{
    std::string message(rawMessage, rawMessage+length);
    ToxWrapperRegistry::get().lookup(tox)->
        onFriendStatusMessageChanged(alias, message);
}

void friend_connection_status_changed(Tox* tox, uint32_t alias,
                                      TOX_CONNECTION status, void* userData)
{
    bool online = (status != TOX_CONNECTION_NONE);
    ToxWrapperRegistry::get().lookup(tox)->
        onFriendConnectionStatusChanged(alias, online);
}

void friend_read_reciept(Tox* tox, uint32_t alias, uint32_t messageId,
                                  void *user_data)
{
    ToxWrapperRegistry::get().lookup(tox)->onMessageSentSuccess(alias,
                                                                messageId);
}

void friend_message(Tox* tox, uint32_t alias, TOX_MESSAGE_TYPE type,
                    const uint8_t* rawMessage, size_t length, void* userData)
{
    std::string message(rawMessage, rawMessage+length);
    bool actionType = (type == TOX_MESSAGE_TYPE_ACTION);
    ToxWrapperRegistry::get().lookup(tox)->onMessageRecieved(alias, message,
                                                             actionType);
}


// The ToxWrapperRegistry implementation

void ToxWrapperRegistry::registerWrapper(Tox* tox, ToxWrapper* wrapper)
{
    // Register
    mRegistrations[tox] = wrapper;

    // Add callbacks
    tox_callback_self_connection_status(tox, self_connection_status_changed);
    tox_callback_friend_name(tox, friend_name_changed);
    tox_callback_friend_status_message(tox, friend_status_message_changed);
    tox_callback_friend_connection_status(tox,
                                          friend_connection_status_changed);
    tox_callback_friend_read_receipt(tox, friend_read_reciept);
    tox_callback_friend_message(tox, friend_message);
    tox_callback_friend_request(tox, friend_request);
}

void ToxWrapperRegistry::unregisterWrapper(Tox* instance)
{
    mRegistrations.erase(instance);
}

ToxWrapper* ToxWrapperRegistry::lookup(Tox* instance)
{
    return mRegistrations[instance];
}

ToxWrapperRegistry& ToxWrapperRegistry::get()
{
    static ToxWrapperRegistry instance;
    return instance;
}


// The ToxOptionsWrapper implementation

ToxOptionsWrapper::ToxOptionsWrapper()
{
    mOptions = tox_options_new(nullptr);
    assert(mOptions);
}

ToxOptionsWrapper::~ToxOptionsWrapper()
{
    tox_options_free(mOptions);
}

void ToxOptionsWrapper::enableIpv6(bool enable)
{
    tox_options_set_ipv6_enabled(mOptions, enable);
}

void ToxOptionsWrapper::enableUdp(bool enable)
{
    tox_options_set_udp_enabled(mOptions, enable);
}

void ToxOptionsWrapper::enableLocalDiscovery(bool enable)
{
    tox_options_set_local_discovery_enabled(mOptions, enable);
}

void ToxOptionsWrapper::setProxyType(ProxyType type)
{
    switch (type)
    {
    case PT_None:
        tox_options_set_proxy_type(mOptions, TOX_PROXY_TYPE_NONE);
        break;
    case PT_Http:
        tox_options_set_proxy_type(mOptions, TOX_PROXY_TYPE_HTTP);
        break;
    case PT_Socks5:
        tox_options_set_proxy_type(mOptions, TOX_PROXY_TYPE_SOCKS5);
        break;
    }
}

void ToxOptionsWrapper::setProxyHost(const std::string& address)
{
    assert(address.size() < 256);
    mProxyHost = address;
    tox_options_set_proxy_host(mOptions, mProxyHost.c_str());
}

void ToxOptionsWrapper::setProxyPort(uint16_t port)
{
    tox_options_set_proxy_port(mOptions, port);
}

void ToxOptionsWrapper::setPortRange(uint16_t start, uint16_t end)
{
    tox_options_set_start_port(mOptions, start);
    tox_options_set_end_port(mOptions, end);
}

void ToxOptionsWrapper::enableHolePunching(bool enable)
{
    tox_options_set_hole_punching_enabled(mOptions, enable);
}

void ToxOptionsWrapper::loadSaveData(std::istream& data)
{
    // TODO improve the file format
    data.seekg(0, data.end);
    size_t length = data.tellg();
    data.seekg(0, data.beg);

    mSaveData.resize(length, 0);
    data.read((char*)&mSaveData[0], length);

    tox_options_set_savedata_type(mOptions, TOX_SAVEDATA_TYPE_TOX_SAVE);
    tox_options_set_savedata_length(mOptions, mSaveData.size());
    tox_options_set_savedata_data(mOptions, &mSaveData[0], mSaveData.size());
}


// The ToxKey implementation

ToxKey::InvalidSize::InvalidSize(size_t given, size_t expected)
    : mGiven(given)
    , mExpected(expected)
{
    mMessage = "Invalid key size. Expected key of size " +
               std::to_string(expected) + " but given key of size " +
               std::to_string(given) + ".";
}

std::string ToxKey::InvalidSize::getMessage() const
{
    return mMessage;
}

size_t ToxKey::InvalidSize::getGivenSize() const
{
    return mGiven;
}

size_t ToxKey::InvalidSize::getExpectedSize() const
{
    return mExpected;
}

ToxKey::ToxKey()
    : mType(None)
{
}

ToxKey::ToxKey(Type type, const std::string& hex)
    : mType(type)
    , mHex(hex)
    , mBin(convertToBinary(hex))
{
    std::transform(mHex.begin(), mHex.end(), mHex.begin(), ::tolower);
    validate();
}

ToxKey::ToxKey(Type type, const std::vector<uint8_t>& bin)
    : mType(type)
    , mHex(convertToHex(bin.data(), bin.size()))
    , mBin(bin)
{
    std::transform(mHex.begin(), mHex.end(), mHex.begin(), ::tolower);
    validate();
}

ToxKey::Type ToxKey::getType() const
{
    return mType;
}

const std::string& ToxKey::getHex() const
{
    return mHex;
}

const std::vector<uint8_t>& ToxKey::getBin() const
{
    return mBin;
}

bool ToxKey::operator<(const ToxKey& other) const
{
    return (*this).mBin < other.mBin;
}

void ToxKey::validate()
{
    size_t given = mBin.size();
    size_t expected = 0;

    switch (mType)
    {
    case Address:
        expected = tox_address_size();
        break;
    case Public:
        expected = tox_public_key_size();
        break;
    case Secret:
        expected = tox_secret_key_size();
        break;
    case None:
        expected = given;
    }

    if (expected > given)
    {
        mType = None;
        throw InvalidSize(given, expected);
    }
    else
    {
        // Trim off extra characters
        mBin.resize(expected);
        mHex.resize(expected * 2);
    }
}


// The ToxWrapper implementation

ToxWrapper::ToxWrapper(const ToxOptionsWrapper& options)
    : mTox(nullptr)
    , mStop(false)
{
    // Create tox instance
    mTox = tox_new(options.mOptions, nullptr);
    assert(mTox);

    // Register
    ToxWrapperRegistry::get().registerWrapper(mTox, this);
}

ToxWrapper::~ToxWrapper()
{
    // Destroy tox instance
    tox_kill(mTox);

    // Unregister
    ToxWrapperRegistry::get().unregisterWrapper(mTox);
}

uint32_t ToxWrapper::getPublicKeySize() const
{
    return tox_public_key_size();
}

uint32_t ToxWrapper::getSecretKeySize() const
{
    return tox_secret_key_size();
}

uint32_t ToxWrapper::getAddressSize() const
{
    return tox_address_size();
}

uint32_t ToxWrapper::getHashSize() const
{
    return tox_hash_length();
}

uint32_t ToxWrapper::getFileIDSize() const
{
    return tox_file_id_length();
}

uint32_t ToxWrapper::getMaxNameSize() const
{
    return tox_max_name_length();
}

uint32_t ToxWrapper::getMaxStatusMessageSize() const
{
    return tox_max_status_message_length();
}

uint32_t ToxWrapper::getMaxFriendRequestSize() const
{
    return tox_max_friend_request_length();
}

uint32_t ToxWrapper::getMaxMessageSize() const
{
    return tox_max_message_length();
}

uint32_t ToxWrapper::getMaxCustomPacketSize() const
{
    return tox_max_custom_packet_size();
}

uint32_t ToxWrapper::getMaxFileNameSize() const
{
    return tox_max_filename_length();
}

bool ToxWrapper::bootstrapNode(const std::string& address, uint16_t port,
                               const ToxKey& publicKey)
{
    // Attempt to bootstrap the node
    assert(publicKey.getType() == ToxKey::Public);
    return tox_bootstrap(mTox, address.c_str(), port,
                         publicKey.getBin().data(), nullptr);
}

void ToxWrapper::save(std::ostream& str)
{
    // Read in data from tox
    size_t length = tox_get_savedata_size(mTox);

    std::vector<uint8_t> data;
    data.resize(length, 0);
    tox_get_savedata(mTox, &data[0]);

    // Write to stream
    str.write((char*)&data[0], length);
}

bool ToxWrapper::isConnected()
{
    TOX_CONNECTION status = tox_self_get_connection_status(mTox);
    return (status != TOX_CONNECTION_NONE);
}

ToxKey ToxWrapper::getAddress()
{
    // Retrieve the binary address
    std::vector<uint8_t> addressBin(tox_address_size(), 0);
    tox_self_get_address(mTox, &addressBin[0]);

    return ToxKey(ToxKey::Address, addressBin);
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

uint32_t ToxWrapper::addFriend(const ToxKey& address,
                               const std::string& message)
{
    // Setup
    assert (address.getType() == ToxKey::Address);
    std::vector<uint8_t> rawMessage(message.begin(), message.end());

    // Add friend
    return tox_friend_add(mTox, address.getBin().data(), &rawMessage[0],
                          rawMessage.size(), nullptr);
}

uint32_t ToxWrapper::addFriendNoRequest(const ToxKey& publicKey)
{
    // Add friend
    assert(publicKey.getType() == ToxKey::Public);
    return tox_friend_add_norequest(mTox, publicKey.getBin().data(), nullptr);
}

uint32_t ToxWrapper::getFriendByPublicKey(const ToxKey& publicKey)
{
    // Retrieve alias
    assert(publicKey.getType() == ToxKey::Public);
    return tox_friend_by_public_key(mTox, publicKey.getBin().data(),
                                    nullptr);
}

bool ToxWrapper::friendExists(uint32_t alias)
{
    return tox_friend_exists(mTox, alias);
}

bool ToxWrapper::deleteFriend(uint32_t alias)
{
    // They must be DESTROYED!!!
    return tox_friend_delete(mTox, alias, nullptr);
}

ToxKey ToxWrapper::getFriendPublicKey(uint32_t alias)
{
    // Retrieve key
    std::vector<uint8_t> rawKey(tox_public_key_size(), 0);
    tox_friend_get_public_key(mTox, alias, &rawKey[0], nullptr);

    return ToxKey(ToxKey::Public, rawKey);
}

bool ToxWrapper::isFriendConnected(uint32_t alias)
{
    // Check if the friend is online.
    TOX_CONNECTION status;
    status = tox_friend_get_connection_status(mTox, alias, nullptr);

    return (status != TOX_CONNECTION_NONE);
}

uint32_t ToxWrapper::sendMessage(uint32_t friendAlias, std::string message,
                                 bool actionType)
{
    // Select message type
    TOX_MESSAGE_TYPE messageType = (actionType) ? TOX_MESSAGE_TYPE_ACTION :
                                                  TOX_MESSAGE_TYPE_NORMAL;

    // Convert message fo uint8_t form
    std::vector<uint8_t> rawMessage(message.begin(), message.end());

    // Send the message
    return tox_friend_send_message(mTox, friendAlias, messageType,
                                   &rawMessage[0], rawMessage.size(), nullptr);
}

void ToxWrapper::onConnectionStatusChanged(bool online)
{
}

void ToxWrapper::onFriendRequestRecieved(const ToxKey& publicKey,
                                         const std::string& message)
{
}

void ToxWrapper::onFriendNameChanged(uint32_t alias, const std::string& name)
{
}

void ToxWrapper::onFriendStatusMessageChanged(uint32_t alias,
                                              const std::string& message)
{
}

void ToxWrapper::onFriendConnectionStatusChanged(uint32_t alias, bool online)
{
}

void ToxWrapper::onMessageSentSuccess(uint32_t friendAlias, uint32_t messageId)
{
}

void ToxWrapper::onMessageRecieved(uint32_t friendAlias,
                                   const std::string& message, bool actionType)
{
}

void ToxWrapper::onCoreUpdate()
{
}

void ToxWrapper::run()
{
    mStop = false;
    while (!mStop)
    {
        // Wait until the next update is required
        std::this_thread::sleep_for(
            std::chrono::milliseconds(tox_iteration_interval(mTox)));

        // Let tox do its work
        tox_iterate(mTox, nullptr);

        // Callback
        onCoreUpdate();
    }
}

void ToxWrapper::stop()
{
    mStop = true;
}
