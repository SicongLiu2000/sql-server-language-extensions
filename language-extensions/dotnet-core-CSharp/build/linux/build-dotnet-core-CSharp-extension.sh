#!/bin/bash

# Set environment variables (adjust based on your environment)
ENL_ROOT="$(dirname "$(realpath "$0")")/../../../.."
DOTNET_EXTENSION_HOME="$ENL_ROOT/language-extensions/dotnet-core-CSharp"
DOTNET_EXTENSION_WORKING_DIR="$ENL_ROOT/build-output/dotnet-core-CSharp-extension/linux"

# Clean up the previous working directory if it exists
if [ -d "$DOTNET_EXTENSION_WORKING_DIR" ]; then
    rm -rf "$DOTNET_EXTENSION_WORKING_DIR"
fi
mkdir -p "$DOTNET_EXTENSION_WORKING_DIR"

# Loop to handle different build configurations (Debug, Release, etc.)
while [ ! -z "$1" ]; do
    # Set build configuration (default to "release" if not specified)
    BUILD_CONFIGURATION="${1:-release}"

    # Ensure build configuration is "release" if not "debug"
    if [[ ! "$BUILD_CONFIGURATION" =~ ^[Dd]ebug$ ]]; then
        BUILD_CONFIGURATION="release"
    fi

    # Output directory and target DLL name
    TARGET="$ENL_ROOT/build-output/dotnet-core-CSharp-extension/target/$BUILD_CONFIGURATION"

    # Remove the output directory if it exists, then create it
    if [ -d "$TARGET" ]; then
        rm -rf "$TARGET"
    fi
    mkdir -p "$TARGET"

    # Set build output directory
    BUILD_OUTPUT="$DOTNET_EXTENSION_WORKING_DIR/$BUILD_CONFIGURATION"
    rm -rf "$BUILD_OUTPUT"
    mkdir -p "$BUILD_OUTPUT"

    # Set paths for source, include, and libraries
    DOTNET_NATIVE_SRC="$DOTNET_EXTENSION_HOME/src/native"
    DOTNET_NATIVE_INCLUDE="$DOTNET_EXTENSION_HOME/include"
    EXTENSION_HOST_INCLUDE="$ENL_ROOT/extension-host/include"
    DOTNET_NATIVE_LIB="$DOTNET_EXTENSION_HOME/lib"

    # Build the native C# extension using g++
    if [ "$BUILD_CONFIGURATION" == "debug" ]; then
		g++ -fPIC -shared "$DOTNET_NATIVE_SRC"/*.cpp -I "$DOTNET_NATIVE_INCLUDE" -I "$EXTENSION_HOST_INCLUDE" -DDEBUG -g -o "$BUILD_OUTPUT/nativecsharpextension.so"
	else
		g++ -fPIC -shared "$DOTNET_NATIVE_SRC"/*.cpp -I "$DOTNET_NATIVE_INCLUDE" -I "$EXTENSION_HOST_INCLUDE" -O2 -o "$BUILD_OUTPUT/nativecsharpextension.so"
	fi

    # Check for build errors
    if [ $? -ne 0 ]; then
        echo "Error: Failed to build nativecsharpextension for configuration=$BUILD_CONFIGURATION"
        exit 1
    fi

    echo "[Info] Copying dependent libraries..."
    cp "$DOTNET_NATIVE_LIB/hostfxr.dll" "$BUILD_OUTPUT/"

    echo "[Info] Building Microsoft.SqlServer.CSharpExtension DLL..."

    # Build the .NET managed project using dotnet CLI
    DOTNET_MANAGED_SRC="$DOTNET_EXTENSION_HOME/src/managed"
    dotnet build "$DOTNET_MANAGED_SRC/Microsoft.SqlServer.CSharpExtension.csproj" -m -c "$BUILD_CONFIGURATION" -o "$BUILD_OUTPUT" --no-dependencies

    # Check for build errors
    if [ $? -ne 0 ]; then
        echo "Error: Failed to build Microsoft.SqlServer.CSharpExtension.dll for configuration=$BUILD_CONFIGURATION"
        exit 1
    fi

    echo "Success: Built dotnet-core-CSharp-extension for $BUILD_CONFIGURATION configuration."

    # Shift to next argument for multiple configurations
    shift
done

exit 0
