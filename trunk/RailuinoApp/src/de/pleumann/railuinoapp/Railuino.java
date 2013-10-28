package de.pleumann.railuinoapp;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.UUID;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

public class Railuino {

    private BluetoothDevice device;

    private BluetoothSocket socket;

    private InputStream input;

    private OutputStream output;

    private int messageID;

    public Railuino(BluetoothDevice device) throws IOException {
        this.device = device;

        socket = getSocket(device);
        socket.connect();

        input = socket.getInputStream();
        output = socket.getOutputStream();

        Log.d("XXX", "Connected to " + device.getName());
    }

    /**
     * Sends the given message, assigning a new message ID on the fly.
     */
    public void send(String s) throws IOException {
        output.write(s.getBytes());
        output.write(13);
        output.write(10);

        Log.d("XXX", "Sent: " + s);
    }

    /**
     * Receives the next message and returns it.
     */
    public String receive() throws IOException {
        StringBuilder builder = new StringBuilder(256);
        int i = input.read();
        while (i != 13) {
            if (i >= ' ') {
                builder.append((char)i);
            }
            i = input.read();
        }

        Log.d("XXX", "Received:  " + builder.toString());
        
        return builder.toString();
    }

    /**
     * Sends the given message, assigning a new message ID on the fly, and
     * receives and returns the response message. This also handles the
     * necessary ACK messages in both directions. Synchronous. No error handling
     * is being done. The messages are not checked for correctness.
     */
    public boolean sendAndReceive(final String s) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    send(s);
                    String response = receive();

                    // return response.startsWith("OK");
                } catch (IOException e) {
                    e.printStackTrace();
                    // return false;
                }
            }
        }).start();
        
        return true;
    }

    /**
     * Closes the connection to the DiOBD880, ignoring all problems that might
     * occur. Should be called when the object is not needed anymore.
     */
    public void close() {
        try {
            if (input != null) {
                input.close();
                input = null;
            }
        } catch (Exception ignored) {
        }

        try {
            if (output != null) {
                output.close();
                output = null;
            }
        } catch (Exception ignored) {
        }

        try {
            if (socket != null) {
                socket.close();
                socket = null;
            }
        } catch (Exception ignored) {
        }

        if (device != null) {
            device = null;
        }

        Log.d("XXX", "Connection closed");
    }

    /**
     * Creates an RFCOMM Bluetooth socket. It we are paired with the box, we can
     * use a secure socket. Otherwise we have to fallback to an insecure one. We
     * use reflection to access the method, because it is at least
     * hidden/undocumented in older releases. It might even be missing
     * completely.
     */
    private BluetoothSocket getSocket(BluetoothDevice device) {
        try {
            if (device.getBondState() == BluetoothDevice.BOND_BONDED) {
                return device.createInsecureRfcommSocketToServiceRecord(UUID
                        .fromString("00001101-0000-1000-8000-00805F9B34FB"));
            } else {
                // This also works on Android 2.2, where insecure RFCOMM is
                // undocumented. Normally, starting from Android 2.3, we would
                // simply do this:
                //
                // device.createInsecureRfcommSocketToServiceRecord(UUID
                // .fromString("00001101-0000-1000-8000-00805F9B34FB"));

                Method method = BluetoothDevice.class.getMethod(
                        "createInsecureRfcommSocket", int.class);
                return (BluetoothSocket) method.invoke(device, 1);
            }
        } catch (Exception e) {
            Log.e("XXX", "Error getting socket", e);
            return null;
        }
    }

    /**
     * Returns the readable name of the Bluetooth device we are connected to.
     */
    public String getName() {
        return device.getName();
    }

    public boolean setPower(boolean value) {
        return sendAndReceive("setPower(" + (value ? 1 : 0) + ")");
    }

    public boolean setLocoDirection(int address, int dir) {
        return sendAndReceive("setLocoDirection(" + address + ", " + ( 1 + dir) + ")");
    }

    public boolean setLocoSpeed(int address, int speed) {
        return sendAndReceive("setLocoSpeed(" + address + ", " + speed + ")");
    }

    public boolean setLocoFunction(int address, int index, boolean value) {
        return sendAndReceive("setLocoFunction(" + address + ", " + index + ", " + (value ? 1 : 0) + ")");
    }

    public boolean setTurnout(int address, boolean value) {
        return sendAndReceive("setTurnout(" + address + ", " + (value ? 1 : 0) + ")");
    }

}
