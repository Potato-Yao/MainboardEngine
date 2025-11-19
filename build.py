#!/usr/bin/env python3
"""
Mainboard Engine Build Script
Handles native library building and dependency management across platforms.
"""

import os
import sys
import platform
import subprocess
import shutil
from pathlib import Path


def find_cmake_exe() -> str:
    """Locate cmake executable. Returns absolute path or None if not found.
    Order: env CMAKE_EXE -> PATH -> known install locations.
    """
    # Allow override via environment variable
    env_override = os.environ.get("CMAKE_EXE")
    if env_override and os.path.exists(env_override):
        return env_override

    cmake = shutil.which("cmake")
    if cmake:
        return cmake

    # Common Windows install paths (Visual Studio bundled CMake + standalone)
    candidates = [
        # Standalone CMake
        r"C:\\Program Files\\CMake\\bin\\cmake.exe",
        r"C:\\Program Files (x86)\\CMake\\bin\\cmake.exe",
        # Visual Studio 2022 variants
        r"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
        r"C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
        r"C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
        r"C:\\Program Files\\Microsoft Visual Studio\\2022\\BuildTools\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
        # Visual Studio 2019 variants
        r"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
        r"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
        r"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
        r"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe",
    ]
    for c in candidates:
        if os.path.exists(c):
            return c
    return None


class BuildConfig:
    """Build configuration for different platforms and build types."""

    def __init__(self):
        self.project_root = Path(__file__).parent.resolve()
        self.native_dir = self.project_root / "native"
        self.third_party_dir = self.native_dir / "third_party"
        self.bgfx_cmake_dir = self.third_party_dir / "bgfx.cmake"
        self.build_dir = self.native_dir / "build"
        self.output_dir = self.native_dir / "MEbuild"

        # Detect OS
        self.os_name = platform.system()
        self.is_windows = self.os_name == "Windows"
        self.is_linux = self.os_name == "Linux"
        self.is_macos = self.os_name == "Darwin"

        # Build type (default to Release)
        self.build_type = "Release"

    def __str__(self):
        return f"BuildConfig(OS={self.os_name}, BuildType={self.build_type})"


class Dependency:
    """Represents a single dependency to download."""

    def __init__(self, name: str, dep_type: str, url: str, target_dir: str,
                 verify_paths: list = None, has_submodules: bool = False):
        """
        Initialize a dependency.

        Args:
            name: Human-readable name of the dependency
            dep_type: Type of dependency ('git' or 'file')
            url: URL to download from
            target_dir: Target directory relative to third_party (for git) or filename (for file)
            verify_paths: List of paths to verify installation (relative to target_dir for git, or just the filename for file)
            has_submodules: Whether the git repository has submodules (only for git type)
        """
        self.name = name
        self.dep_type = dep_type
        self.url = url
        self.target_dir = target_dir
        self.verify_paths = verify_paths or []
        self.has_submodules = has_submodules


# Define all dependencies here
DEPENDENCIES = [
    Dependency(
        name="bgfx.cmake",
        dep_type="git",
        url="https://github.com/bkaradzic/bgfx.cmake.git",
        target_dir="bgfx.cmake",
        verify_paths=["CMakeLists.txt", "bgfx"],
        has_submodules=True
    ),
    # Add more dependencies here as needed:
    Dependency(
        name="stb_image",
        dep_type="file",
        url="https://raw.githubusercontent.com/nothings/stb/master/stb_image.h",
        target_dir="stb_image.h",
        verify_paths=["stb_image.h"]
    ),
]


