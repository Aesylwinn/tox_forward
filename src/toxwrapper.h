#ifndef TOXWRAPPER_H
#define TOXWRAPPER_H

#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <tox/tox.h>

/*! @brief Wraps the creation options for a tox instance.
 */
class ToxOptionsWrapper
{
public:

    /*! @brief The supported types of proxies.
     */
    enum ProxyType
    {
        PT_None,
        PT_Http,
        PT_Socks5
    };


    /*! @brief Constructor.
     */
    ToxOptionsWrapper();

    /*! @brief Destructor.
     */
    ~ToxOptionsWrapper();


    // No copy/assignment allowed
    ToxOptionsWrapper(const ToxOptionsWrapper&) = delete;
    ToxOptionsWrapper& operator=(const ToxOptionsWrapper&) = delete;


    /*! @brief Enables ipv6 addressing.
     *  @param enable True to enable, false to disable.
     */
    void enableIpv6(bool enable);

    /*! @brief Enables the usage of udp.
     *  @param enable True to enable, false to disable.
     */
    void enableUdp(bool enable);

    /*! @brief Enables peer discovery on LAN networks.
     *  @param enable True to enable, false to disable.
     */
    void enableLocalDiscovery(bool enable);

    /*! @brief Determines the type of proxy to be used.
     *  @param type The type of proxy.
     */
    void setProxyType(ProxyType type);

    /*! @brief Sets the host used for the proxy.
     *  @param address The ip address or DNS name of the host. The size of this
     *                 address is limited to 255 characters.
     */
    void setProxyHost(const std::string& address);

    /*! @brief Sets the port used for the proxy.
     *  @param port The port number.
     */
    void setProxyPort(uint16_t port);

    /*! @brief Specifies the range of ports that should be used by the tox
     *         instance.
     *  @param start The first port in the inclusive range.
     *  @param end The last port in the inclusive range.
     */
    void setPortRange(uint16_t start, uint16_t end);

    /*! @brief Enables UDP hole punching.
     *  @param enable True to enable, false to disable.
     */
    void enableHolePunching(bool enable);

    /*! @brief Loads saved information from a previous tox instance.
     *  @param data The data to load created using ToxWrapper::save().
     */
    void loadSaveData(std::istream& data);

private:

    Tox_Options* mOptions;
    std::string mProxyHost;
    std::vector<uint8_t> mSaveData;

    friend class ToxWrapper;
};


/*! @brief Can be used to store the various types of keys used in tox.
 */
class ToxKey
{
public:

    /*! @brief The type of key. Imposes a size constraint.
     */
    enum Type
    {
        None,
        Address,
        Public,
        Secret
    };

    /*! @brief Thrown when the key size is invalid for the type.
     */
    class InvalidSize
    {
    public:
        /*! @brief Constructor.
         *  @param given The given length of the key.
         *  @param expected The expected length of the key.
         */
        InvalidSize(size_t given, size_t expected);

        /*! @brief The error message in a human readable format.
         */
        std::string getMessage() const;

        /*! @brief The size of the key given.
         */
        size_t getGivenSize() const;

        /*! @brief The expected size of the key.
         */
        size_t getExpectedSize() const;

    private:

        std::string mMessage;
        size_t mGiven, mExpected;
    };

    /*! @brief Default constructor.
     */
    ToxKey();

    /*! @brief Constructs the key from a hex string.
     *  @param type The type of key.
     *  @param hex The key in hexadecimal.
     */
    ToxKey(Type type, const std::string& hex);

    /*! @brief Constructs the key from a byte array.
     *  @param type The type of key.
     *  @param bin The key in binary.
     */
    ToxKey(Type type, const std::vector<uint8_t>& bin);

    /*! @brief Retrieves the key type.
     */
    Type getType() const;

    /*! @brief Retrieves the hexadecimal version of the key.
     */
    const std::string& getHex() const;

    /*! @brief Retrieves the binary version of the key.
     */
    const std::vector<uint8_t>& getBin() const;

    bool operator<(const ToxKey& other) const;

private:

    void validate();

    Type mType;
    std::string mHex;
    std::vector<uint8_t> mBin;
};

namespace std
{
    template<> struct hash<ToxKey>
    {
        typedef ToxKey argument_type;
        typedef size_t result_type;

        result_type operator()(argument_type const& key) const
        {
            return hash<string>{}(key.getHex());
        }
    };
}


/*! @brief Wraps a tox instance. This is based largely off of the tox api, look
 *         there for more documentation.
 */
class ToxWrapper
{
public:

    /*! @brief Initializes a tox instance.
     *  @param options The options to be set when creating the tox instance.
     */
    ToxWrapper(const ToxOptionsWrapper& options);

    /*! @brief Destroys the associated tox instance.
     */
    virtual ~ToxWrapper();


    // No copy/assignment allowed
    ToxWrapper(const ToxWrapper&) = delete;
    ToxWrapper& operator=(const ToxWrapper&) = delete;


    /*! @brief Returns the number of bytes used to store a public key.
     */
    uint32_t getPublicKeySize() const;

    /*! @brief Returns the number of bytes used to store a secret key.
     */
    uint32_t getSecretKeySize() const;

    /*! @brief Returns the number of bytes used to store an address.
     */
    uint32_t getAddressSize() const;

    /*! @brief Returns the number of bytes used to store a hash.
     */
    uint32_t getHashSize() const;

    /*! @brief Returns the number of bytes used to store file id.
     */
    uint32_t getFileIDSize() const;

    /*! @brief Returns the maximum size of a name in bytes.
     */
    uint32_t getMaxNameSize() const;

    /*! @brief Returns the maximum size of a status message in bytes.
     */
    uint32_t getMaxStatusMessageSize() const;

