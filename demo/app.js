'use strict';
const model = new window.RadioModel();
const el = id => document.getElementById(id);
const messages = {
  OK: 'Operation completed successfully.',
  INVALID: 'Invalid input. Check the frequency, hexadecimal bytes, or poll budget.',
  STATE: 'This operation requires initialization and, for sending, a tuned frequency.',
  IO: 'The bus reported an error. Clear the fault, initialize, and tune before retrying.',
  TIMEOUT: 'The status-read budget expired. Clear a stuck-status fault or increase the budget. Reinitialize and retune to recover.'
};
function render(result) {
  el('state').textContent = !model.initialized ? 'UNINITIALIZED' : model.frequency ? 'TUNED' : 'READY';
  el('tuned').textContent = model.frequency ? `${(model.frequency / 1000).toLocaleString()} MHz` : '—';
  const meanings = ['Device identity', '1 = RESET · 2 = TX', `READY ${model.registers[2]&1 ? '1' : '0'} · DONE ${model.registers[2]&2 ? '1' : '0'}`, 'Frequency in kHz', 'Last staged byte'];
  el('registers').replaceChildren(...model.registers.map((value, i) => {
    const row = document.createElement('tr');
    for (const text of [['ID','CONTROL','STATUS','FREQ','DATA'][i],`0x${value.toString(16).toUpperCase().padStart(8,'0')}`,meanings[i]]) {
      const cell = document.createElement('td'); cell.textContent = text; row.append(cell);
    }
    return row;
  }));
  el('length').textContent = `${model.payload.length} / 64 bytes`;
  el('buffer').textContent = model.payload.length ? model.payload.map(b => b.toString(16).toUpperCase().padStart(2,'0')).join(' ') : 'Empty';
  if (result) {
    el('result').textContent = result; el('result').dataset.result = result;
    el('feedback').textContent = messages[result];
    el('trace').textContent = model.trace.length ? model.trace.join('\n') : 'No bus operations — rejected before accessing registers.';
  }
}
function run(command) {
  model.trace = []; model.fail = el('bus-fault').checked; model.stuck = el('stuck').checked;
  const budget = Number(el('budget').value);
  let result;
  if (command === 'init') result = model.init(budget);
  if (command === 'tune') result = model.tune(Number(el('frequency').value));
  if (command === 'send') {
    const text = el('payload').value.trim();
    const bytes = text ? text.split(/\s+/) : [];
    // State validation precedes payload validation, just as in the native API.
    result = !model.initialized || !model.frequency ? 'STATE' :
      bytes.some(b => !/^[0-9a-f]{2}$/i.test(b)) ? 'INVALID' : model.send(bytes.map(b => parseInt(b,16)),budget);
  }
  render(result);
}
for (const command of ['init','tune','send']) el(command).addEventListener('click', () => run(command));
el('reset').addEventListener('click', () => {
  model.reset(); el('budget').value = 3; el('frequency').value = 3500000;
  el('payload').value = '01 02 03 FF'; el('bus-fault').checked = false; el('stuck').checked = false;
  el('result').textContent = '—'; delete el('result').dataset.result;
  el('feedback').textContent = 'Start by initializing the radio. Default readiness takes three status reads.';
  el('trace').textContent = 'Waiting for a command…'; render();
});
render();
