#pragma once
#include <pgmspace.h>

// =============================================================================
// Beomaster 5500 — Settings UI (stored in flash via PROGMEM)
// =============================================================================
// Served as text/html on GET /settings
// Fetches current codes from GET /codes (JSON)
// Saves updated codes via POST /save_settings (application/x-www-form-urlencoded)
// =============================================================================

const char SETTINGS_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>Beomaster — Settings</title>
<style>
  :root {
    --bg:       #111111;
    --surface:  #1e1e1e;
    --border:   #2e2e2e;
    --gold:     #c8a96e;
    --gold-dim: #8a7248;
    --text:     #d4c5a9;
    --text-dim: #6b6055;
    --active:   #e8c97e;
    --danger:   #c85050;
    --radius:   6px;
    --gap:      8px;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: var(--bg);
    color: var(--text);
    font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
    font-size: 13px;
    letter-spacing: 0.05em;
    min-height: 100vh;
    padding: 16px;
    display: flex;
    flex-direction: column;
    align-items: center;
  }
  header {
    text-align: center;
    margin-bottom: 20px;
    padding-bottom: 16px;
    border-bottom: 1px solid var(--border);
    width: 100%;
    max-width: 480px;
  }
  header h1 {
    font-size: 18px;
    font-weight: 300;
    color: var(--gold);
    letter-spacing: 0.3em;
    text-transform: uppercase;
  }
  header a {
    display: inline-block;
    font-size: 10px;
    color: var(--text-dim);
    letter-spacing: 0.2em;
    margin-top: 6px;
    text-decoration: none;
  }
  header a:hover { color: var(--gold-dim); }
  #status {
    font-size: 11px;
    color: var(--gold-dim);
    text-align: center;
    height: 18px;
    margin-bottom: 12px;
    transition: color 0.3s;
  }
  #status.ok  { color: #6aaa6a; }
  #status.err { color: var(--danger); }
  .wrap {
    width: 100%;
    max-width: 480px;
    display: flex;
    flex-direction: column;
    gap: 16px;
  }
  .group {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 12px;
  }
  .group-label {
    font-size: 9px;
    color: var(--text-dim);
    letter-spacing: 0.25em;
    text-transform: uppercase;
    margin-bottom: 10px;
  }
  table { width: 100%; border-collapse: collapse; }
  tr + tr td { border-top: 1px solid #232323; }
  td { padding: 6px 4px; }
  td:first-child { color: var(--text); font-size: 12px; }
  td:nth-child(2) { text-align: center; }
  td:last-child {
    color: var(--text-dim);
    font-family: monospace;
    font-size: 11px;
    text-align: right;
  }
  input[type=text] {
    background: #1a1a1a;
    border: 1px solid var(--border);
    border-radius: 4px;
    color: var(--gold);
    font-family: monospace;
    font-size: 13px;
    padding: 4px 6px;
    width: 68px;
    text-align: center;
    outline: none;
    transition: border-color 0.15s;
  }
  input[type=text]:focus { border-color: var(--gold-dim); }
  input[type=text].invalid { border-color: var(--danger); color: var(--danger); }
  .actions {
    width: 100%;
    max-width: 480px;
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: var(--gap);
    margin: 4px 0 24px;
  }
  button {
    background: #282828;
    border: 1px solid var(--border);
    border-radius: var(--radius);
    color: var(--text);
    cursor: pointer;
    font-family: inherit;
    font-size: 11px;
    letter-spacing: 0.1em;
    padding: 11px 6px;
    text-transform: uppercase;
    touch-action: manipulation;
    user-select: none;
    -webkit-tap-highlight-color: transparent;
    transition: background 0.12s, border-color 0.12s, color 0.12s;
  }
  button:hover  { background: #333; border-color: var(--gold-dim); color: var(--gold); }
  button:active { background: #3a3020; border-color: var(--gold); color: var(--active); }
  button.reset  { border-color: #2e2040; color: #8878a8; }
  button.reset:hover { border-color: #5040a0; color: #a898d8; }
  button.reset:active { background: #201828; }
</style>
</head>
<body>

<header>
  <h1>Settings</h1>
  <a href="/">&#8592; Back to Remote</a>
</header>

<div id="status">Loading...</div>
<div class="wrap" id="wrap"></div>
<div class="actions">
  <button onclick="saveAll()">Save All</button>
  <button class="reset" onclick="resetAll()">Reset Defaults</button>
</div>

<script>
const SECTIONS = [
  { label: 'Power', buttons: [
    { key: 'standby',      label: 'Standby',      def: 0x0C }
  ]},
  { label: 'Volume', buttons: [
    { key: 'volume_up',    label: 'Vol +',        def: 0x60 },
    { key: 'volume_down',  label: 'Vol \u2212',   def: 0x64 },
    { key: 'mute',         label: 'Mute',         def: 0x0D },
    { key: 'loudness',     label: 'Loudness',     def: 0x3C }
  ]},
  { label: 'Tone & Balance', buttons: [
    { key: 'bass_up',      label: 'Bass +',       def: 0x70 },
    { key: 'bass_down',    label: 'Bass \u2212',  def: 0x74 },
    { key: 'treble_up',    label: 'Treble +',     def: 0x78 },
    { key: 'treble_down',  label: 'Treble \u2212',def: 0x7C },
    { key: 'bal_left',     label: 'Bal Left',     def: 0x68 },
    { key: 'bal_right',    label: 'Bal Right',    def: 0x6C }
  ]},
  { label: 'Source', buttons: [
    { key: 'fm',           label: 'FM',           def: 0x81 },
    { key: 'am',           label: 'AM',           def: 0x82 },
    { key: 'phono',        label: 'Phono',        def: 0x83 },
    { key: 'cd',           label: 'CD',           def: 0x92 },
    { key: 'tape1',        label: 'Tape 1',       def: 0x87 },
    { key: 'tape2',        label: 'Tape 2',       def: 0x88 },
    { key: 'aux',          label: 'AUX',          def: 0x8A }
  ]},
  { label: 'Tuner', buttons: [
    { key: 'preset_up',    label: 'Preset +',     def: 0x1E },
    { key: 'preset_down',  label: 'Preset \u2212',def: 0x1F },
    { key: 'tune_up',      label: 'Tune +',       def: 0x1C },
    { key: 'tune_down',    label: 'Tune \u2212',  def: 0x1D },
    { key: 'store_preset', label: 'Store Preset', def: 0x5C }
  ]},
  { label: 'Numeric', buttons: [
    { key: 'num_0',        label: '0',            def: 0x20 },
    { key: 'num_1',        label: '1',            def: 0x21 },
    { key: 'num_2',        label: '2',            def: 0x22 },
    { key: 'num_3',        label: '3',            def: 0x23 },
    { key: 'num_4',        label: '4',            def: 0x24 },
    { key: 'num_5',        label: '5',            def: 0x25 },
    { key: 'num_6',        label: '6',            def: 0x26 },
    { key: 'num_7',        label: '7',            def: 0x27 },
    { key: 'num_8',        label: '8',            def: 0x28 },
    { key: 'num_9',        label: '9',            def: 0x29 }
  ]},
  { label: 'Tape Transport', buttons: [
    { key: 'play',         label: 'Play',         def: 0x35 },
    { key: 'stop',         label: 'Stop',         def: 0x36 },
    { key: 'record',       label: 'Record',       def: 0x37 }
  ]},
  { label: 'Timer & Clock', buttons: [
    { key: 'timer',        label: 'Timer',        def: 0x44 },
    { key: 'clock',        label: 'Clock',        def: 0x43 },
    { key: 'sleep',        label: 'Sleep',        def: 0x45 }
  ]}
];

function hex(n) {
  return '0x' + n.toString(16).toUpperCase().padStart(2,'0');
}

let currentCodes = {};

async function load() {
  try {
    const r = await fetch('/codes');
    if (r.ok) currentCodes = await r.json();
  } catch(e) {}
  render();
  setStatus('', '');
}

function render() {
  const wrap = document.getElementById('wrap');
  wrap.innerHTML = '';
  for (const sec of SECTIONS) {
    const group = document.createElement('div');
    group.className = 'group';
    const lbl = document.createElement('div');
    lbl.className = 'group-label';
    lbl.textContent = sec.label;
    group.appendChild(lbl);
    const tbl = document.createElement('table');
    for (const btn of sec.buttons) {
      const val = (currentCodes[btn.key] !== undefined) ? currentCodes[btn.key] : btn.def;
      const tr = document.createElement('tr');
      tr.innerHTML =
        '<td>' + btn.label + '</td>' +
        '<td><input type="text" data-key="' + btn.key + '" data-def="' + btn.def +
        '" value="' + hex(val) + '"></td>' +
        '<td>' + hex(btn.def) + '</td>';
      tbl.appendChild(tr);
    }
    group.appendChild(tbl);
    wrap.appendChild(group);
  }
}

async function saveAll() {
  const inputs = document.querySelectorAll('input[data-key]');
  const params = new URLSearchParams();
  let valid = true;
  inputs.forEach(inp => {
    inp.classList.remove('invalid');
    const n = parseInt(inp.value.replace(/^0x/i, ''), 16);
    if (isNaN(n) || n < 0 || n > 255) {
      inp.classList.add('invalid');
      valid = false;
    } else {
      params.set(inp.dataset.key, n);
    }
  });
  if (!valid) { setStatus('Fix invalid values first', 'err'); return; }
  try {
    const r = await fetch('/save_settings', { method: 'POST', body: params });
    if (r.ok) setStatus('Saved', 'ok');
    else      setStatus('Save failed ' + r.status, 'err');
  } catch(e) { setStatus('No response', 'err'); }
}

function resetAll() {
  document.querySelectorAll('input[data-key]').forEach(inp => {
    inp.classList.remove('invalid');
    inp.value = hex(parseInt(inp.dataset.def));
  });
}

function setStatus(msg, cls) {
  const el = document.getElementById('status');
  el.textContent = msg;
  el.className = cls || '';
  if (msg) setTimeout(() => { el.textContent = ''; el.className = ''; }, 2500);
}

window.addEventListener('load', load);
</script>
</body>
</html>
)=====";
