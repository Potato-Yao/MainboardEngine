package com.potato;

public class Config {
    public static EngineType engineType = null;
    public static OSType os = null;

    public static void init() {
        String osName = System.getProperty("os.name");
        if (osName.contains("Windows")) {
            os = OSType.Windows;
        }
    }
}