    /*! @brief Returns the maximum size of a friend request message in bytes.
     */
    uint32_t getMaxFriendRequestSize() const;

    /*! @brief Returns the maximum size of a message in bytes.
     */
    uint32_t getMaxMessageSize() const;

    /*! @brief Returns the maximum size of a custom packet in bytes.
     */
    uint32_t getMaxCustomPacketSize() const;

    /*! @brief Returns the maximum size of a file name for file transfers in
     *         bytes.
     */
    uint32_t getMaxFileNameSize() const;


    /*! @brief Attemps to connect to a node, see the tox documentation.
     *  @param address The ip adress or domain name of the node.
     *  @param port The port the node is listening on.
     *  @param publicKey The public key of the node.
     *  @return True on success.
     */
    bool bootstrapNode(const std::string& address, uint16_t port,
                       const ToxKey& publicKey);

    /*! @brief Saves the current instance data so it can be recreated later.
     *  @param str The stream to save the instance to.
     */
    void save(std::ostream& str);


    /*! @brief Returns whether or not this instance is online.
     *  @return True if connected.
     */
    bool isConnected();

    /*! @brief Returns the Tox ID of this instance.
     *  @return The Tox ID in hexadecimal.
     */
    ToxKey getAddress();

    /*! @brief Returns the name of the Tox instance.
     *  @return The name in utf8.
     */
    std::string getName();

    /*! @brief Sets the name of the Tox instance.
     *  @param name The desired name in utf8.
     *  @return True on success.
     */
    bool setName(const std::string& name);

    /*! @brief Gets the status message of the Tox instance.
     *  @return The status message in utf8.
     */
    std::string getStatusMessage();

    /*! @brief Sets the status message of the Tox instance.
     *  @param message The status message in utf8.
     *  @return True on success.
     */
    bool setStatusMessage(const std::string& message);


    /*! @brief Adds a friend, sending them a request.
     *  @param address The Tox ID of the friend.
     *  @param message The message to send.
     *  @return The alias for the friend if successful, otherwise UINT32_MAX.
     */
    uint32_t addFriend(const ToxKey& address, const std::string& message);

    /*! @brief Adds the friend without sending a request.
     *  @param publicKey The public key of the friend.
     *  @return The alias for the friend if successful, otherwise UINT32_MAX.
     */
    uint32_t addFriendNoRequest(const ToxKey& publicKey);

    /*! @brief Returns the alias for a specific friend.
     *  @param publicKey The public key of the friend.
     *  @return The alias for the friend if succussful, otherwise UINT32_MAX.
     */
    uint32_t getFriendByPublicKey(const ToxKey& publicKey);

    /*! @brief Returns whether an alias has been mapped to a friend. Can be used
     *         for validation.
     *  @param alias The alias to check.
     *  @return True if the the friend exists.
     */
    bool friendExists(uint32_t alias);

    /*! @brief Removes the friend from the friend list.
     *  @param alias The alias for the friend.
     *  @return True on success.
     */
    bool deleteFriend(uint32_t alias);


    /*! @brief Returns the public key of a specific friend.
     *  @param alias The alias for the friend.
     */
    ToxKey getFriendPublicKey(uint32_t alias);

    /*! @brief Returns whether or not a specific friend is online.
     *  @param alias
     *  @return True if the friend is connected.
     */
    bool isFriendConnected(uint32_t alias);

    /*! @brief Sends a message to a specific friend.
     *  @param friendAlias The alias for a friend.
     *  @param message The message. Must be shorter than TOX_MAX_MESSAGE_LENGTH.
     *  @param actionType Whether or not the message is an action (/me for ex.).
     *  @return The unique message identifier for the given friend. Can be used
     *          to verify the message sent was recieved.
     */
    uint32_t sendMessage(uint32_t friendAlias, std::string message,
                         bool actionType=false);


    /*! @brief Called when the connection status changes.
     *  @param online True if we are online.
     */
    virtual void onConnectionStatusChanged(bool online);

    /*! @brief Called when a friend request is recieved.
     *  @param publicKey The public key of the sender.
     *  @param message The message sent with the request.
     */
    virtual void onFriendRequestRecieved(const ToxKey& publicKey,
                                         const std::string& message);

    /*! @brief Called when a friend's name is changed.
     *  @param alias The alias for the friend.
     *  @param name The new name.
     */
    virtual void onFriendNameChanged(uint32_t alias, const std::string& name);

    /*! @brief Called when a friend's status message is changed.
     *  @param alias The alias for the friend.
     *  @param message The new status message.
     */
    virtual void onFriendStatusMessageChanged(uint32_t alias,
                                              const std::string& message);

    /*! @brief Called when the connection status of a friend is changed.
     *  @param alias The alias for the friend.
     *  @param online True if the friend is online.
     */
    virtual void onFriendConnectionStatusChanged(uint32_t alias, bool online);

    /*! @brief Called when the reciept for a sent message is recieved.
     *  @param friendAlias The alias for the friend the message was sent to.
     *  @param messageId The unique id (for the friend) of the message sent.
     */
    virtual void onMessageSentSuccess(uint32_t friendAlias, uint32_t messageId);

    /*! @brief Called when a message from a friend is recieved.
     *  @param friendAlias The alias for the friend.
     *  @param message The message recieved.
     *  @param actionType Whether or not the message is an action (/me for ex.).
     */
    virtual void onMessageRecieved(uint32_t friendAlias,
                                   const std::string& message, bool actionType);

    /*! @brief Called after each update to the Tox instance.
     */
    virtual void onCoreUpdate();


    /*! @brief Executes main loop for Tox instance. Currently not thread safe.
     */
    void run();

    /*! @brief Stops the main loop started by calling run().
     */
    void stop();

private:

    Tox* mTox;
    bool mStop;
};

#endif
