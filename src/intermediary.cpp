#include "intermediary.h"
#include <algorithm>
#include <array>
#include <cassert>


Intermediary::Intermediary(const ToxOptionsWrapper& opts, double waitInterval)
    : ToxWrapper(opts)
    , mWaitInterval(waitInterval)
{
    // Add default allowed commands
    mValidCommands.push_back("alias");
    mValidCommands.push_back("forward");
    mValidCommands.push_back("help");
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

std::string escapeMessage(const std::string& original)
{
    if (!original.empty() && original[0] == '!')
    {
        return "!" + original;
    }
    else
    {
        return original;
    }
}

void Intermediary::onMessageRecieved(uint32_t alias, const std::string& message,
                                     bool actionType)
{
    // Process the message based on the type.
    if (messageIsCommand(message))
    {
        processCommand(alias, message);
    }
    else
    {
        sendStandardMessage(alias, mFriends[alias].currentReciever,
			    escapeMessage(message));
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

std::string readArg(const std::string& str, size_t& start)
{
    if (start >= str.size())
    {
        return "";
    }

    // Read the next arg and setup for the one that folows
    std::string arg;
    size_t end = str.find_first_of(' ', start);
    if (end != std::string::npos)
    {
        // Don't include the space
        arg = str.substr(start, end - start);
        start = end + 1;
    }
    else
    {
        arg = str.substr(start);
        start = str.size();
    }

    return arg;
}

bool Intermediary::messageIsCommand(const std::string& message) const
{
    // Basic preliminary test
    if (message.size() < 2 || message[0] != '!')
    {
        return false;
    }

    // Determine if the command is valid
    size_t start = 1;
    std::string command = readArg(message, start);
    for (auto it = mValidCommands.begin(); it != mValidCommands.end(); ++it)
    {
        if (command == *it)
        {
            return true;
        }
    }

    return false;
}

void Intermediary::processCommand(uint32_t from, const std::string& message)
{
    Friend& f = mFriends[from];

    // Process command
    size_t start = 1;
    std::string command = readArg(message, start);

    if (command == "alias")
    {
        std::string name = readArg(message, start);
        std::string publicKey = readArg(message, start);

        // Validate
        if (name.empty() || publicKey.empty())
        {
            sendServerMessage(from, "Use !help to see the description for "
                                    "how to use the alias command.");
        }
        else if (!friendExists(getFriendByPublicKey(publicKey)))
        {
            sendServerMessage(from, "Unknown tox id passed to the alias "
                                    "command.");
        }
        else
        {
            // Convert to lower case
            std::transform(publicKey.begin(), publicKey.end(),
			   publicKey.begin(), ::tolower);

            // Make sure to only store the public key
            publicKey.resize(2 * getPublicKeySize(), '0');

	    // Store mappings
            f.aliases[name] = publicKey;
            f.reverseAliases[publicKey] = name;
        }
    }
    else if (command == "forward")
    {
        std::string recipient = readArg(message, start);
        uint32_t reciever = UINT32_MAX;

        // Figure out who will recieve the message
        if (f.aliases.find(recipient) != f.aliases.end())
        {
            reciever = getFriendByPublicKey(f.aliases[recipient]);
        }
        else
        {
            reciever = getFriendByPublicKey(recipient);
        }

        // Process if valid
        if (!friendExists(reciever))
        {
            sendServerMessage(from, "Unknown alias or tox id sent to the "
                                    "forward command.");
        }
        else
        {
            f.currentReciever = reciever;
        }
    }
    else if (command == "help")
    {
        sendServerMessage(from, "Commands: alias, forward, help\n"
                                "!alias <nickname> <tox id> - associates a "
                                "name with a tox id if the server knows them\n"
                                "!forward <alias> - will forward messages to "
                                "an assigned alias\n"
                                "!forward <tox id> - will forward messages to "
                                "a tox id if the server knows them\n"
                                "!help - displays some helpful information");
    }
    else
    {
        // Unhandled command.
        assert(false);
    }
}

void Intermediary::sendStandardMessage(uint32_t from, uint32_t to,
                                       const std::string& message)
{
    if (friendExists(to))
    {
        // Regular message

        Friend& sender = mFriends[from];
        Friend& reciever = mFriends[to];

        // Alert client to who is sending
        if (reciever.lastSender != sender.alias)
        {
            // Retrieve user defined name if available, otherwise tox id
            std::string name = getFriendPublicKey(sender.alias);
            if (reciever.reverseAliases.find(name) !=
                    reciever.reverseAliases.end())
            {
                name = reciever.reverseAliases[name];
            }

	    reciever.lastSender = sender.alias;
            reciever.unrecievedMessages.push("!sender " + name);
        }

        reciever.unrecievedMessages.push(message);

        // Send the message if they are online.
        if (isFriendConnected(reciever.alias))
        {
            mWorkQueue.insert(reciever.alias);
        }
    }
    else
    {
        sendServerMessage(from, "No reciever specified");
    }
}

void Intermediary::sendServerMessage(uint32_t to, const std::string& message)
{
    Friend& reciever = mFriends[to];
    reciever.unrecievedMessages.push("!server " + message);

    if (isFriendConnected(reciever.alias))
    {
        mWorkQueue.insert(reciever.alias);
    }
}
