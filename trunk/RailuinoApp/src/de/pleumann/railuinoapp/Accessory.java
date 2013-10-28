package de.pleumann.railuinoapp;

import android.graphics.Bitmap;

public class Accessory {

    public static final int TYPE_NEW = -1;
    
    public static final int TYPE_MM2 = 0;
    
    public static final int TYPE_DCC = 1;

    private String name = "";
    
    private int type = 0;
    
    private int address = 0;
    
    private int states;

    static String untitled;
    
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public int getAddress() {
        return address;
    }

    public void setAddress(int address) {
        this.address = address;
    }
    
    public boolean getState(int index) {
        int bit = 1 << index;
        return (states & bit) != 0;
    }

    public void setState(int index, boolean value) {
        int bit = 1 << index;
        states = value ? states | bit : states & ~bit;
    }
    
    public int getFullAddress() {
        return type == 0 ? 0x2fff + address : 0x3800 + address;
    }

    public String toString() {
        return name.equals("") ? untitled : name;
    }

}
