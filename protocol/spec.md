# Specification of the *Doppelkopf AI Protocol*

## Transport Layer

The protocol can be implemented on top of any reliably, in-order data stream between a client and a server. Such
connections include:

  - TCP connections
  - UNIX stream sockets
  - Pipes and Stdio
