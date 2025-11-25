package com.potato;

import com.potato.Utils.GameContext;
import com.potato.Variable.EngineType;
import com.potato.Variable.OSType;

public class Config {
    public static final int BLOCK_ARRAY_SIZE = 1024;  // TODO make it in config file
    public static EngineType engineType = null;
    public static OSType os = OSType.Windows;
    public static GameContext gameContext = GameContext.getGameContext();
    public static final String DEFAULT_MAP_ID = "main_map";

    public static void init() {
        String osName = System.getProperty("os.name");
        if (osName.contains("Windows")) {
            os = OSType.Windows;
        }
    }
}
