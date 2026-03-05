#pragma once
#include <pgmspace.h>

// =============================================================================
// Beomaster 5500 — Web UI (stored in flash via PROGMEM)
// =============================================================================
// Served as text/html on GET /
// Button presses POST to GET /cmd?code=<decimal value>
// =============================================================================

const char WEB_UI_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Beomaster 5500</title>
<style>
  :root {
    --bg:        #111111;
    --surface:   #1e1e1e;
    --border:    #2e2e2e;
    --gold:      #c8a96e;
    --gold-dim:  #8a7248;
    --text:      #d4c5a9;
    --text-dim:  #6b6055;
    --active:    #e8c97e;
    --danger:    #c85050;
    --radius:    6px;
    --gap:       8px;
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

  header p {
    font-size: 10px;
    color: var(--text-dim);
    letter-spacing: 0.2em;
    margin-top: 4px;
  }

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

  .remote {
    width: 100%;
    max-width: 480px;
    display: flex;
    flex-direction: column;
    gap: 20px;
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

  .btn-grid {
    display: grid;
    gap: var(--gap);
  }

  .cols-1 { grid-template-columns: 1fr; }
  .cols-2 { grid-template-columns: 1fr 1fr; }
  .cols-3 { grid-template-columns: 1fr 1fr 1fr; }
  .cols-4 { grid-template-columns: 1fr 1fr 1fr 1fr; }
  .cols-5 { grid-template-columns: repeat(5, 1fr); }

  button {
    background: #282828;
    border: 1px solid #383838;
    border-radius: var(--radius);
    color: var(--text);
    cursor: pointer;
    font-family: inherit;
    font-size: 11px;
    letter-spacing: 0.1em;
    padding: 11px 6px;
    text-transform: uppercase;
    transition: background 0.12s, border-color 0.12s, color 0.12s;
    user-select: none;
    -webkit-tap-highlight-color: transparent;
  }

  button:hover  { background: #333; border-color: var(--gold-dim); color: var(--gold); }
  button:active { background: #3a3020; border-color: var(--gold); color: var(--active); }

  button.standby {
    background: #2a1818;
    border-color: #4a2020;
    color: #c86464;
    font-size: 14px;
    padding: 14px;
  }
  button.standby:hover  { background: #3a1818; border-color: var(--danger); color: #e07070; }
  button.standby:active { background: #501818; border-color: #e05050; }

  button.source {
    border-color: #2a3040;
    color: #8ab0d0;
  }
  button.source:hover  { border-color: #4a70a0; color: #aad0f0; }

  button.gold {
    border-color: var(--gold-dim);
    color: var(--gold);
  }
  button.gold:hover { border-color: var(--gold); color: var(--active); }

  .pair {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: var(--gap);
  }

  footer {
    margin-top: 24px;
    font-size: 10px;
    color: var(--text-dim);
    letter-spacing: 0.15em;
  }
</style>
</head>
<body>

<header>
  <h1>Beomaster 5500</h1>
  <p>Master Control Panel</p>
</header>

<div id="status">Ready</div>

<div class="remote">

  <!-- POWER -->
  <div class="group">
    <div class="group-label">Power</div>
    <div class="btn-grid cols-1">
      <button class="standby" onclick="send(12)">STANDBY</button>
    </div>
  </div>

  <!-- VOLUME -->
  <div class="group">
    <div class="group-label">Volume</div>
    <div class="btn-grid cols-2">
      <button onclick="send(96)">VOL +</button>
      <button onclick="send(100)">VOL -</button>
      <button onclick="send(13)">MUTE</button>
      <button onclick="send(60)">LOUDNESS</button>
    </div>
  </div>

  <!-- BALANCE & TONE -->
  <div class="group">
    <div class="group-label">Tone &amp; Balance</div>
    <div class="btn-grid cols-2" style="margin-bottom:8px;">
      <button onclick="send(112)">BASS +</button>
      <button onclick="send(116)">BASS -</button>
      <button onclick="send(120)">TREBLE +</button>
      <button onclick="send(124)">TREBLE -</button>
    </div>
    <div class="btn-grid cols-2">
      <button onclick="send(104)">BAL LEFT</button>
      <button onclick="send(108)">BAL RIGHT</button>
    </div>
  </div>

  <!-- SOURCE SELECTION -->
  <div class="group">
    <div class="group-label">Source</div>
    <div class="btn-grid cols-4">
      <button class="source" onclick="send(129)">FM</button>
      <button class="source" onclick="send(130)">AM</button>
      <button class="source" onclick="send(131)">PHONO</button>
      <button class="source" onclick="send(146)">CD</button>
      <button class="source" onclick="send(135)">TAPE 1</button>
      <button class="source" onclick="send(136)">TAPE 2</button>
      <button class="source" onclick="send(138)">AUX</button>
    </div>
  </div>

  <!-- TUNER -->
  <div class="group">
    <div class="group-label">Tuner</div>
    <div class="btn-grid cols-2" style="margin-bottom:8px;">
      <button onclick="send(30)">PRESET +</button>
      <button onclick="send(31)">PRESET -</button>
      <button onclick="send(28)">TUNE +</button>
      <button onclick="send(29)">TUNE -</button>
    </div>
    <div class="btn-grid cols-1">
      <button class="gold" onclick="send(92)">STORE PRESET</button>
    </div>
  </div>

  <!-- NUMERIC PAD -->
  <div class="group">
    <div class="group-label">Preset Number</div>
    <div class="btn-grid cols-5" style="margin-bottom:8px;">
      <button onclick="send(33)">1</button>
      <button onclick="send(34)">2</button>
      <button onclick="send(35)">3</button>
      <button onclick="send(36)">4</button>
      <button onclick="send(37)">5</button>
      <button onclick="send(38)">6</button>
      <button onclick="send(39)">7</button>
      <button onclick="send(40)">8</button>
      <button onclick="send(41)">9</button>
      <button onclick="send(32)">0</button>
    </div>
  </div>

  <!-- TAPE TRANSPORT -->
  <div class="group">
    <div class="group-label">Tape Transport</div>
    <div class="btn-grid cols-3">
      <button onclick="send(53)">PLAY</button>
      <button onclick="send(54)">STOP</button>
      <button onclick="send(55)">REC</button>
    </div>
  </div>

  <!-- TIMER / CLOCK -->
  <div class="group">
    <div class="group-label">Timer &amp; Clock</div>
    <div class="btn-grid cols-3">
      <button onclick="send(68)">TIMER</button>
      <button onclick="send(67)">CLOCK</button>
      <button onclick="send(69)">SLEEP</button>
    </div>
  </div>

</div><!-- .remote -->

<footer>B&amp;O Legacy IR &bull; 40 kHz &bull; ESP32</footer>

<script>
  async function send(code) {
    const el = document.getElementById('status');
    el.className = '';
    el.textContent = 'Sending 0x' + code.toString(16).toUpperCase().padStart(2,'0') + '...';
    try {
      const r = await fetch('/cmd?code=' + code);
      if (r.ok) {
        el.className = 'ok';
        el.textContent = 'Sent 0x' + code.toString(16).toUpperCase().padStart(2,'0');
      } else {
        el.className = 'err';
        el.textContent = 'Error ' + r.status;
      }
    } catch(e) {
      el.className = 'err';
      el.textContent = 'No response';
    }
    setTimeout(() => { el.className=''; el.textContent='Ready'; }, 2000);
  }
</script>
</body>
</html>
)=====";
