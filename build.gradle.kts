plugins {
    id("java")
}

group = "com.potato"
version = "1.0-SNAPSHOT"

repositories {
//    maven { setUrl("https://maven.aliyun.com/repository/central") }
//    maven { setUrl("https://maven.aliyun.com/repository/jcenter") }
//    maven { setUrl("https://maven.aliyun.com/repository/google") }
//    maven { setUrl("https://maven.aliyun.com/repository/gradle-plugin") }
//    maven { setUrl("https://maven.aliyun.com/repository/public") }
//    maven { setUrl("https://maven.aliyun.com/nexus/content/groups/public/") }
//    maven { setUrl("https://maven.aliyun.com/nexus/content/repositories/jcenter") }
//    maven { setUrl("https://maven.pkg.jetbrains.space/public/p/compose/dev") }

    maven { setUrl("https://maven.pkg.jetbrains.space/public/p/compose/dev") }
    gradlePluginPortal()
    google()
    mavenCentral()
}

dependencies {
    testImplementation(platform("org.junit:junit-bom:5.7.1"))
    testImplementation("org.junit.jupiter:junit-jupiter")
    testRuntimeOnly("org.junit.platform:junit-platform-launcher")
    // https://mvnrepository.com/artifact/net.java.dev.jna/jna
    implementation("net.java.dev.jna:jna:4.5.0")
    // https://mvnrepository.com/artifact/net.java.dev.jna/jna-platform
    implementation("net.java.dev.jna:jna-platform:5.18.0")
    // JNAerator for code generation (compileOnly since it's build-time only)
    compileOnly("com.nativelibs4java:jnaerator:0.12")
    // JNAerator as annotation processor (optional, for programmatic use)
    annotationProcessor("com.nativelibs4java:jnaerator:0.12")
}

tasks.test {
    useJUnitPlatform()
}