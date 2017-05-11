#ifndef INTERMEDIARY_H
#define INTERMEDIARY_H

#include <ctime>
#include <map>
#include <set>
#include <queue>
#include "toxwrapper.h"

class Intermediary : public ToxWrapper
{
public:

    /*! @brief Constructor.
     *  @param waitInterval How long to wait before resending a message.
     */
    Intermediary(double waitInterval=10);

    // Does not send a request
    void addAllowedFriend(const std::string& publicKey);

    void onFriendConnectionStatusChanged(uint32_t alias, bool online) override;

    void onMessageSentSuccess(uint32_t friendAlias, uint32_t messageId) override;

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
    };

    // The amount of time in seconds to wait before resending a message.
    double mWaitInterval;

    // Contains the data for any given friend
    std::map<uint32_t, Friend> mFriends;
    // Contains a list of the friends that need processing
    std::set<uint32_t> mWorkQueue;
};

#endif
