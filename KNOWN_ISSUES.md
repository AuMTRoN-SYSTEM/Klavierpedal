1. HARDWARE: 6.35" jacks: In normal use this is NOT an issue, however it should be documented...

When ordering the components I did not understand about breadboarded jack sockets isolating themselves to ground (or whatever the proper term is). When providing power to the RPU-3 this is no issue. Unfortunately it turns out in my use case, with no RPU-3 connected, or when I pull out the jack plug for any reason, the 3.3v line gets immediately grounded (?) which causes some kind of short, and the Pico does some safety thing and powers off (or something).

When I realised this fact, I immediately rectified it by bending the middle (ring) contact spring up slightly, which prevents the spring resting on the other side contact. However I did not consider spring bounce, and it occasionally happens when the jack is pulled quickly.

The fix is simple: Cover the opposite contacts entirely with tape or some hot glue. I will update this issue when it's fixed :D