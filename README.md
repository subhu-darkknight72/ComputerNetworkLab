# ComputerNetworkLab
## Contributors
- 20CS30019: Gitanjali Gupta
- 20CS10064: Subhajyoti Halder

## Assignment 1
- **Question 1:** A <code>iterative TCP server</code> to allow client programs to get the system date and time from the server. The client displays the date and time on the screen, and terminates.
- **Question 2:** Write a simple TCP iterative server and client to evaluate arithmetic expressions

## Assignment 2
- **Question 1:** A <code>UDP client-server</code> system to make the time server. However, the client uses the poll() function to wait for 3 seconds to receive on the UDP socket with timeout, else exits with “Timeout exceeded” message.
- **Question 2:** The client interacts with the <code>concurrent TCP server</code> using a communication protocol: LOGIN, run valid shell commands on the server system, and display the output on the screen. Connection terminates when the client sends the command “quit” to the server.

## Assignment 3
- **Question 1:** Create a simplified <code>Load Balancer</code> that balances incoming client requests between two TCP iterative servers based on their current loads, using a dummy load. The load balancer should be a concurrent TCP server that allows multiple copies to run simultaneously using a command-line parameter for the port number. Clients connect to the load balancer and receive the date/time from the server with the least load.

## Assignment 4
- **Question 1:** Implement a simplified <code>TCP concurrent HTTP client</code> and server that supports basic HTTP commands GET and PUT for downloading and uploading html, text, and pdf files.The client opens a TCP connection with the HTTP server and retrieves the document from the HTTP server upon receiving a GET command. The server listens for incoming requests and responds to them.


<!-- .
## Instruction
- **Create virtual environment**
```bash
sudo pip install virtualenv      # This may already be installed
virtualenv .env                  # Create a virtual environment
```
- **Run** start.sh **bash To Start Web Application**
```bash
./start.sh                       # All neccessary library will be downloaded
```
- **Open http://127.0.0.1:8000 in  your browser**
. -->