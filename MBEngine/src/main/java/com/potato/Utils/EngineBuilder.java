package com.potato.Utils;

public class EngineBuilder {
    private Engine engine;

    public EngineBuilder() {
        this.engine = new Engine();
    }

    public EngineBuilder configFile(String configFilePath) {
        engine.setConfigFilePath(configFilePath);

        return this;
    }

    public Engine build() {
        return engine;
    }
}
