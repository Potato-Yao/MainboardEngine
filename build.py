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


class NativeBuilder:
    """Handles building the native library using CMake."""

    def __init__(self, config: BuildConfig):
        self.config = config

    def check_cmake_available(self) -> bool:
        """Check if CMake is available in the system."""
        try:
            result = subprocess.run(["cmake", "--version"],
                                  capture_output=True,
                                  check=True,
                                  text=True)
            version_line = result.stdout.split('\n')[0]
            print(f"[OK] {version_line}")
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
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

        if not self.check_cmake_available():
            print("ERROR: CMake is not available in the system.")
            print("Please install CMake (version 3.20 or higher) and try again.")
            sys.exit(1)

        # Create build directory
        self.config.build_dir.mkdir(parents=True, exist_ok=True)
        self.config.output_dir.mkdir(parents=True, exist_ok=True)

        # Prepare CMake command
        cmake_cmd = ["cmake", str(self.config.native_dir)]

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

        # Prepare build command
        build_cmd = [
            "cmake",
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

    # Step 2: Build native library (unless download-only)
    if not options['download_only']:
        builder = NativeBuilder(config)
        builder.configure_cmake()
        builder.build_native()
        builder.verify_output()

        print("=" * 60)
        print("  Build completed successfully!")
        print("=" * 60)
        print()
        print(f"Output directory: {config.output_dir}")
        print()
    else:
        print("=" * 60)
        print("  Dependencies downloaded successfully!")
        print("=" * 60)
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

