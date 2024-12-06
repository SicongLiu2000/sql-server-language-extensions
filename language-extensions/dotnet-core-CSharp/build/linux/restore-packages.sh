#!/bin/bash

# Error check function (Moved to the top)
CHECK_ERROR() {
    if [ "$1" -ne 0 ]; then
        echo "$2"
        exit $1
    fi
}

# Step 1: Update package list
echo "Updating package list..."
sudo apt-get update -y
CHECK_ERROR $? "Error: Failed to update package list."

# Step 2: Install .NET SDK 8.0
echo "Installing .NET SDK 8.0..."
sudo apt-get install -y dotnet-sdk-8.0
CHECK_ERROR $? "Error: Failed to install dotnet-sdk-8.0."

# Step 3: Verify installation
echo "Verifying .NET SDK installation..."
dotnet --version
CHECK_ERROR $? "Error: .NET SDK installation failed."

# Step 4: Set the root directory relative to the script's location
ENL_ROOT="$(dirname "$(realpath "$0")")/../../../.."
echo $ENL_ROOT
# Step 5: Root-level restore packages
"$ENL_ROOT/restore-packages.sh"
CHECK_ERROR $? "Error: Failed to restore common NuGet packages."

# Step 6: Run dotnet restore
dotnet restore "$ENL_ROOT/language-extensions/dotnet-core-CSharp/src/managed/Microsoft.SqlServer.CSharpExtension.csproj"
CHECK_ERROR $? "Error: dotnet restore failed."

# Exit with the same error level
exit $?
