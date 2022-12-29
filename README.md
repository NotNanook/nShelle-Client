# nShelle-Client
Currently works by listening to a host (`hostServer`) on a specific port (`hostServerPort`)<br>
Commands are send through TCP/Socket connection and are unencrypted so its easily detectable

## How to use
1. Start the server
2. Compile and build the client (With changed IP and Port)
3. Send the executable to the victim
4. Wait for connection

## Detectability
Did some really basic String obfuscation so you cant see suspicious strings when doing really basic analysis<br>
Warning: This is only the payload so there is no build-in persistency
