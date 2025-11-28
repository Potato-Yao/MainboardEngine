package com.potato.Map;

import com.potato.Config;

import java.util.ArrayList;

public class Map {
    // use bucket to accelerate access speed
    private ArrayList<BlockItem> blockItems;
    private ArrayList<Block> blocks;
    private int blockWidth;
    private int blockHeight;

    public Map(ArrayList<BlockItem> blockItems, ArrayList<Block> blocks) {
        this.blocks = blocks;
        this.blockItems = new ArrayList<>(Config.blockArraySize);
        for (int i = 0; i < Config.blockArraySize; i++) {
            this.blockItems.add(null);
        }

        for (BlockItem item : blockItems) {
            if (item.getId() > Config.blockArraySize) {
                throw new RuntimeException("BlockItem id exceeds BLOCK_ARRAY_SIZE");
            }

            if (this.blockItems.get(item.getId()) != null) {
                throw new RuntimeException("Duplicate BlockItem id: " + item.getId());
            }
            this.blockItems.set(item.getId(), item);
        }
    }

    public ArrayList<BlockItem> getBlockItems() {
        return blockItems;
    }

    /**
     * Get the list of blocks that need to be rendered.
     * @return
     */
    public ArrayList<Block> getRenderedBlocks() {
        return blocks;
    }

    public void setBlockHeight(int blockHeight) {
        this.blockHeight = blockHeight;
    }

    public void setBlockWidth(int blockWidth) {
        this.blockWidth = blockWidth;
    }

    public int getBlockWidth() {
        return blockWidth;
    }

    public int getBlockHeight() {
        return blockHeight;
    }
}