class DependencyManager:
    """Handles downloading and managing all project dependencies."""

    def __init__(self, config: BuildConfig):
        self.config = config

    def check_git_available(self) -> bool:
        """Check if git is available in the system."""
        try:
            subprocess.run(["git", "--version"],
                         capture_output=True,
                         check=True)
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            return False

    def check_curl_available(self) -> bool:
        """Check if curl is available in the system."""
        try:
            subprocess.run(["curl", "--version"],
                         capture_output=True,
                         check=True)
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            return False

    def is_dependency_installed(self, dep: Dependency) -> bool:
        """Check if a dependency is already downloaded and valid."""
        if dep.dep_type == "git":
            target_path = self.config.third_party_dir / dep.target_dir
            for verify_path in dep.verify_paths:
                check_path = target_path / verify_path
                if not check_path.exists():
                    return False
            return True
        elif dep.dep_type == "file":
            file_path = self.config.third_party_dir / dep.target_dir
            return file_path.exists()
        return False

    def download_git_repo(self, dep: Dependency):
        """Download a git repository."""
        print(f"Cloning {dep.name} from {dep.url}...")

        if not self.check_git_available():
            print("ERROR: Git is not available in the system.")
            print("Please install Git and try again.")
            sys.exit(1)

        target_path = self.config.third_party_dir / dep.target_dir

        # Remove existing directory if it exists but is incomplete
        if target_path.exists() and not self.is_dependency_installed(dep):
            print(f"Removing incomplete {dep.name} directory...")
            shutil.rmtree(target_path)

        # Clone the repository
        if not target_path.exists():
            subprocess.run([
                "git", "clone",
                dep.url,
                str(target_path)
            ], check=True)

        # Handle submodules if needed
        if dep.has_submodules:
            print(f"Initializing submodules for {dep.name}...")
            subprocess.run([
                "git", "submodule", "init"
            ], cwd=target_path, check=True)

            print(f"Updating submodules (this may take a while)...")
            subprocess.run([
                "git", "submodule", "update"
            ], cwd=target_path, check=True)

        print(f"[OK] {dep.name} downloaded successfully!")

    def download_file(self, dep: Dependency):
        """Download a file using curl."""
        print(f"Downloading {dep.name} from {dep.url}...")

        if not self.check_curl_available():
            print("ERROR: curl is not available in the system.")
            print("Please install curl and try again.")
            sys.exit(1)

        target_path = self.config.third_party_dir / dep.target_dir

        # Download the file
        subprocess.run([
            "curl",
            "-L",  # Follow redirects
            "-o", str(target_path),
            dep.url
        ], check=True)

        print(f"[OK] {dep.name} downloaded successfully!")

    def download_dependency(self, dep: Dependency):
        """Download a single dependency based on its type."""
        print("=" * 60)
        print(f"Downloading {dep.name}...")
        print("=" * 60)

        # Create third_party directory if it doesn't exist
        self.config.third_party_dir.mkdir(parents=True, exist_ok=True)

        if dep.dep_type == "git":
            self.download_git_repo(dep)
        elif dep.dep_type == "file":
            self.download_file(dep)
        else:
            print(f"ERROR: Unknown dependency type '{dep.dep_type}' for {dep.name}")
            sys.exit(1)

        print()

    def ensure_all_dependencies(self):
        """Ensure all dependencies are available, download if needed."""
        for dep in DEPENDENCIES:
            if self.is_dependency_installed(dep):
                print(f"[OK] {dep.name} is already installed")
            else:
                self.download_dependency(dep)
        print()


