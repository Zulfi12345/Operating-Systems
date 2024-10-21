import socket
import threading
import os
import argparse

class Node:
    def __init__(self, line, bookID):
        self.line = line
        self.next = None
        self.nextBook = None
        self.bookID = bookID


class SharedList:
    def __init__ (self):
        self.head = None
        self.lock = threading.Lock()

    def addNode(self, line, bookID):
        newNode = Node(line, bookID)

        with self.lock:

            if self.head is None:
                self.head = newNode
            else:
                curr= self.head
                while curr.next is not None:
                    curr = curr.next
                curr.next = newNode

            print(f"Added line to book {bookID}: {line.strip()}")



    def writer(self, bookID):
        fileName = f"book_{bookID:02d}.txt"
        with open(fileName,'w') as file:
            curr = self.head
                
            while curr is not None:  
                if curr.bookID == bookID:
                    file.write(curr.line + '\n')
                curr = curr.next
        print(f"Book {bookID:02d} written to {fileName}")


def clientHandler(connection, bookCounter):

    bookID = bookCounter
    print(f"Connection accepted: Book ID {bookID}")

    # when the client does not send any information
    shared_list.writer(bookID)

    while True:
        try:
            buffer = connection.recv(1234)
            if len(buffer) == 0:
                break

            lines = buffer.decode('utf-8').splitlines()
            for line in lines:
                shared_list.addNode(line, bookID)

        except Exception as e:
            print(f'error: {e}')
            break

    shared_list.writer(bookID)
    connection.close()
    print(f"Connection closed: Book ID {bookID}")


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--port', type=int, help='Port to listen on', default=1234)
    args = parser.parse_args()

    global shared_list
    shared_list = SharedList()


    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverSocket.bind(('localhost', args.port))  

    serverSocket.listen(10)

    book_counter = 1

    while True:
        clientSocket, clientAddress = serverSocket.accept()

        print('f"Accepted connection from {clientAddress}')

        threading.Thread(target=clientHandler, args=(clientSocket, book_counter)).start()
        book_counter += 1


if __name__ == "__main__":
    main()