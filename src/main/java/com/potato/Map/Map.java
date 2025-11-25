package com.potato.Map;

import com.potato.Config;

import java.util.ArrayList;

public class Map {
    // use bucket to accelerate access speed
    private ArrayList<BlockItem> blockItems;
    private ArrayList<Block> blocks;

    public Map(ArrayList<BlockItem> blockItems, ArrayList<Block> blocks) {
        this.blocks = blocks;
        this.blockItems = new ArrayList<>(Config.BLOCK_ARRAY_SIZE);
        for (int i = 0; i < Config.BLOCK_ARRAY_SIZE; i++) {
            this.blockItems.add(null);
        }

        for (BlockItem item : blockItems) {
            if (item.getId() > Config.BLOCK_ARRAY_SIZE) {
                throw new RuntimeException("BlockItem id exceeds BLOCK_ARRAY_SIZE");
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
}
