# Implementation and discussion

## What this project demonstrates

An injected transport, register-level state transitions, bounded polling, error propagation, and deterministic fault testing. Everything runs in software. These are implementation examples, not evidence of hardware measurements or professional experience.

## API contract

| Call | Requirements | Successful effect |
| --- | --- | --- |
| `radio_init` | Non-null radio, valid callbacks, nonzero poll budget | Read ID `0x5246`, reset, observe READY, mark initialized |
| `radio_tune` | Initialized radio; 700,000–6,000,000 kHz | Write FREQ and update cached frequency |
| `radio_send` | Initialized and tuned radio; non-null payload, 1–64 bytes; nonzero budget | Stage bytes, issue TX, observe DONE |

Initialize a radio before use; zero-initialize it if checking pre-initialization behavior. A non-null but uninitialized C object is not safe to inspect. The caller owns the bus context and must keep it alive for the radio's lifetime. Bus callbacks return zero on success and nonzero on error. The API is synchronous and not thread-safe.

Invalid initialization arguments return before modifying an existing radio. Once valid initialization begins, the driver clears its initialized flag and cached frequency, so an ID, reset, or readiness failure leaves it unavailable for tune/send. A failed frequency write preserves the previous cached frequency. An unsuccessful send does not automatically roll back staged bytes or cancel TX.

## Why polling is bounded

`wait_for` performs at most the supplied number of status reads. A READY or DONE bit on the final allowed read succeeds; completion after that read times out. A read error returns IO immediately. The callback itself must return: the poll budget cannot interrupt a blocking transport call. Counts are not wall-clock deadlines.

## Stale status and recovery

The fictional device clears DONE when TX is issued, then sets DONE after three status reads by default. Without that clear, a previous transaction could incorrectly satisfy a new wait. The regression sequence completes one send, then attempts another with budget 2 and expects TIMEOUT.

The mock's staged-byte buffer wraps when another byte arrives after capacity. Consecutive short sends append to the buffer; it is not a last-packet recorder. Reinitialization clears staged bytes. After a bus fault or timeout, the documented recovery is clear fault → initialize → tune → send. These tests do not prove that blindly retrying a hardware transmission is safe or that a timed-out operation never completed.

## Test strategy

The original suite covers the basic lifecycle and boundaries. The edge suite wraps the bus and fails exactly one selected call. It exercises the ID read, reset write, each readiness read, every payload write in a four-byte transfer, the TX write, and each completion read. It verifies immediate error returns and reset-based recovery. The browser model has its own 45 deterministic checks; those do not execute or replace the native tests.

The deployed UI was checked for pre-initialization STATE errors, successful initialization/tuning/sending, exact payload display, timeout at budget 2, completion at budget 3, bus errors, recovery, and invalid hexadecimal input.

## Questions to practice answering

- Why inject a bus rather than call the mock directly? Explain how transport callbacks isolate the driver and allow deterministic failures.
- Why cache frequency only after a successful write? Explain the difference between intended state and confirmed state.
- Why can TIMEOUT leave an operation in progress? Explain observation failure versus cancellation.
- What would change for real hardware? Discuss the actual register specification, transport guarantees, FIFO framing, synchronization, elapsed-time deadlines, interrupts, and measurement on a target.
- What would be required for an RTOS port? Discuss locking, task ownership, nonblocking transfers, ISR constraints, and a real scheduler integration. No RTOS port exists here.
