package com.potato.Utils;

import com.potato.NativeUtils.NativeCaller;

public interface EventProcesser {
    void process(GameContext gameContext, NativeCaller caller);
}
