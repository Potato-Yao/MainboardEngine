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


class BGFXDownloader:
    """Handles downloading and setting up bgfx.cmake dependency."""

    BGFX_CMAKE_REPO = "https://github.com/bkaradzic/bgfx.cmake.git"

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

    def is_bgfx_installed(self) -> bool:
        """Check if bgfx.cmake is already downloaded."""
        cmake_file = self.config.bgfx_cmake_dir / "CMakeLists.txt"
        bgfx_dir = self.config.bgfx_cmake_dir / "bgfx"
        return cmake_file.exists() and bgfx_dir.exists()

    def download_bgfx(self):
        """Download bgfx.cmake repository with submodules."""
        print("=" * 60)
        print("Downloading bgfx.cmake...")
        print("=" * 60)

        if not self.check_git_available():
            print("ERROR: Git is not available in the system.")
            print("Please install Git and try again.")
            sys.exit(1)

        # Create third_party directory if it doesn't exist
        self.config.third_party_dir.mkdir(parents=True, exist_ok=True)

        # Remove existing directory if it exists but is incomplete
        if self.config.bgfx_cmake_dir.exists() and not self.is_bgfx_installed():
            print(f"Removing incomplete bgfx.cmake directory...")
            shutil.rmtree(self.config.bgfx_cmake_dir)

        # Clone the repository
        if not self.config.bgfx_cmake_dir.exists():
            print(f"Cloning {self.BGFX_CMAKE_REPO}...")
            subprocess.run([
                "git", "clone",
                self.BGFX_CMAKE_REPO,
                str(self.config.bgfx_cmake_dir)
            ], check=True)

        # Initialize and update submodules
        print("Initializing submodules...")
        subprocess.run([
            "git", "submodule", "init"
        ], cwd=self.config.bgfx_cmake_dir, check=True)

        print("Updating submodules (this may take a while)...")
        subprocess.run([
            "git", "submodule", "update"
        ], cwd=self.config.bgfx_cmake_dir, check=True)

        print("✓ bgfx.cmake downloaded successfully!")
        print()

    def ensure_bgfx(self):
        """Ensure bgfx.cmake is available, download if needed."""
        if self.is_bgfx_installed():
            print("✓ bgfx.cmake is already installed")
            print()
        else:
            self.download_bgfx()


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
            print(f"✓ {version_line}")
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
        print("✓ CMake configuration successful!")
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
        print("✓ Build successful!")
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
                print(f"  ✓ {file_name}")

        if not found_files:
            print("  ⚠ Warning: Expected output files not found in MEbuild directory")
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
    print("  --help           Show this help message")
    print()


def parse_arguments():
    """Parse command line arguments."""
    args = sys.argv[1:]
    options = {
        'build_type': 'Release',
        'clean': False,
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
        elif arg == '--help' or arg == '-h':
            options['help'] = True
        else:
            print(f"Unknown option: {arg}")
            print_usage()
            sys.exit(1)

    return options


def clean_build_directory(config: BuildConfig):
    """Clean the build directory."""
    print("=" * 60)
    print("Cleaning build directory...")
    print("=" * 60)

    if config.build_dir.exists():
        print(f"Removing {config.build_dir}...")
        shutil.rmtree(config.build_dir)
        print("✓ Build directory cleaned!")
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

    # Step 1: Download dependencies
    downloader = BGFXDownloader(config)
    downloader.ensure_bgfx()

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

