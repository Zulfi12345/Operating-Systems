import socket
import threading
import time
import argparse

class Node:
    def __init__(self, data):
        self.data = data
        self.next = None
        self.book_next = None
        self.book_id = None
        self.next_frequent_search = None


class SharedList:
    """A thread-safe shared multi-link list for storing nodes."""
    def __init__(self):
        self.lock = threading.Lock()
        self.head = None

    def add_node(self, data, connection_id):
        """Add a new node with `data` to the shared list."""
        new_node = Node(data)
        new_node.book_id = connection_id
        with self.lock:
            if self.head is None:
                self.head = new_node
            else:
                curr = self.head
                while curr.next is not None:
                    curr = curr.next
                curr.next = new_node
            print(f"Added node to book: {connection_id}")

    def get_data(self, book_id):
        """Retrieve data for a specific book (connection) based on book_id."""
        lines = []
        node = self.head
        while node:
            if node.book_id == book_id:
                lines.append(node.data)
            node = node.next
        return lines

    def get_all_data(self):
        """Retrieve all data in the shared list, grouped by book_id."""
        books_data = {}
        node = self.head
        while node:
            if node.book_id not in books_data:
                books_data[node.book_id] = []
            books_data[node.book_id].append(node.data)
            node = node.next
        return books_data


def client_handler(connection, address, shared_list, connection_id):
    """Handle the client connection."""
    print(f"Connected to client: {address}")
    connection.setblocking(False)  # non-blocking mode
    buffer = ""
    
    try:
        while True:
            try:
                raw_data = connection.recv(1024)
                if not raw_data:
                    break  # No more data from the client; exit loop
                
                # Decode data received
                try:
                    decoded_data = raw_data.decode("utf-8")
                except UnicodeDecodeError:
                    print("Received undecodable data")
                    continue
                
                buffer += decoded_data
                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)
                    shared_list.add_node(line.strip(), connection_id)
                    print(f"Added line to shared list for connection {connection_id}")
            
            except BlockingIOError:
                time.sleep(0.1)  # Sleep for 100ms before checking again
                continue  # No data available
    
    finally:
        # Write collected data to a file when the connection closes
        with open(f"book_{connection_id:02}.txt", "w") as file:
            for line in shared_list.get_data(connection_id):
                file.write(line + "\n")
        
        connection.close()
        print(f"Connection closed for client: {address}")


def analysis_thread(shared_list, search_pattern, interval):
    """Periodically analyze data in shared_list for a search pattern."""
    printed_results = set()
    output_lock = threading.Lock()


    while True:
        with output_lock:
            # Gather data from shared list
            books_data = shared_list.get_all_data()
            frequency_data = []
            
            # Calculate frequency of search pattern for each book
            for book_id, lines in books_data.items():
                frequency_count = sum(line.count(search_pattern) for line in lines)
                frequency_data.append((book_id, frequency_count))
            
            # Sort books by frequency count in descending order
            frequency_data.sort(key=lambda x: x[1], reverse=True)
            
            # Output the result 
            for rank, (book_id, frequency) in enumerate(frequency_data, start=1):
                result = (book_id, frequency)
                if result not in printed_results:
                    print(f"{rank} --> Book: {book_id}, Pattern: \"{search_pattern}\", Frequency: {frequency}")
                    printed_results.add(result)  # Mark as printed
        
        # Wait for the next interval
        time.sleep(interval)


def main():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="network server.")
    parser.add_argument("-l", "--listen_port", type=int, required=True)
    parser.add_argument("-p", "--pattern", type=str, required=True, help="Search pattern for analysis threads.")
    parser.add_argument("-i", "--interval", type=int, default=5, help="Interval (seconds) for output.")
    args = parser.parse_args()

    # Extract the listening port, search pattern, and interval
    listen_port = args.listen_port
    search_pattern = args.pattern
    interval = args.interval

    # Create server socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', listen_port)) 
    server_socket.listen(10)
    print(f"Server listening on port {listen_port}")

    shared_list = SharedList()
    connection_count = 0

    # Start analysis threads
    for _ in range(2):  # Create two analysis threads
        thread = threading.Thread(target=analysis_thread, args=(shared_list, search_pattern, interval))
        thread.start()

    # Main server loop to accept client connections
    while True:
        # Accept incoming connection
        client_socket, address = server_socket.accept()
        connection_count += 1

        # Create and start a new thread for each client
        thread = threading.Thread(target=client_handler, args=(client_socket, address, shared_list, connection_count))
        thread.start()


if __name__ == "__main__":
    main()
