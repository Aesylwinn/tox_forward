#include "intermediary.h"


Intermediary::Intermediary(const ToxOptionsWrapper& opts, double waitInterval)
    : ToxWrapper(opts)
    , mWaitInterval(waitInterval)
{
}

void Intermediary::addAllowedFriend(const std::string& publicKey)
{
    uint32_t alias = addFriendNoRequest(publicKey);

    // Check for an error
    if (alias != UINT32_MAX)
    {
        mFriends[alias].alias = alias;
    }
}

void Intermediary::onFriendConnectionStatusChanged(uint32_t alias, bool online)
{
    Friend& f = mFriends[alias];

    // Add or remove the friend to/from the work queue
    if (online && !f.unrecievedMessages.empty())
    {
        mWorkQueue.insert(alias);
    }
    else if (!online)
    {
        mWorkQueue.erase(alias);
    }
}

void Intermediary::onMessageSentSuccess(uint32_t alias, uint32_t messageId)
{
    Friend& f = mFriends[alias];

    // TODO: There is the potential for there to be an issue once messageId
    // goes back to zero.
    if (messageId >= f.lastMessageMinId && !f.lastMessageSentSuccessful)
    {
        f.lastMessageSentSuccessful = true;
        f.unrecievedMessages.pop();

        // Check if any work remains
        if (f.unrecievedMessages.empty())
        {
            mWorkQueue.erase(f.alias);
        }
    }
}

void Intermediary::onMessageRecieved(uint32_t alias, const std::string& message,
                                     bool actionType)
{
    Friend& sender = mFriends[alias];

    // Process the message based on the type.
    // TODO add a random component to the the commands to make it more difficult
    // to crack
    if (message.substr(0, 9) == "_forward:")
    {
        // A new reciever is being designated.
        std::string recieverKey = message.substr(9); // TODO limit size
        sender.currentReciever = getFriendByPublicKey(recieverKey);
    }
    else if (friendExists(sender.currentReciever))
    {
        // A regular message is being sent.
        Friend& reciever = mFriends[sender.currentReciever];

        if (reciever.lastSender != sender.alias)
        {
            // Alert client they are getting a new sender.
            reciever.unrecievedMessages.push("_sender:");// + getFriendPublicKey(sender.alias));
            reciever.lastSender = sender.alias;
        }

        reciever.unrecievedMessages.push(message);

        // Add the sender to the work queue if online
        if (isFriendConnected(reciever.alias))
        {
            mWorkQueue.insert(reciever.alias);
        }
    }
}

void Intermediary::onCoreUpdate()
{
    // Perform work
    std::time_t now = std::time(nullptr);
    for (auto workIt = mWorkQueue.begin(); workIt != mWorkQueue.end(); ++workIt)
    {
        Friend& f = mFriends[*workIt];

        if (f.lastMessageSentSuccessful)
        {
            // Send the next message
            f.lastMessageSentSuccessful = false;
            f.lastMessageMinId = sendMessage(f.alias,
                                             f.unrecievedMessages.front());
            f.lastMessageTimeStamp = now;
        }
        else if (std::difftime(now, f.lastMessageTimeStamp) > mWaitInterval)
        {
            // Try resending
            sendMessage(f.alias, f.unrecievedMessages.front());
            f.lastMessageTimeStamp = now;
        }
    }
}
