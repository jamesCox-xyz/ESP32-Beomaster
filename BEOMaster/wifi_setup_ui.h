#pragma once
#include <pgmspace.h>

// =============================================================================
// Beomaster 5500 — WiFi Setup UI (stored in flash via PROGMEM)
// =============================================================================

const char WIFI_SETUP_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Beomaster WiFi Setup</title>
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
    --radius:    6px;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: var(--bg); color: var(--text);
    font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
    font-size: 13px; letter-spacing: 0.05em;
    min-height: 100vh; padding: 16px;
    display: flex; flex-direction: column; align-items: center; justify-content: center;
  }
  .container {
    background: var(--surface); border: 1px solid var(--border);
    border-radius: var(--radius); padding: 24px;
    width: 100%; max-width: 320px; text-align: center;
  }
  h1 { font-size: 16px; font-weight: 300; color: var(--gold); letter-spacing: 0.2em; text-transform: uppercase; margin-bottom: 24px; }
  form { display: flex; flex-direction: column; gap: 16px; }
  .field { display: flex; flex-direction: column; text-align: left; }
  label { font-size: 10px; color: var(--text-dim); letter-spacing: 0.1em; text-transform: uppercase; margin-bottom: 6px; }
  input { 
    background: #0d0d0d; border: 1px solid var(--border); border-radius: 4px; 
    color: var(--text); padding: 12px; font-size: 14px; 
    transition: border-color 0.2s;
  }
  input:focus { outline: none; border-color: var(--gold-dim); }
  button {
    background: #282828; border: 1px solid var(--gold-dim); border-radius: var(--radius);
    color: var(--gold); cursor: pointer; font-size: 12px; letter-spacing: 0.1em;
    padding: 14px; text-transform: uppercase; margin-top: 8px;
    transition: background 0.1s, border-color 0.1s, color 0.1s;
  }
  button:hover { background: #333; border-color: var(--gold); color: var(--active); }
  button:active { background: #3a3020; border-color: var(--gold); color: var(--active); }
</style>
</head>
<body>
<div class="container">
  <h1>WiFi Setup</h1>
  <form action="/save_wifi" method="POST">
    <div class="field">
      <label for="ssid">Network Name (SSID)</label>
      <input type="text" id="ssid" name="ssid" required placeholder="My Home Network">
    </div>
    <div class="field">
      <label for="pass">Password</label>
      <input type="password" id="pass" name="pass" placeholder="Optional">
    </div>
    <button type="submit">Save & Restart</button>
  </form>
</div>
</body>
</html>
)=====";
