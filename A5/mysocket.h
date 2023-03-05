int mul(int a, int b);
my_socket

// my_socket – This function opens a standard TCP socket with the socket call. It also creates two threads R and S (to be described later), allocates and initializes space for two tables Send_Message and Received_Message (to be described later), and any additional space that may be needed. The parameters to these are the same as the normal socket( ) call, except that it will take only SOCK_MyTCP as the socket type.
// my_bind – binds the socket with some address-port using the bind call.
// my_listen – makes a listen call.
// my_accept – accepts a connection on the MyTCP socket by making the accept call on the TCP socket (only on server side)
// my_connect – opens a connection through the MyTCP socket by making the connect call on the TCP socket
// my_send – sends a message (see description later). One message is defined as what is sent in one my_send call.
// my_recv – receives a message. This is a blocking call and will return only if a message, sent using the my_send call, is received. If the buffer length specified is less than the message length, the extra bytes in the message are discarded (similar to that in UDP).
// my_close – closes the socket and cleans up everything. If there are any messages to be sent/received, they are discarded.