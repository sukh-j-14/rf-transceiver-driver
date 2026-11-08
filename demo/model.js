/* Browser teaching model of radio.c + mock.c; does not execute native C. */
(function (root) {
  'use strict';
  class RadioModel {
    constructor() { this.reset(); }
    reset() {
      this.registers = [0x5246, 0, 0, 0, 0];
      this.initialized = false; this.frequency = 0;
      this.payload = []; this.polls = 0; this.delay = 3;
      this.fail = false; this.stuck = false; this.trace = [];
    }
    record(message) { this.trace.push(message); }
    read(reg) {
      if (this.fail) { this.record('READ → bus fault'); throw new Error('IO'); }
      if (reg === 2 && !this.stuck && ++this.polls >= this.delay) {
        this.registers[2] |= this.registers[1] === 2 ? 2 : 1;
      }
      this.record(`READ ${['ID','CONTROL','STATUS','FREQ','DATA'][reg]} → ${this.registers[reg]}`);
      return this.registers[reg];
    }
    write(reg, value) {
      if (this.fail) { this.record('WRITE → bus fault'); throw new Error('IO'); }
      if (reg === 1) {
        if (value === 1) { this.registers[2] = 0; this.payload = []; }
        if (value === 2) this.registers[2] &= ~2;
        this.polls = 0;
      }
      if (reg === 4) {
        if (this.payload.length === 64) this.payload = [];
        this.payload.push(value);
      }
      this.registers[reg] = value;
      this.record(`WRITE ${['ID','CONTROL','STATUS','FREQ','DATA'][reg]} ← ${value}`);
    }
    wait(mask, budget) {
      for (let i = 0; i < budget; i++) if ((this.read(2) & mask) === mask) return 'OK';
      return 'TIMEOUT';
    }
    init(budget) {
      if (!Number.isInteger(budget) || budget < 1 || budget > 10000) return 'INVALID';
      this.initialized = false; this.frequency = 0;
      try {
        if (this.read(0) !== 0x5246) return 'INVALID';
        this.write(1, 1);
        const result = this.wait(1, budget);
        this.initialized = result === 'OK'; return result;
      } catch (_) { return 'IO'; }
    }
    tune(khz) {
      if (!this.initialized) return 'STATE';
      if (!Number.isInteger(khz) || khz < 700000 || khz > 6000000) return 'INVALID';
      try { this.write(3, khz); this.frequency = khz; return 'OK'; }
      catch (_) { return 'IO'; }
    }
    send(bytes, budget) {
      if (!this.initialized || !this.frequency) return 'STATE';
      if (!Array.isArray(bytes) || !bytes.length || bytes.length > 64 ||
          bytes.some(b => !Number.isInteger(b) || b < 0 || b > 255) ||
          !Number.isInteger(budget) || budget < 1 || budget > 10000) return 'INVALID';
      try {
        for (const byte of bytes) this.write(4, byte);
        this.write(1, 2); return this.wait(2, budget);
      } catch (_) { return 'IO'; }
    }
  }
  if (typeof module !== 'undefined') module.exports = RadioModel;
  else root.RadioModel = RadioModel;
})(typeof window !== 'undefined' ? window : globalThis);