class ShaderBuilder:
    """Handles building shaders using bgfx shaderc."""

    def __init__(self, config: BuildConfig):
        self.config = config
        self.shader_dir = self.config.project_root / "shader"
        self.shader_output_dir = self.shader_dir  # Output to same folder as input
        self.shaderc_path = None

    def find_shaderc(self) -> bool:
        """Find the shaderc executable in the build directory."""
        print("Looking for shaderc compiler...")

        # Possible locations for shaderc
        possible_paths = [
            self.config.build_dir / "bgfx" / "shaderc" / "Release" / "shaderc.exe",
            self.config.build_dir / "bgfx" / "shaderc" / "Debug" / "shaderc.exe",
            self.config.build_dir / "bgfx" / "shaderc" / "shaderc.exe",
            self.config.build_dir / "bin" / "shaderc.exe",
            self.config.build_dir / "bin" / "shaderc",
            # Additional common VS/CMake output locations
            self.config.build_dir / "Release" / "shaderc.exe",
            self.config.build_dir / "Debug" / "shaderc.exe",
            self.config.build_dir / "shaderc" / "Release" / "shaderc.exe",
            self.config.build_dir / "shaderc" / "Debug" / "shaderc.exe",
            self.config.build_dir / "third_party" / "bgfx.cmake" / "cmake" / "bgfx" / "Release" / "shaderc.exe",
        ]

        for path in possible_paths:
              if path.exists() and path.is_file() and os.access(path, os.X_OK):
                    self.shaderc_path = path
                    print(f"  [OK] Found shaderc at: {path}")
                    return True

        print("  [WARNING] shaderc not found. Shaders will not be compiled.")
        print("  Tip: Build the project first to generate shaderc.")
        return False

    def _configure_if_needed(self):
        """Configure CMake build dir if not configured yet."""
        cache = self.config.build_dir / "CMakeCache.txt"
        if not cache.exists():
            from build import NativeBuilder  # local import to avoid cycle at import time
            nb = NativeBuilder(self.config)
            nb.configure_cmake()

    def build_shaderc_tool(self) -> bool:
        """Attempt to build the 'shaderc' target via CMake and locate it afterwards."""
        print("Attempting to build 'shaderc' tool...")
        try:
            self._configure_if_needed()
            cmake_exe = find_cmake_exe()
            if not cmake_exe:
                print("  [ERROR] CMake not found. Install CMake and ensure it's in PATH.")
                return False
            build_cmd = [
                cmake_exe, "--build", str(self.config.build_dir),
                "--config", self.config.build_type,
                "--target", "shaderc"
            ]
            print(f"Running: {' '.join(build_cmd)}")
            result = subprocess.run(build_cmd)
            if result.returncode != 0:
                print("  [ERROR] Failed to build shaderc tool")
                return False
            # Try to find again after build
            return self.find_shaderc()
        except Exception as e:
            print(f"  [ERROR] Exception while building shaderc: {e}")
            return False

    def ensure_shaderc(self) -> bool:
        """Ensure shaderc exists; attempt to build it if missing."""
        if self.find_shaderc():
            return True
        # If not found, try to build it
        return self.build_shaderc_tool()

    def get_shader_platform(self) -> tuple:
        """Get platform-specific shader compilation settings."""
        if self.config.is_windows:
            return "windows", "dx11"
        elif self.config.is_linux:
            return "linux", "glsl"
        elif self.config.is_macos:
            return "osx", "metal"
        return "windows", "dx11"

    def get_shader_profile(self, shader_type: str, renderer: str) -> str:
        """Get the shader profile based on type and renderer."""
        profiles = {
            "dx11": {
                #"vertex": "vs_5_0",
                #"fragment": "ps_5_0",
                #"compute": "cs_5_0"
                "vertex": "s_5_0",
                "fragment": "s_5_0",
                "compute": "s_5_0"
            },
            "glsl": {
                "vertex": "120",
                "fragment": "120",
                "compute": "430"
            },
            "metal": {
                "vertex": "metal",
                "fragment": "metal",
                "compute": "metal"
            }
        }
        return profiles.get(renderer, {}).get(shader_type, "s_5_0")

    def detect_shader_type(self, shader_file: Path) -> str:
        """Detect shader type from filename prefix."""
        name = shader_file.stem.lower()
        if name.startswith("vs_") or name.startswith("v_"):
            return "vertex"
        elif name.startswith("fs_") or name.startswith("f_"):
            return "fragment"
        elif name.startswith("cs_") or name.startswith("c_"):
            return "compute"
        else:
            # Try to detect from content
            try:
                content = shader_file.read_text()
                if "gl_Position" in content:
                    return "vertex"
                elif "gl_FragColor" in content or "gl_FragData" in content:
                    return "fragment"
            except:
                pass
        return "vertex"  # Default

    def find_shader_files(self) -> list:
        """Find all shader source files (.sc) in the shader directory (excluding varying.def.sc)."""
        shader_files = []
        if self.shader_dir.exists():
            shader_files = [p for p in self.shader_dir.rglob("*.sc") if p.name != "varying.def.sc"]
        return shader_files

    def find_varying_def(self, shader_file: Path) -> Path:
        """Find the varying.def.sc file for the shader."""
        # Check in the same directory as the shader
        varying_file = shader_file.parent / "varying.def.sc"
        if varying_file.exists():
            return varying_file

        # Check in parent directory
        varying_file = shader_file.parent.parent / "varying.def.sc"
        if varying_file.exists():
            return varying_file

        # Check in shader root directory (now at project root)
        varying_file = self.shader_dir / "varying.def.sc"
        if varying_file.exists():
            return varying_file

        # Check in tests directory (fallback)
        varying_file = self.config.native_dir / "tests" / "varying.def.sc"
        if varying_file.exists():
            return varying_file

        return None

    def compile_shader(self, shader_file: Path) -> bool:
        """Compile a single shader file."""
        if not self.shaderc_path:
            return False

        shader_type = self.detect_shader_type(shader_file)
        platform, renderer = self.get_shader_platform()
        profile = self.get_shader_profile(shader_type, renderer)

        # Create output directory structure
        relative_path = shader_file.relative_to(self.shader_dir).parent
        output_dir = self.shader_output_dir / relative_path
        output_dir.mkdir(parents=True, exist_ok=True)

        # Output file path
        output_file = output_dir / f"{shader_file.stem}.bin"

        # Build shaderc command
        cmd = [
            str(self.shaderc_path),
            "-f", str(shader_file),
            "-o", str(output_file),
            "--platform", platform,
            "--type", shader_type[0],  # 'v' for vertex, 'f' for fragment, 'c' for compute
            "-p", profile,
            "-i", str(self.config.bgfx_cmake_dir / "bgfx" / "src"),
        ]

        # Add varying.def.sc if found
        varying_def = self.find_varying_def(shader_file)
        if varying_def:
            cmd.extend(["--varyingdef", str(varying_def)])

        print(f"  Compiling {shader_file.name} ({shader_type})...")
        print(f"    Command: {' '.join(cmd)}")

        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            if result.returncode == 0:
                print(f"    [OK] {output_file.name}")
                return True
            else:
                print(f"    [ERROR] Compilation failed (return code: {result.returncode}):")
                if result.stdout:
                    print(f"    STDOUT: {result.stdout}")
                if result.stderr:
                    print(f"    STDERR: {result.stderr}")
                return False
        except Exception as e:
            print(f"    [ERROR] {e}")
            return False

    def build_all_shaders(self) -> bool:
        """Build all shaders in the shader directory."""
        print("=" * 60)
        print("Building shaders...")
        print("=" * 60)

        # Ensure shaderc is present; try to build it if missing
        if not self.ensure_shaderc():
            print("ERROR: shaderc tool is not available. Aborting shader build.")
            return False

        shader_files = self.find_shader_files()

        if not shader_files:
            print("  No shader files found in shader directory")
            print()
            return True

        print(f"  Found {len(shader_files)} shader file(s)")
        print()

        success_count = 0
        fail_count = 0

        for shader_file in shader_files:
            if self.compile_shader(shader_file):
                success_count += 1
            else:
                fail_count += 1

        print()
        print(f"Shader compilation complete: {success_count} succeeded, {fail_count} failed")
        print()

        return fail_count == 0


