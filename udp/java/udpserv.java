/*****************************************************************************
 This is a UDP echo server.  It will listen on a port for incomming UDP packets,
 and then echo them to the sender. 
*****************************************************************************/

import java.net.*;
import java.io.*;

public class udpserv {
	protected DatagramSocket dgSocket;	// declare socket
	private static  DelayClass dc = new DelayClass();

    // Constructor
	public udpserv (int port) throws IOException {
	    dgSocket = new DatagramSocket(port);    // allocate socket
	}
	
	// execute() loop to wait for incomming datagram packets
	public void execute(InetAddress address)throws IOException {
	    while (true) 
	    {
	        DatagramPacket dgPacket = receive();	// this function will block until packet received

			dc.Delay(1000);		// delay between sending packets
			byte[] buffer;
			String msg = "Message Received";
			buffer = msg.getBytes();
			dgSocket.send(new DatagramPacket(buffer, buffer.length, 
				address, 0x1234));
			System.out.println("Sent packet");

	    }
	}

    // method to receive a single datagram packet
    protected DatagramPacket receive() throws IOException {
        byte buffer[] = new byte[65536];
        DatagramPacket dgPacket = new DatagramPacket(buffer, buffer.length);
        dgSocket.receive(dgPacket);

		System.out.print("Rx Addr:" + dgPacket.getAddress());
		System.out.print(" Port: " + dgPacket.getPort());
        System.out.println(" bytes: " + dgPacket.getLength());
		String msg = new String(buffer, 0, dgPacket.getLength());
		System.out.println("Message: " + msg);
		System.out.println("");

        return dgPacket;
    }

    // method to send echo to client
    protected void sendEcho(InetAddress address, int port, byte data[], int length) throws IOException {
        DatagramPacket packet = new DatagramPacket(data, length, address, port);
        dgSocket.send(packet);
        System.out.println("Sent response to " + address);
    }

    public static void main(String args[]) throws IOException {
        int port;
        if (args.length != 1) 
            throw new RuntimeException("Syntax:  EchoServerUDP <port>");
        port = Integer.parseInt(args[0]);
        udpserv serverUDP = new udpserv(port);
        System.out.println("Starting UDP echo server on port " + port);
		InetAddress address = InetAddress.getByName("204.210.19.100");
//		System.out.println("Host Name: " + address.getHostName());
//		System.out.println("Host Addr: " + address.getHostAddress());
//		dc.Delay(3000);
        serverUDP.execute(address);
    }
}	


// An instance of this class is used to create a delay function
class DelayThread extends Thread {
	int DelayTime;
	volatile boolean complete;

	public DelayThread(int time)
	{
		DelayTime = time;
		complete = false;
	}
	
	public void run()
	{
		try {
 			sleep(DelayTime);
		}
		catch (InterruptedException e) {
			System.err.println("Exception: " + e.toString());
		}
		complete = true;
	}

	public boolean Complete()
	{
		return complete;
	}
}


// This class is used in a program to execute a delay.
class DelayClass  {
	public DelayClass()	{ }

	public void Delay (int time)
	{
		DelayThread dt = new DelayThread(time);
		dt.start();
		while (! dt.Complete());
	}
}