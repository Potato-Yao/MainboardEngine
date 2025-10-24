package com.potato;

import com.potato.Utils.GameContext;

public class Config {
    public static EngineType engineType = null;
    public static OSType os = OSType.Windows;
    public static GameContext gameContext = GameContext.getGameContext();

    public static void init() {
        String osName = System.getProperty("os.name");
        if (osName.contains("Windows")) {
            os = OSType.Windows;
        }
    }
}
