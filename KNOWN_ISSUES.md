1. HARDWARE: 6.35" jacks: In normal use this is NOT an issue, however it should be documented...

When ordering the components I did not understand about breadboarded jack sockets isolating themselves to ground (the proper term is "switched (normally-closed)"). When applying power via the jack sockets to the RPU-3 this is no issue. Unfortunately it turns out in my use case, when no RPU-3 is connected, or the jack is unplugged, the Pico immediately powers down.

Investigation showed that the switched contacts inside the jack socket connect the 3.3 V line to another contact when no plug is inserted, effectively shorting the supply.

As a temporary fix I bent the normally-closed spring contact away from its mating contact. This works, but because the contact can still bounce while the plug is being removed, brief shorts can still occur.

Planned permanent fix: insulate the unused switched contact with Kapton tape or hot glue, or replace the socket with a non-switched jack if appropriate.

I will update this issue when it's fixed :D
