package network

import (
	"fmt"

	"github.com/zeromq/goczmq"
)

type ZmqSubscriberSocket struct {
	socket *goczmq.Sock
}

func NewZmqSubscriberSocket(address string, topics string) *ZmqSubscriberSocket {
	socket, err := goczmq.NewSub(address, topics)

	if err != nil {
		panic(err)
	}

	return &ZmqSubscriberSocket{
		socket: socket,
	}
}

func (sock *ZmqSubscriberSocket) Receive() ZmqMultipartDatagram {
	bytes, err := sock.socket.RecvMessage()

	if err != nil {
		fmt.Println("failed to receive message", err)
		return *NewZmqMultipartDatagram(nil, nil)
	}

	return *NewZmqMultipartDatagram(bytes[0], bytes[1])
}

func (sock *ZmqSubscriberSocket) Close() {
	sock.socket.Destroy()
}
