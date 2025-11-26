package com.potato;

import com.moandjiezana.toml.Toml;
import com.potato.Utils.GameContext;
import com.potato.Variable.EngineType;
import com.potato.Variable.OSType;

import java.io.File;

public class Config {
    public static int blockArraySize;
    public static EngineType engineType;
    public static OSType os;
    public static GameContext gameContext = GameContext.getGameContext();
    public static String default_map_id;

    public static void init() {
        String osName = System.getProperty("os.name");
        blockArraySize = 1024;
        engineType = EngineType.Directx11;
        if (osName.contains("Windows")) {
            os = OSType.Windows;
        } else if (osName.contains("Linux")) {
            os = OSType.Linux;
        }
        default_map_id = "main_map";
    }

    public static void init(File configFilePath) {
        init();
        if (!configFilePath.exists()) {
            throw new RuntimeException("Config file does not exist, at " + configFilePath);
        }
        Toml configToml = new Toml().read(configFilePath);
        String configFileVersion = configToml.getString("version");
        int blockArraySizeConfig = configToml.getLong("block_count").intValue();
        String defaultMapIdConfig = configToml.getString("default_map");
        String engineTypeConfig = configToml.getString("engine");

        blockArraySize = blockArraySizeConfig;
        default_map_id = defaultMapIdConfig;
        if (engineTypeConfig.equals("Directx11")) {
            engineType = EngineType.Directx11;
        } else if (engineTypeConfig.equals("Directx12")) {
            engineType = EngineType.Directx12;
        } else if (engineTypeConfig.equals("OpenGL")) {
            engineType = EngineType.OpenGL;
        }
    }
}
