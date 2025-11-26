package com.potato.Map;

public class BlockItem {
    private int id;
    private String path;

    public BlockItem(int id, String path) {
        this.id = id;
        this.path = path;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getPath() {
        return path;
    }
}