class NativeBuilder:
    """Handles building the native library using CMake."""

    def __init__(self, config: BuildConfig):
        self.config = config
        self.cmake_exe = find_cmake_exe()

    def check_cmake_available(self) -> bool:
        """Check if CMake is available in the system."""
        try:
            result = subprocess.run([self.cmake_exe or "cmake", "--version"],
                                  capture_output=True,
                                  check=True,
                                  text=True)
            version_line = result.stdout.split('\n')[0]
            print(f"[OK] {version_line}")
            return True
        except (subprocess.CalledProcessError, FileNotFoundError, TypeError):
            return False

    def get_cmake_generator(self) -> list:
        """Get the appropriate CMake generator for the platform."""
        if self.config.is_windows:
            # Try to detect Visual Studio
            # Default to Visual Studio 17 2022, but could be made more flexible
            return ["-G", "Visual Studio 17 2022"]
        elif self.config.is_linux or self.config.is_macos:
            # Use Unix Makefiles or Ninja if available
            try:
                subprocess.run(["ninja", "--version"],
                             capture_output=True,
                             check=True)
                return ["-G", "Ninja"]
            except (subprocess.CalledProcessError, FileNotFoundError):
                return ["-G", "Unix Makefiles"]
        return []

    def configure_cmake(self):
        """Configure the project using CMake."""
        print("=" * 60)
        print("Configuring CMake...")
        print("=" * 60)

        if not self.cmake_exe:
            print("ERROR: CMake not found. Please install CMake and ensure it's in PATH.")
            sys.exit(1)

        if not self.check_cmake_available():
            print("ERROR: CMake is not available in the system.")
            print("Please install CMake (version 3.20 or higher) and try again.")
            sys.exit(1)

        # Create build directory
        self.config.build_dir.mkdir(parents=True, exist_ok=True)
        self.config.output_dir.mkdir(parents=True, exist_ok=True)

        # Prepare CMake command
        cmake_cmd = [self.cmake_exe, str(self.config.native_dir)]

        # Add generator
        generator = self.get_cmake_generator()
        cmake_cmd.extend(generator)

        # Add output directory
        cmake_cmd.extend([
            f"-DOUTPUT_DIR={self.config.output_dir}",
            f"-DCMAKE_BUILD_TYPE={self.config.build_type}"
        ])

        print(f"Running: {' '.join(cmake_cmd)}")
        print()

        # Run CMake configuration
        result = subprocess.run(cmake_cmd, cwd=self.config.build_dir)

        if result.returncode != 0:
            print("ERROR: CMake configuration failed!")
            sys.exit(1)

        print()
        print("[OK] CMake configuration successful!")
        print()

    def build_native(self):
        """Build the native library."""
        print("=" * 60)
        print(f"Building native library ({self.config.build_type})...")
        print("=" * 60)

        if not self.cmake_exe:
            print("ERROR: CMake not found. Please install CMake and ensure it's in PATH.")
            sys.exit(1)

        # Prepare build command
        build_cmd = [
            self.cmake_exe,
            "--build",
            str(self.config.build_dir),
            "--config",
            self.config.build_type,
            "--target",
            "mainboard_native"  # Only build mainboard_native, not test_native
        ]

        # Add parallel build flag
        if self.config.is_linux or self.config.is_macos:
            # Get number of CPU cores
            try:
                import multiprocessing
                cpu_count = multiprocessing.cpu_count()
                build_cmd.extend(["--parallel", str(cpu_count)])
            except:
                pass

        print(f"Running: {' '.join(build_cmd)}")
        print()

        # Run build
        result = subprocess.run(build_cmd)

        if result.returncode != 0:
            print("ERROR: Build failed!")
            sys.exit(1)

        print()
        print("[OK] Build successful!")
        print()

    def verify_output(self):
        """Verify that the build output exists."""
        print("Verifying build output...")

        expected_files = []
        if self.config.is_windows:
            expected_files = ["mainboard_native.dll"]
        elif self.config.is_linux:
            expected_files = ["libmainboard_native.so"]
        elif self.config.is_macos:
            expected_files = ["libmainboard_native.dylib"]

        found_files = []
        for file_name in expected_files:
            file_path = self.config.output_dir / file_name
            if file_path.exists():
                found_files.append(file_name)
                print(f"  [OK] {file_name}")

        if not found_files:
            print("  [WARNING] Expected output files not found in MEbuild directory")
            print(f"  Looking for: {', '.join(expected_files)}")
            print(f"  Directory contents:")
            if self.config.output_dir.exists():
                for item in self.config.output_dir.iterdir():
                    print(f"    - {item.name}")

        print()


