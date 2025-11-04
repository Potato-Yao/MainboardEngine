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
    implementation("net.java.dev.jna:jna:5.14.0")
    implementation("net.java.dev.jna:jna-platform:5.14.0")
}

tasks.test {
    useJUnitPlatform()
}

// Native build configuration
val nativeDir = file("native")
val nativeBuildScript = file("build.py")
val nativeOutputDir = file("native/MEbuild")

// Detect OS for native library name
val osName = System.getProperty("os.name").lowercase()
val nativeLibraryName = when {
    osName.contains("win") -> "mainboard_native.dll"
    osName.contains("mac") -> "libmainboard_native.dylib"
    osName.contains("nix") || osName.contains("nux") -> "libmainboard_native.so"
    else -> "mainboard_native.dll"
}

// Task to build native code
val buildNative by tasks.registering(Exec::class) {
    description = "Build native C++ library using CMake"
    group = "build"

    workingDir = projectDir

    // Use python or python3 depending on platform
    val pythonCmd = if (osName.contains("win")) "python" else "python3"

    commandLine(pythonCmd, nativeBuildScript.absolutePath, "--release")

    inputs.files(
        fileTree(nativeDir) {
            include("**/*.cpp", "**/*.h", "**/*.c", "CMakeLists.txt")
            exclude("build/**", "cmake-build-*/**", "MEbuild/**", "third_party/**")
        }
    )
    outputs.dir(nativeOutputDir)

    doFirst {
        println("========================================")
        println("Building native library...")
        println("Native directory: ${nativeDir.absolutePath}")
        println("Expected output: ${nativeOutputDir.absolutePath}/${nativeLibraryName}")
        println("========================================")
    }

    doLast {
        val builtLibrary = file("${nativeOutputDir}/${nativeLibraryName}")
        if (!builtLibrary.exists()) {
            throw GradleException("Native library was not built: ${builtLibrary.absolutePath}")
        }
        println("[OK] Native library built successfully: ${builtLibrary.absolutePath}")
    }
}

// Task to build native code with Visual Studio Developer environment
val buildNativeWithVSDev by tasks.registering(Exec::class) {
    description = "Build native C++ library with Visual Studio Developer environment setup"
    group = "build"

    workingDir = projectDir

    // Use python or python3 depending on platform
    val pythonCmd = if (osName.contains("win")) "python" else "python3"

    commandLine(pythonCmd, nativeBuildScript.absolutePath, "--release", "--vsdev")

    inputs.files(
        fileTree(nativeDir) {
            include("**/*.cpp", "**/*.h", "**/*.c", "CMakeLists.txt")
            exclude("build/**", "cmake-build-*/**", "MEbuild/**", "third_party/**")
        }
    )
    outputs.dir(nativeOutputDir)

    doFirst {
        println("========================================")
        println("Building native library with VS Developer environment...")
        println("Native directory: ${nativeDir.absolutePath}")
        println("Expected output: ${nativeOutputDir.absolutePath}/${nativeLibraryName}")
        println("This will setup Visual Studio 2022 Developer environment")
        println("========================================")
    }

    doLast {
        val builtLibrary = file("${nativeOutputDir}/${nativeLibraryName}")
        if (!builtLibrary.exists()) {
            throw GradleException("Native library was not built: ${builtLibrary.absolutePath}")
        }
        println("[OK] Native library built successfully with VS Developer environment: ${builtLibrary.absolutePath}")
    }
}

// Task to clean native build artifacts
val cleanNative by tasks.registering(Delete::class) {
    description = "Clean native build artifacts"
    group = "build"

    delete(file("native/build"))
    delete(file("native/cmake-build-debug"))
    delete(file("native/cmake-build-debug-visual-studio"))
    delete(file("native/cmake-build-release"))
    delete(nativeOutputDir)

    doLast {
        println("[OK] Native build artifacts cleaned")
    }
}

// Function to check if CMake is available in PATH
fun isCMakeAvailable(): Boolean {
    return try {
        val process = if (osName.contains("win")) {
            ProcessBuilder("cmd", "/c", "cmake", "--version")
        } else {
            ProcessBuilder("cmake", "--version")
        }.start()
        process.waitFor()
        process.exitValue() == 0
    } catch (_: Exception) {
        false
    }
}

// Function to check if Visual Studio is installed on Windows
fun isVisualStudioInstalled(): Boolean {
    if (!osName.contains("win")) return false
    val vsDevCmd = file("C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat")
    return vsDevCmd.exists()
}

// Hook native build into Java build lifecycle
// Use buildNativeWithVSDev on Windows if VS is installed and CMake is not in PATH
tasks.named("compileJava") {
    val shouldUseVSDev = osName.contains("win") && isVisualStudioInstalled() && !isCMakeAvailable()

    if (shouldUseVSDev) {
        dependsOn(buildNativeWithVSDev)
        doFirst {
            println("[INFO] CMake not found in PATH, using Visual Studio Developer environment")
        }
    } else {
        dependsOn(buildNative)
    }
}

// Include native clean in main clean task
tasks.named("clean") {
    dependsOn(cleanNative)
}

// Configure test task to add native library path
tasks.withType<Test> {
    // Add MEbuild directory to JNA library path
    systemProperty("jna.library.path", nativeOutputDir.absolutePath)

    doFirst {
        println("JNA library path: ${nativeOutputDir.absolutePath}")
    }
}

// Configure run tasks to add native library path
tasks.withType<JavaExec> {
    // Add MEbuild directory to JNA library path
    systemProperty("jna.library.path", nativeOutputDir.absolutePath)
}

