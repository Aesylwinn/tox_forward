#ifndef INTERMEDIARY_H
#define INTERMEDIARY_H

#include <ctime>
#include <map>
#include <set>
#include <queue>
#include "toxwrapper.h"

/*! @brief Forwards messages sent by one friend to another.
 */
class Intermediary : public ToxWrapper
{
public:

    /*! @brief Constructor.
     *  @param options Configurations options for the underlying tox instance.
     *  @param waitInterval How long to wait before resending a message.
     */
    Intermediary(const ToxOptionsWrapper& options, double waitInterval=10);

    /*! @brief Sets up a friend to recieve forwarded messages, does not send a
     *         request.
     *  @param publicKey The public key of the friend.
     */
    void addAllowedFriend(const std::string& publicKey);

    void onFriendConnectionStatusChanged(uint32_t alias, bool online) override;

    void onMessageSentSuccess(uint32_t friendAlias,
                              uint32_t messageId) override;

    void onMessageRecieved(uint32_t friendAlias, const std::string& message,
                           bool actionType) override;

    void onCoreUpdate() override;

private:

    /*! @brief Contains the messages and other important data for a friend.
     */
    struct Friend
    {
        /*! @brief The alias that maps to this friend.
         */
        uint32_t alias = UINT32_MAX;

        /*! @brief The alias of the last friend to add a message to the queue.
         */
        uint32_t lastSender = UINT32_MAX;

        /*! @brief The alias of the friend to whom messages are being sent.
         */
        uint32_t currentReciever = UINT32_MAX;

        /*! @brief The queued up messages (and other information, such as a
         *         change in sender) that have yet to be delivered.
         */
        std::queue<std::string> unrecievedMessages;

        /*! @brief The status of the last message sent.
         */
        bool lastMessageSentSuccessful = true;

        /*! @brief The starting unique id of the last message sent to this
         *         friend. According to the tox documentation, this number will
         *         increment linearly.
         */
        uint32_t lastMessageMinId = UINT32_MAX;

        /*! @brief The time stamp of the last message sent to this friend.
         */
        std::time_t lastMessageTimeStamp;

        /*! @brief The user defined aliases for different friends. The name
         *         is the key.
         */
        std::map<std::string, std::string> aliases;

        /*! @brief The used defined aliases for different friends. The public
         *         key is the key.
         */
        std::map<std::string, std::string> reverseAliases;
    };

    /*! @brief Determines if a message is a command. A command is any message
     *         starting with a '!' followed by a specific keyword. The current
     *         keywords can be queried using !help.
     *  @param message The recieved message.
     */
    bool messageIsCommand(const std::string& message) const;

    /*! @brief Processes a command.
     *  @param from The alias of the sender.
     *  @param message The entire command string.
     */
    void processCommand(uint32_t from, const std::string& message);

    /*! @brief Sends a regular message from one user to another.
     *  @param from The alias of the sender.
     *  @param to The alias of the reciever.
     *  @param message The message to send.
     */
    void sendStandardMessage(uint32_t from, uint32_t to,
                             const std::string& message);

    /*! @brief Sends a server message to a user.
     *  @param to The alias of the reciever.
     *  @param message The message to send.
     */
    void sendServerMessage(uint32_t to, const std::string& message);

    // The amount of time in seconds to wait before resending a message.
    double mWaitInterval;

    // Contains the data for any given friend
    std::map<uint32_t, Friend> mFriends;
    // Contains a list of the friends that need processing
    std::set<uint32_t> mWorkQueue;

    std::vector<std::string> mValidCommands;
};

#endif