def print_banner():
    """Print build script banner."""
    print()
    print("=" * 60)
    print("  Mainboard Engine Build Script")
    print("=" * 60)
    print()


def print_usage():
    """Print usage information."""
    print("Usage: python build.py [options]")
    print()
    print("Options:")
    print("  --debug          Build in Debug mode (default: Release)")
    print("  --release        Build in Release mode")
    print("  --clean          Clean build directory before building")
    print("  --download-only  Only download dependencies, don't build")
    print("  --shaders-only   Only build shaders, don't build native library")
    print("  --vsdev          Setup Visual Studio Developer environment before building")
    print("  --help           Show this help message")
    print()


def parse_arguments():
    """Parse command line arguments."""
    args = sys.argv[1:]
    options = {
        'build_type': 'Release',
        'clean': False,
        'vsdev': False,
        'download_only': False,
        'shaders_only': False,
        'help': False
    }

    for arg in args:
        if arg == '--debug':
            options['build_type'] = 'Debug'
        elif arg == '--release':
            options['build_type'] = 'Release'
        elif arg == '--clean':
            options['clean'] = True
        elif arg == '--download-only':
            options['download_only'] = True
        elif arg == '--shaders-only':
            options['shaders_only'] = True
        elif arg == '--vsdev':
            options['vsdev'] = True
        elif arg == '--help' or arg == '-h':
            options['help'] = True
        else:
            print(f"Unknown option: {arg}")
            print_usage()
            sys.exit(1)

    return options


