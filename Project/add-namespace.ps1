param(
    [Parameter(Mandatory=$true, Position=0)]
    [string]$NamespaceName,
    
    [Parameter(Mandatory=$false, Position=1)]
    [string]$TargetDirectory = "."
)

# Validate target directory
if (-not (Test-Path $TargetDirectory)) {
    Write-Error "Target directory not found: $TargetDirectory"
    exit 1
}

# Process .h files
Get-ChildItem -Path $TargetDirectory -Filter "*.h" -Recurse | ForEach-Object {
    $filePath = $_.FullName
    Write-Host "Processing header: $filePath"
    
    $lines = Get-Content $filePath
    
    if ($lines.Count -lt 1) {
        Write-Warning "Skip: $filePath (file is empty)"
        return
    }
    
    # Search for the first class or struct declaration
    $classLineIndex = -1
    $templateLineIndex = -1
    $namespaceLineIndex = -1
    
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        
        # Skip comment lines
        if ($line -match '^\s*//') {
            continue
        }

        # Detect existing namespace declaration (named or anonymous)
        if ($namespaceLineIndex -eq -1 -and $line -match '^\s*namespace\b') {
            $namespaceLineIndex = $i
            Write-Host "  Detected namespace: $($line.Trim())"
            continue
        }
        
        # Detect template declaration
        if ($templateLineIndex -eq -1 -and $line -match '^\s*template\s*<') {
            $templateLineIndex = $i
            Write-Host "  Detected template: $($line.Trim())"
        }
        
        # Detect class or struct declaration (supports 'final')
        if ($line -match '^\s*(class|struct)\s+[\w<>]+(\s+final)?\s*(:|\{|;|$)') {
            $classLineIndex = $i
            Write-Host "  Detected class/struct: $($line.Trim())"
            break
        }
    }
    
    if ($classLineIndex -eq -1) {
        Write-Warning "Skip: $filePath (no class/struct declaration found)"
        return
    }
    
    # Decide where to insert the namespace
    # Priority: existing namespace > template > class
    $insertLineIndex = $classLineIndex
    
    if ($namespaceLineIndex -ne -1 -and $namespaceLineIndex -lt $classLineIndex) {
        $insertLineIndex = $namespaceLineIndex
        Write-Host "  Namespace insert position: before existing namespace (line $($insertLineIndex + 1))"
    } elseif ($templateLineIndex -ne -1 -and $templateLineIndex -lt $classLineIndex) {
        $insertLineIndex = $templateLineIndex
        Write-Host "  Namespace insert position: before template (line $($insertLineIndex + 1))"
    } else {
        Write-Host "  Namespace insert position: before class (line $($insertLineIndex + 1))"
    }
    
    $newLines = @()
    
    # Add lines before insert position
    if ($insertLineIndex -gt 0) {
        $newLines += $lines[0..($insertLineIndex - 1)]
    }
    
    # Add namespace
    $newLines += "namespace $NamespaceName {"
    $newLines += ""
    
    # Add lines from insert position to end
    $newLines += $lines[$insertLineIndex..($lines.Count - 1)]
    
    # Add closing brace for namespace
    $newLines += ""
    $newLines += "}; // $NamespaceName"
    
    # Write back to file
    $newLines | Set-Content $filePath -Encoding UTF8
}

# Process .cpp files
Get-ChildItem -Path $TargetDirectory -Filter "*.cpp" -Recurse | ForEach-Object {
    $filePath = $_.FullName
    Write-Host "Processing source: $filePath"
    
    $lines = Get-Content $filePath
    
    if ($lines.Count -lt 1) {
        Write-Warning "Skip: $filePath (file is empty)"
        return
    }
    
    # Get base file name (without extension)
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($filePath)
    
    # Search for include line for the corresponding header
    $includeLineIndex = -1
    for ($i = 0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -match "^\s*#include\s+[<""].*$baseName\.h[>""]") {
            $includeLineIndex = $i
            Write-Host "  Detected include: $($lines[$i].Trim())"
            break
        }
    }
    
    if ($includeLineIndex -eq -1) {
        Write-Warning "Skip: $filePath (include for corresponding header not found)"
        return
    }
    
    $newLines = @()
    
    # Add lines up to and including the include line
    $newLines += $lines[0..$includeLineIndex]
    
    # Add using namespace line
    $newLines += ""
    $newLines += "using namespace $NamespaceName;"
    
    # Add remaining lines
    if ($includeLineIndex -lt $lines.Count - 1) {
        $newLines += $lines[($includeLineIndex + 1)..($lines.Count - 1)]
    }
    
    # Write back to file
    $newLines | Set-Content $filePath -Encoding UTF8
}

Write-Host "Completed."
