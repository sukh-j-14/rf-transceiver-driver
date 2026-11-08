$ErrorActionPreference='Stop'
Push-Location $PSScriptRoot
try {
 New-Item -ItemType Directory -Force build | Out-Null
 gcc -std=c11 -Wall -Wextra -Werror -O2 src/radio.c src/mock.c src/main.c -o build/driver-tests.exe
 if($LASTEXITCODE -ne 0){throw 'Compilation failed'}
 & ./build/driver-tests.exe
 if($LASTEXITCODE -ne 0){throw 'Tests failed'}
 gcc -std=c11 -Wall -Wextra -Werror -O2 src/radio.c src/mock.c src/edge_tests.c -o build/edge-tests.exe
 if($LASTEXITCODE -ne 0){throw 'Edge-test compilation failed'}
 & ./build/edge-tests.exe
 if($LASTEXITCODE -ne 0){throw 'Edge tests failed'}
} finally {Pop-Location}