def setup_vsdev_environment():
    """Setup Visual Studio Developer Command Prompt environment."""
    print("=" * 60)
    print("Setting up Visual Studio Developer environment...")
    print("=" * 60)

    vsdev_bat = r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

    if not os.path.exists(vsdev_bat):
        print(f"ERROR: Visual Studio Developer Command file not found at:")
        print(f"  {vsdev_bat}")
        print("Please make sure Visual Studio 2022 Community is installed.")
        sys.exit(1)

    # Create a batch script that runs VsDevCmd and then outputs the environment variables
    temp_bat = Path("temp_vsdev_setup.bat")
    temp_env = Path("temp_vsdev_env.txt")

    try:
        # Write a batch script that sets up VS environment and outputs all env vars
        with open(temp_bat, "w") as f:
            f.write(f'@echo off\n')
            f.write(f'call "{vsdev_bat}" -arch=x64\n')
            f.write(f'set > "{temp_env.absolute()}"\n')

        # Run the batch script
        result = subprocess.run([str(temp_bat)], shell=True, capture_output=True, text=True)

        if result.returncode != 0:
            print("ERROR: Failed to setup Visual Studio Developer environment")
            print(result.stderr)
            sys.exit(1)

        # Read the environment variables and set them in current process
        if temp_env.exists():
            with open(temp_env, "r", encoding="utf-8", errors="ignore") as f:
                for line in f:
                    line = line.strip()
                    if "=" in line:
                        key, value = line.split("=", 1)
                        os.environ[key] = value

            print("[OK] Visual Studio Developer environment configured")
            print()
        else:
            print("ERROR: Failed to capture environment variables")
            sys.exit(1)

    finally:
        # Clean up temporary files
        if temp_bat.exists():
            temp_bat.unlink()
        if temp_env.exists():
            temp_env.unlink()


def clean_build_directory(config: BuildConfig):
    """Clean the build directory."""
    print("=" * 60)
    print("Cleaning build directory...")
    print("=" * 60)

    if config.build_dir.exists():
        print(f"Removing {config.build_dir}...")
        shutil.rmtree(config.build_dir)
        print("[OK] Build directory cleaned!")
    else:
        print("Build directory doesn't exist, nothing to clean.")

    print()


def main():
    """Main entry point."""
    print_banner()

    # Parse arguments
    options = parse_arguments()

    if options['help']:
        print_usage()
        return

    # Create configuration
    config = BuildConfig()
    config.build_type = options['build_type']

    print(f"Configuration: {config}")
    print()

    # Clean if requested
    if options['clean']:
        clean_build_directory(config)

    # Setup Visual Studio Developer environment if requested
    if options['vsdev'] and config.is_windows:
        setup_vsdev_environment()

    # Step 1: Download dependencies
    dep_manager = DependencyManager(config)
    dep_manager.ensure_all_dependencies()

    # Step 2: Handle different build modes
    if options['shaders_only']:
        # Only build shaders
        shader_builder = ShaderBuilder(config)
        shader_builder.build_all_shaders()

        print("=" * 60)
        print("  Shader build completed!")
        print("=" * 60)
        print()
        print(f"Shader output directory: {config.project_root / 'shader'}")
        print()
    elif options['download_only']:
        # Only download dependencies
        print("=" * 60)
        print("  Dependencies downloaded successfully!")
        print("=" * 60)
        print()
    else:
        # Full build: native library + shaders
        builder = NativeBuilder(config)
        builder.configure_cmake()
        builder.build_native()
        builder.verify_output()

        # Step 3: Build shaders after native library is built
        shader_builder = ShaderBuilder(config)
        shader_success = shader_builder.build_all_shaders()

        print("=" * 60)
        print("  Build completed successfully!")
        print("=" * 60)
        print()
        print(f"Output directory: {config.output_dir}")
        if shader_success:
            print(f"Shader output directory: {config.project_root / 'shader'}")
        print()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nBuild interrupted by user.")
        sys.exit(1)
    except Exception as e:
        print(f"\n\nERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
