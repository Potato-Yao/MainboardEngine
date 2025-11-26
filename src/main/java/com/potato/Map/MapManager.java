package com.potato.Map;

import com.moandjiezana.toml.Toml;
import com.potato.Config;
import com.potato.NativeUtils.NativeCaller;

import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class MapManager {
    private static MapManager mapManager;

    private HashMap<String, Map> maps;

    private MapManager() {
        maps = new HashMap<>();
    }

    public static MapManager getMapManager() {
        if (mapManager == null) {
            mapManager = new MapManager();
        }

        return mapManager;
    }

    public void registerMap(String mapId, Map map) {
        if (maps.containsKey(mapId)) {
            throw new RuntimeException("Map " + mapId + " already registered.");
        }
        maps.put(mapId, map);
    }

    public void registerMap(String mapId, String mapFileName) {
        File mapFile = new File(Config.mapLocation + mapFileName + ".toml");
        if (maps.containsKey(mapId)) {
            throw new RuntimeException("Map " + mapId + " already registered.");
        }
        if (!mapFile.exists()) {
            throw new RuntimeException("Map " + mapId + " does not exist at " + mapFile.getAbsolutePath());
        }

        Toml mapFileToml = new Toml().read(mapFile);
        List<Toml> blockItemTomls = mapFileToml.getTables("block-item");
        List<Toml> blockTomls = mapFileToml.getTables("block");

        ArrayList<BlockItem> blockItems = new ArrayList<>();
        for (Toml blockItemToml : blockItemTomls) {
            int id = blockItemToml.getLong("id").intValue();
            String path = blockItemToml.getString("path");
            BlockItem blockItem = new BlockItem(id, path);
            blockItems.add(blockItem);
        }

        ArrayList<Block> blocks = new ArrayList<>();
        for (Toml blockToml : blockTomls) {
            int id = blockToml.getLong("id").intValue();
            int x = blockToml.getLong("x").intValue();
            int y = blockToml.getLong("y").intValue();
            Block block = new Block(id, x, y);
            blocks.add(block);
        }

        Map map = new Map(blockItems, blocks);

        int blockWidth = mapFileToml.getLong("block_width").intValue();
        int blockHeight = mapFileToml.getLong("block_height").intValue();
        map.setBlockWidth(blockWidth);
        map.setBlockHeight(blockHeight);

        maps.put(mapId, map);
    }

    public void loadMap(String mapId, NativeCaller caller) {
        if (!maps.containsKey(mapId)) {
            throw new RuntimeException("Map " + mapId + " not registered.");
        }

        Config.gameContext.setCurrentMap(mapId);

        caller.clearBlock();
        Map map = maps.get(mapId);
        ArrayList<BlockItem> blockItems = map.getBlockItems();
        for (BlockItem blockItem : blockItems) {
            if (blockItem == null) {
                continue;
            }
            caller.loadBlock(blockItem.getId(), blockItem.getPath());
        }
    }

    public void renderMap(NativeCaller caller) {
        String mapId = Config.gameContext.getCurrentMap();
        if (!maps.containsKey(mapId)) {
            throw new RuntimeException("Map " + mapId + " not registered.");
        }

        Map map = maps.get(mapId);
        for (Block block : map.getRenderedBlocks()) {
            caller.renderBlock(block.getId(), block.getX() * map.getBlockWidth(), block.getY() * map.getBlockHeight());
        }

        caller.renderFrame();
    }
}
