# README

## Tox-Forward

### Project Summary
This project makes it possible for users to send messages to each other using
the tox protocol even while one of the users is offline. It does this by
acting as an intermediary. A sender passes their message to the intermediary.
The intemediary then waits for the reciever to go online before passing on the
message.

### Current State of Development
The core functionality has been mostly implemented, but there still remains
some work to be done. Preferably, the intermediary would be passed an encrypted
message that only the reciever can decrypt. Additionally, the commands for the
intermediary should be simplified. (etc...)

### Usage
Required libraries:
 * toxcore (The tok-tok variant)
 * libsodium
 * libconfig

The project can be built on linux using make. This produces an executable
called tox-forwardd.

In order to run tox-forwardd, you need to pass it a config file and a data
directory like so:

`tox-forwardd --config tox-forwardd.conf --datadir .`

Friends and bootstrap nodes should be specified in the config file. Once
executed, tox-forwardd will output an address that the clients will need
to befriend.

