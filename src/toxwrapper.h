#ifndef TOXWRAPPER_H
#define TOXWRAPPER_H

#include <string>
#include <tox/tox.h>

/*! @brief Wraps a tox instance.
 */
class ToxWrapper
{
public:

    /*! @brief Initializes a tox instance.
     */
    ToxWrapper();

    /*! @brief Destroys the associated tox instance.
     */
    ~ToxWrapper();


    /*! @brief Attemps to connect to a node, see the tox documentation.
     *  @param address The ip adress or domain name of the node.
     *  @param port The port the node is listening on.
     *  @param publicKey The public key of the node in hexadecimal.
     *  @return True on success.
     */
    bool bootstrapNode(const std::string& address, uint16_t port,
                       const std::string& publicKey);


    /*! @brief Returns whether or not this instance is online.
     *  @return True if connected.
     */
    bool isConnected();

    /*! @brief Returns the Tox ID of this instance.
     *  @return The Tox ID in hexadecimal.
     */
    std::string getAddress();

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
    bool setStatusMessage(const std::string& name);


    /*! @brief Adds a friend, sending them a request.
     *  @param address The Tox ID of the friend in hexadecimal.
     *  @param message The message to send.
     *  @return The alias for the friend if successful, otherwise UINT32_MAX.
     */
    uint32_t addFriend(const std::string& address, const std::string& message);

    /*! @brief Adds the friend without sending a request.
     *  @param publicKey The public key of the friend in hexadecimal.
     *  @return The alias for the friend if successful, otherwise UINT32_MAX.
     */
    uint32_t addFriendNoRequest(const std::string& publicKey);

    /*! @brief Removes the friend from the friend list.
     *  @param alias The alias for the friend.
     *  @return True on success.
     */
    bool deleteFriend(uint32_t alias);

    // TODO at line 1300

private:

    Tox* mTox;
};

#endif
