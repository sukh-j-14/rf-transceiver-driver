# Simulated RF Transceiver Driver in C

Software-only portfolio project for learning modem/RF software concepts.

[Live browser demo](https://rf-transceiver-driver.vercel.app/) · [GitHub repository](https://github.com/sukh-j-14/rf-transceiver-driver)

The native implementation is C. The browser companion is a separate JavaScript teaching model; it does not execute C, FreeRTOS, or RF hardware.

## Build and run on Windows

Prerequisites: GCC/MinGW in PATH and PowerShell.

```powershell
.\build.ps1
```

Run from PowerShell in this folder. Build outputs go into `build/`, which is excluded from Git.

## Architecture and scope

The API uses an injected register bus; mock.c implements a fictional register bank. No actual RF or chipset interaction occurs. Poll budgets bound waits; they are not hardware timeouts in milliseconds.

## Verified on 8 November 2026

139 checks passed locally: 24 original driver checks and 115 additional edge checks. Tests exercise invalid arguments, frequency boundaries, initialization and send timeouts, payload transfer, stale completion status, and recovery. The edge suite injects failures at every bus operation in initialization and a four-byte send, verifies exact poll limits, and checks recovery after partial transfers.

`build.ps1` compiles and runs both native suites with warnings treated as errors. Another 45 JavaScript model checks pass with `node demo/model.test.cjs`. GitHub Actions runs both native suites and the model checks on each push and pull request; see the repository's Actions tab for hosted results.

Compiler: MinGW.org GCC 6.3.0 on Windows. Native tests passed with normal process access; restricted process execution could not start this compiler. This project does not include an RTOS. Do not use host timing to claim embedded latency.

## Browser demo

Open the live demo, or serve `demo/` with any static HTTP server. No package installation is required. For example, with Python installed: `python -m http.server 8080 --directory demo`, then open `http://localhost:8080`.

1. Initialize with poll budget 3, tune to 3,500,000 kHz, and send `01 02 03 FF`.
2. Set budget 2 and send to observe TIMEOUT; restore 3 to observe completion.
3. Enable a bus fault to observe IO, then clear it, initialize, retune, and send to recover.
4. Enable stuck status and initialize to observe readiness timeout. Clear the fault and reinitialize.

The browser bounds budgets to 1–10,000 to keep the UI responsive. Payloads use exactly two hex digits per byte. Registers and staged payload intentionally follow the existing mock, including its wrapping buffer. All state stays in memory and resets on reload. No database is needed.

Vercel serves `demo/` using the root `vercel.json`; there are no server-side functions, secrets, or native compilation in the deployment. To redeploy from a linked Vercel CLI checkout, run `vercel --prod` at the repository root. The first deployment was uploaded through the connected Vercel tools.

## Build on Linux / macOS

With a C11 compiler and Make installed, run `make test`. Node.js 22 or newer is needed only for `make test-demo`. The native driver has no third-party dependencies.

## Understanding the driver

`src/radio.c` talks only through two injected bus callbacks. `src/mock.c` supplies those callbacks using an in-memory fictional register bank. This lets the tests control readiness, completion, and bus errors without physical hardware.

Initialization reads the device ID, issues reset, and polls for READY. Only successful initialization enables tuning. Tuning accepts 700,000 through 6,000,000 kHz, including both endpoints, and updates the cached frequency only after a successful register write. Sending requires a tuned radio, writes 1–64 payload bytes, issues TX, and polls for DONE. The mock clears stale DONE when it receives TX.

A poll budget limits status reads; it does not measure elapsed time. A failed send can leave partially staged bytes or an operation in progress. The tested recovery sequence is successful reinitialization followed by tuning before another send. The API does not promise rollback, cancellation, or safe concurrent access; callers must serialize operations. The mock payload buffer records staged writes and wraps at capacity; it is not a packet queue or a radio propagation model.

`src/main.c` contains the original checks. `src/edge_tests.c` wraps the mock bus to fail one selected operation, making read and write failure paths independently testable.

## Limitations

These implementations and test results do not establish professional RF, RTOS, LTE, or 5G experience. The register layout and frequency range are fictional API choices. No physical hardware is required. The model does not simulate modulation, propagation, real transport ordering, or calibrated timing. Persistence is intentionally omitted because the workbench has no durable data requirement.

See [the implementation discussion](docs/implementation.md) for API contracts, recovery reasoning, and interview preparation prompts.
