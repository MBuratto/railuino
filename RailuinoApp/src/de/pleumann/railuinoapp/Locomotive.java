package de.pleumann.railuinoapp;

import java.io.File;

public class Locomotive {

    public static final int TYPE_NEW = -1;
    
    public static final int TYPE_MM2 = 0;
    
    public static final int TYPE_DCC = 1;

    public static final int TYPE_MFX = 2;
    
    private String name = "";
    
    private int type = 0;
    
    private int address = 0;
    
    private File photo;
    
    private int direction;
    
    private int speed;

    private int functions;

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

    public File getPhoto() {
        return photo;
    }
    
    public void setPhoto(File photo) {
        this.photo = photo;
    }
    
    public int getDirection() {
        return direction;
    }

    public void setDirection(int direction) {
        this.direction = direction;
    }

    public int getSpeed() {
        return speed;
    }

    public void setSpeed(int speed) {
        this.speed = speed;
    }
    
    public boolean getFunction(int index) {
        int bit = 1 << index;
        return (functions & bit) != 0;
    }

    public void setFunction(int index, boolean value) {
        int bit = 1 << index;
        functions = value ? functions | bit : functions & ~bit;
    }
    
    public int getFullAddress() {
        return type == 0 ? address : type == 1 ? 0xc000 + address : 0x4000 + address;
    }
    
    public String toString() {
        return name.equals("") ? untitled : name;
    }

}
