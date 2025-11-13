#ifndef WEB_PAGES_H
#define WEB_PAGES_H

const char CONFIG_PAGE_TEMPLATE[] PROGMEM = R"HTMLCODE(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>StationBoards.co.uk - Configuration</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
      color: #333;
    }

    .container {
      max-width: 900px;
      margin: 0 auto;
    }

    .header {
      background: white;
      border-radius: 12px;
      padding: 30px;
      margin-bottom: 20px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      text-align: center;
    }

    .header h1 {
      color: #667eea;
      font-size: 28px;
      margin-bottom: 5px;
    }

    .header .subtitle {
      color: #666;
      font-size: 14px;
    }

    .status-bar {
      background: white;
      border-radius: 12px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 15px;
    }

    .status-item {
      text-align: center;
      padding: 15px;
      background: #f8f9fa;
      border-radius: 8px;
    }

    .status-item .label {
      font-size: 12px;
      color: #666;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      margin-bottom: 5px;
    }

    .status-item .value {
      font-size: 18px;
      font-weight: bold;
      color: #333;
    }

    .status-badge {
      display: inline-block;
      padding: 4px 12px;
      border-radius: 20px;
      font-size: 12px;
      font-weight: 600;
    }

    .status-badge.online {
      background: #d4edda;
      color: #155724;
    }

    .status-badge.offline {
      background: #f8d7da;
      color: #721c24;
    }

    .tabs {
      background: white;
      border-radius: 12px 12px 0 0;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      overflow: hidden;
    }

    .tab-buttons {
      display: flex;
      border-bottom: 2px solid #e0e0e0;
    }

    .tab-button {
      flex: 1;
      padding: 18px 20px;
      background: #f8f9fa;
      border: none;
      cursor: pointer;
      font-size: 15px;
      font-weight: 600;
      color: #666;
      transition: all 0.3s ease;
      border-bottom: 3px solid transparent;
    }

    .tab-button:hover {
      background: #e9ecef;
      color: #333;
    }

    .tab-button.active {
      background: white;
      color: #667eea;
      border-bottom-color: #667eea;
    }

    .tab-content {
      display: none;
      padding: 30px;
      background: white;
      border-radius: 0 0 12px 12px;
    }

    .tab-content.active {
      display: block;
    }

    .card {
      background: white;
      border-radius: 12px;
      padding: 30px;
      margin-bottom: 20px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }

    .card h2 {
      color: #333;
      font-size: 20px;
      margin-bottom: 20px;
      padding-bottom: 10px;
      border-bottom: 2px solid #667eea;
    }

    .form-group {
      margin-bottom: 25px;
    }

    .form-group label {
      display: block;
      font-weight: 600;
      margin-bottom: 8px;
      color: #333;
      font-size: 14px;
    }

    .form-group .help-text {
      font-size: 12px;
      color: #666;
      margin-top: 5px;
      display: block;
    }

    .form-group input[type="text"],
    .form-group input[type="password"],
    .form-group input[type="number"],
    .form-group select {
      width: 100%;
      padding: 12px 15px;
      border: 2px solid #e0e0e0;
      border-radius: 8px;
      font-size: 14px;
      transition: all 0.3s ease;
      font-family: inherit;
    }

    .form-group input[type="text"]:focus,
    .form-group input[type="password"]:focus,
    .form-group input[type="number"]:focus,
    .form-group select:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }

    .form-group input.error {
      border-color: #dc3545;
    }

    .form-group input.success {
      border-color: #28a745;
    }

    .form-group .error-message {
      color: #dc3545;
      font-size: 12px;
      margin-top: 5px;
      display: none;
    }

    .form-group input.error + .error-message {
      display: block;
    }

    .form-row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
    }

    .form-row.form-row-three {
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
    }

    .button-group {
      display: flex;
      gap: 15px;
      flex-wrap: wrap;
    }

    .btn {
      padding: 14px 28px;
      border: none;
      border-radius: 8px;
      font-size: 15px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      text-decoration: none;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      font-family: inherit;
    }

    .btn:disabled {
      opacity: 0.6;
      cursor: not-allowed;
    }

    .btn-primary {
      background: #667eea;
      color: white;
      flex: 1;
    }

    .btn-primary:hover:not(:disabled) {
      background: #5568d3;
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
    }

    .btn-secondary {
      background: #6c757d;
      color: white;
      flex: 1;
    }

    .btn-secondary:hover:not(:disabled) {
      background: #5a6268;
      transform: translateY(-2px);
    }

    .btn-danger {
      background: #dc3545;
      color: white;
    }

    .btn-danger:hover:not(:disabled) {
      background: #c82333;
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(220, 53, 69, 0.4);
    }

    .btn-outline {
      background: transparent;
      border: 2px solid #667eea;
      color: #667eea;
    }

    .btn-outline:hover:not(:disabled) {
      background: #667eea;
      color: white;
    }

    .btn .spinner {
      display: none;
      width: 16px;
      height: 16px;
      border: 2px solid rgba(255,255,255,0.3);
      border-radius: 50%;
      border-top-color: white;
      animation: spin 0.6s linear infinite;
    }

    .btn.loading .spinner {
      display: block;
    }

    .btn.loading .btn-text {
      display: none;
    }

    @keyframes spin {
      to { transform: rotate(360deg); }
    }

    .preset-stations {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
      gap: 10px;
      margin-bottom: 15px;
    }

    .preset-btn {
      padding: 10px;
      background: #f8f9fa;
      border: 2px solid #e0e0e0;
      border-radius: 6px;
      font-size: 13px;
      cursor: pointer;
      transition: all 0.2s ease;
      font-weight: 500;
    }

    .preset-btn:hover {
      background: #667eea;
      color: white;
      border-color: #667eea;
    }

    .advanced-settings {
      margin-top: 20px;
      border-top: 1px solid #e0e0e0;
      padding-top: 20px;
    }

    .advanced-toggle {
      display: flex;
      align-items: center;
      gap: 10px;
      cursor: pointer;
      color: #667eea;
      font-weight: 600;
      margin-bottom: 15px;
    }

    .advanced-toggle:hover {
      color: #5568d3;
    }

    .advanced-content {
      display: none;
    }

    .advanced-content.show {
      display: block;
    }

    .network-list {
      max-height: 300px;
      overflow-y: auto;
      border: 2px solid #e0e0e0;
      border-radius: 8px;
      margin-top: 10px;
    }

    .network-item {
      padding: 12px 15px;
      border-bottom: 1px solid #e0e0e0;
      cursor: pointer;
      transition: background 0.2s ease;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .network-item:last-child {
      border-bottom: none;
    }

    .network-item:hover {
      background: #f8f9fa;
    }

    .network-item .network-name {
      font-weight: 600;
      color: #333;
    }

    .network-item .network-signal {
      font-size: 12px;
      color: #666;
    }

    .signal-strength {
      display: inline-block;
      width: 20px;
      text-align: center;
    }

    .modal {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background: rgba(0,0,0,0.5);
      z-index: 1000;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }

    .modal.show {
      display: flex;
    }

    .modal-content {
      background: white;
      border-radius: 12px;
      padding: 30px;
      max-width: 500px;
      width: 100%;
      box-shadow: 0 10px 40px rgba(0,0,0,0.2);
      animation: modalSlideIn 0.3s ease;
    }

    @keyframes modalSlideIn {
      from {
        transform: translateY(-50px);
        opacity: 0;
      }
      to {
        transform: translateY(0);
        opacity: 1;
      }
    }

    .modal-header {
      margin-bottom: 20px;
    }

    .modal-header h3 {
      color: #333;
      font-size: 22px;
      margin-bottom: 5px;
    }

    .modal-header p {
      color: #666;
      font-size: 14px;
    }

    .modal-footer {
      margin-top: 25px;
      display: flex;
      gap: 10px;
      justify-content: flex-end;
    }

    .live-preview {
      background: white;
      border-radius: 12px;
      padding: 30px;
      margin-bottom: 20px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }

    .live-preview h2 {
      color: #333;
      font-size: 20px;
      margin-bottom: 20px;
      padding-bottom: 10px;
      border-bottom: 2px solid #667eea;
    }

    .display-frame {
      background: #000;
      color: #fff;
      padding: 25px;
      border-radius: 8px;
      font-family: 'Courier New', monospace;
      min-height: 220px;
      position: relative;
      border: 3px solid #333;
    }

    .display-waiting {
      text-align: center;
      color: #666;
      padding: 60px 0;
      font-size: 14px;
    }

    .ws-status {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 15px;
      margin-top: 15px;
      font-size: 12px;
      color: #666;
    }

    .ws-indicator {
      display: inline-block;
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: #dc3545;
      animation: pulse 2s infinite;
    }

    .ws-indicator.connected {
      background: #28a745;
    }

    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }

    .toast {
      position: fixed;
      top: 20px;
      right: 20px;
      padding: 15px 20px;
      background: white;
      border-radius: 8px;
      box-shadow: 0 4px 12px rgba(0,0,0,0.15);
      z-index: 2000;
      display: flex;
      align-items: center;
      gap: 12px;
      min-width: 300px;
      animation: slideInRight 0.3s ease;
    }

    @keyframes slideInRight {
      from {
        transform: translateX(400px);
        opacity: 0;
      }
      to {
        transform: translateX(0);
        opacity: 1;
      }
    }

    .toast.info { border-left: 4px solid #2196F3; }
    .toast.success { border-left: 4px solid #4CAF50; }
    .toast.warning { border-left: 4px solid #ff9800; }
    .toast.error { border-left: 4px solid #f44336; }

    .toast-icon {
      font-size: 20px;
    }

    .toast-message {
      flex: 1;
      font-size: 14px;
      color: #333;
    }

    @media (max-width: 768px) {
      .form-row {
        grid-template-columns: 1fr;
      }

      .button-group {
        flex-direction: column;
      }

      .btn {
        width: 100%;
      }

      .status-bar {
        grid-template-columns: 1fr;
      }

      .preset-stations {
        grid-template-columns: repeat(2, 1fr);
      }
    }

    .info-tooltip {
      display: inline-block;
      width: 18px;
      height: 18px;
      background: #667eea;
      color: white;
      border-radius: 50%;
      text-align: center;
      line-height: 18px;
      font-size: 12px;
      font-weight: bold;
      cursor: help;
      margin-left: 5px;
    }

    .autocomplete-wrapper {
      position: relative;
    }

    .autocomplete-results {
      position: absolute;
      top: 100%;
      left: 0;
      right: 0;
      background: white;
      border: 2px solid #667eea;
      border-top: none;
      border-radius: 0 0 8px 8px;
      max-height: 300px;
      overflow-y: auto;
      z-index: 1000;
      display: none;
      box-shadow: 0 4px 12px rgba(0,0,0,0.15);
    }

    .autocomplete-results.show {
      display: block;
    }

    .autocomplete-item {
      padding: 12px 15px;
      cursor: pointer;
      border-bottom: 1px solid #f0f0f0;
      transition: background 0.2s ease;
    }

    .autocomplete-item:last-child {
      border-bottom: none;
    }

    .autocomplete-item:hover {
      background: #f8f9fa;
    }

    .autocomplete-item .station-name {
      font-weight: 600;
      color: #333;
      display: block;
    }

    .autocomplete-item .station-code {
      font-size: 12px;
      color: #667eea;
      font-weight: 500;
    }

    .autocomplete-loading {
      padding: 20px;
      text-align: center;
      color: #666;
      font-size: 14px;
    }

    .autocomplete-no-results {
      padding: 20px;
      text-align: center;
      color: #999;
      font-size: 14px;
    }
  </style>
</head>
<body>
  <div class="container">
    <!-- Header -->
    <div class="header">
      <h1>🚆 StationBoards.co.uk</h1>
      <p class="subtitle">Live Train Departure Board Configuration</p>
    </div>

    <!-- Status Bar -->
    <div class="status-bar">
      <div class="status-item">
        <div class="label">Device Status</div>
        <div class="value">
          <span class="status-badge online">Online</span>
        </div>
        <div style="margin-top: 8px; font-size: 12px; color: #666;" id="wifiStrength">
          📶 <span id="rssiValue">--</span> dBm
        </div>
      </div>
      <div class="status-item">
        <div class="label">Current Station</div>
        <div class="value" id="currentStation">{STATION_NAME}</div>
      </div>
      <div class="status-item">
        <div class="label">Device ID</div>
        <div class="value" style="font-size: 14px;">{DEVICE_ID}</div>
      </div>
      <div class="status-item">
        <div class="label">IP Address</div>
        <div class="value" style="font-size: 14px;">{IP}</div>
      </div>
    </div>

    <!-- Tabs -->
    <div class="tabs">
      <div class="tab-buttons">
        <button class="tab-button active" onclick="switchTab(0)">Station Board</button>
        <button class="tab-button" onclick="switchTab(1)">Network Settings</button>
      </div>

      <!-- Tab 1: Station Board Configuration -->
      <div class="tab-content active" id="tab-0">
        <div id="configForm">
      <div class="card">
        <h2>🚉 Station Configuration</h2>
        
        <div class="form-group">
          <label for="station">
            Station Code (CRS)
            <span class="info-tooltip" title="Three-letter National Rail station code">?</span>
          </label>
          <div class="preset-stations">
            <button type="button" class="preset-btn" onclick="setStation('PAD')">PAD<br><small>Paddington</small></button>
            <button type="button" class="preset-btn" onclick="setStation('VIC')">VIC<br><small>Victoria</small></button>
            <button type="button" class="preset-btn" onclick="setStation('WAT')">WAT<br><small>Waterloo</small></button>
            <button type="button" class="preset-btn" onclick="setStation('KGX')">KGX<br><small>Kings Cross</small></button>
            <button type="button" class="preset-btn" onclick="setStation('EUS')">EUS<br><small>Euston</small></button>
            <button type="button" class="preset-btn" onclick="setStation('LST')">LST<br><small>Liverpool St</small></button>
          </div>
          <div class="autocomplete-wrapper">
            <input type="text" id="station" name="station" value="{STATION}" placeholder="Type station name or code..." required maxlength="50">
            <div id="stationAutocomplete" class="autocomplete-results"></div>
          </div>
          <span class="help-text">Start typing to search for a station</span>
        </div>

        <div class="form-row">
          <div class="form-group">
            <label for="interval">
              Refresh Interval (seconds)
              <span class="info-tooltip" title="How often to fetch new departure data">?</span>
            </label>
            <input type="number" id="interval" name="interval" value="{INTERVAL}" min="30" max="600" required>
            <span class="help-text">Recommended: 60-120 seconds</span>
          </div>

          <div class="form-group">
            <label for="scrollspeed">
              Scroll Speed (ms)
              <span class="info-tooltip" title="Lower = faster scrolling">?</span>
            </label>
            <input type="number" id="scrollspeed" name="scrollspeed" value="{SCROLL}" min="10" max="200" required>
            <span class="help-text">Recommended: 50-100ms</span>
          </div>
        </div>
      </div>

      <div class="card">
        <h2>Display Options</h2>
        
        <div class="form-group">
          <label for="mode">Display Mode</label>
          <select id="mode" name="mode">
            <option value="0"{MODE_SEL_0}>Standard View</option>
            <option value="1"{MODE_SEL_1}>Calling At Mode (shows stops)</option>
          </select>
          <span class="help-text">Calling At mode shows detailed stops for the first train</span>
        </div>

        <div class="form-group">
          <label for="showstation">Show Station Name at Top</label>
          <select id="showstation" name="showstation">
            <option value="1"{SHOWSTATION_SEL_1}>Show Station Name</option>
            <option value="0"{SHOWSTATION_SEL_0}>Hide Station Name (adds extra service line)</option>
          </select>
          <span class="help-text">Hiding the station name adds an extra service at the top for more trains</span>
        </div>

        <div class="form-group">
          <label for="extra">Extra Services on Bottom Line</label>
          <select id="extra" name="extra">
            <option value="0"{EXTRA_SEL_0}>No Extra Services</option>
            <option value="1"{EXTRA_SEL_1}>1 Extra Service</option>
            <option value="2"{EXTRA_SEL_2}>2 Extra Services</option>
            <option value="3"{EXTRA_SEL_3}>3 Extra Services</option>
            <option value="4"{EXTRA_SEL_4}>4 Extra Services</option>
          </select>
          <span class="help-text">Number of additional services that rotate on the bottom line</span>
        </div>

        <div class="form-group">
          <label for="rotationspeed">Bottom Line Rotation Speed (seconds)</label>
          <input type="number" id="rotationspeed" name="rotationspeed" value="{ROTATION}" min="5" max="60" required>
          <span class="help-text">How often the bottom line alternates between services (default: 15 seconds)</span>
        </div>

        <div class="form-row form-row-three">
          <div class="form-group">
            <label for="ytop">Top Line Position (No Station Name)</label>
            <input type="number" id="ytop" name="ytop" value="{YTOP}" min="0" max="64" required>
            <span class="help-text">Default 12. Position when station name is hidden.</span>
          </div>

          <div class="form-group">
            <label for="y1">First Line Vertical Position</label>
            <input type="number" id="y1" name="y1" value="{Y1}" min="0" max="64" required>
            <span class="help-text">Default 26. Higher values move the first line lower.</span>
          </div>

          <div class="form-group">
            <label for="y2">Second Line Vertical Position</label>
            <input type="number" id="y2" name="y2" value="{Y2}" min="0" max="64" required>
            <span class="help-text">Default 38. Keep lower than the bottom line for spacing.</span>
          </div>

          <div class="form-group">
            <label for="y3">Bottom Line Vertical Position</label>
            <input type="number" id="y3" name="y3" value="{Y3}" min="0" max="64" required>
            <span class="help-text">Default 50. Controls the alternating services baseline.</span>
          </div>
        </div>
      </div>

      <div class="card">
        <div class="button-group">
          <button type="button" class="btn btn-danger" onclick="showResetModal()">
            Factory Reset
          </button>
        </div>
        <p style="margin-top: 15px; padding: 12px; background: #e7f3ff; border-radius: 8px; font-size: 13px; color: #004085;">
          <strong>ℹ️ Note:</strong> All settings on this page apply automatically when changed. Only WiFi changes (in Network Settings) require a restart.
        </p>
      </div>

      <!-- Live Display Preview -->
      <div class="card" style="margin-top: 20px;">
        <h2>🖥️ Live Display Preview</h2>
        <div id="livePreview" class="display-frame">
          <div class="display-waiting">Connecting to device...</div>
        </div>
        <div class="ws-status">
          <span><span class="ws-indicator" id="wsIndicator"></span> <span id="wsStatusText">Disconnected</span></span>
          <span>Last Update: <span id="lastUpdate">Never</span></span>
        </div>
      </div>
        </div>
      </div>

      <!-- Tab 2: Network Settings -->
      <div class="tab-content" id="tab-1">
        <form method="POST" action="/save" id="networkForm">
          <input type="hidden" name="station" id="station-hidden" value="{STATION}">
          <input type="hidden" name="interval" id="interval-hidden" value="{INTERVAL}">
          <input type="hidden" name="mode" id="mode-hidden" value="">
          <input type="hidden" name="extra" id="extra-hidden" value="">
          <input type="hidden" name="scrollspeed" id="scrollspeed-hidden" value="{SCROLL}">

          <div class="card">
            <h2>📡 WiFi Configuration</h2>
            
            <div class="form-group">
              <label for="ssid2">WiFi Network (SSID)</label>
              <input type="text" id="ssid2" name="ssid" value="{SSID}" placeholder="Enter WiFi network name" required>
              <span class="help-text">Select from available networks below or enter manually</span>
            </div>

            <button type="button" class="btn btn-outline" onclick="scanNetworks2()">
              <span class="spinner"></span>
              <span class="btn-text">📡 Scan for Networks</span>
            </button>
            <div id="networkList2" class="network-list"></div>

            <div class="form-group" style="margin-top: 20px;">
              <label for="password2">WiFi Password</label>
              <input type="password" id="password2" name="password" placeholder="Enter WiFi password (leave blank to keep current)">
              <span class="help-text">Leave blank if password has not changed</span>
            </div>
          </div>

          <div class="card">
            <div class="button-group">
              <button type="submit" class="btn btn-secondary" id="saveBtn">
                <span class="spinner"></span>
                <span class="btn-text">Save & Restart</span>
              </button>
            </div>
            <p style="margin-top: 15px; padding: 12px; background: #fff3cd; border-radius: 8px; font-size: 13px; color: #856404;">
              <strong>⚠️ Note:</strong> Changing WiFi settings requires a device restart. The device will be unavailable for 10-15 seconds.
            </p>
          </div>
        </form>
      </div>
    </div>

  <!-- Reset Confirmation Modal -->
  <div id="resetModal" class="modal">
    <div class="modal-content">
      <div class="modal-header">
        <h3>WARNING: Confirm Factory Reset</h3>
        <p>This will erase all settings and restart the device in setup mode.</p>
      </div>
      <p style="color: #dc3545; font-weight: 600;">This action cannot be undone!</p>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" onclick="hideResetModal()">Cancel</button>
        <button type="button" class="btn btn-danger" onclick="confirmReset()">Reset Device</button>
      </div>
    </div>
  </div>

  <script>
    let ws = null;
    let reconnectTimer = null;
    let previewUpdateTimer = null;
    let stationData = [];
    let stationDataLoaded = false;

    // Load UK Railway Stations Data
    function loadStationData() {
      // Fallback data with major UK stations
      const fallbackStations = [
        {name: "London Paddington", code: "PAD"}, {name: "London Victoria", code: "VIC"},
        {name: "London Waterloo", code: "WAT"}, {name: "London Kings Cross", code: "KGX"},
        {name: "London Euston", code: "EUS"}, {name: "London Liverpool Street", code: "LST"},
        {name: "London St Pancras International", code: "STP"}, {name: "London Bridge", code: "LBG"},
        {name: "London Marylebone", code: "MYB"}, {name: "London Charing Cross", code: "CHX"},
        {name: "London Cannon Street", code: "CST"}, {name: "London Fenchurch Street", code: "FST"},
        {name: "Birmingham New Street", code: "BHM"}, {name: "Manchester Piccadilly", code: "MAN"},
        {name: "Edinburgh Waverley", code: "EDB"}, {name: "Glasgow Central", code: "GLC"},
        {name: "Leeds", code: "LDS"}, {name: "Liverpool Lime Street", code: "LIV"},
        {name: "Bristol Temple Meads", code: "BRI"}, {name: "Cardiff Central", code: "CDF"},
        {name: "Newcastle", code: "NCL"}, {name: "Sheffield", code: "SHF"},
        {name: "Nottingham", code: "NOT"}, {name: "Reading", code: "RDG"},
        {name: "Brighton", code: "BTN"}, {name: "Southampton Central", code: "SOU"},
        {name: "Oxford", code: "OXF"}, {name: "Cambridge", code: "CBG"},
        {name: "York", code: "YRK"}, {name: "Bath Spa", code: "BTH"},
        {name: "Exeter St Davids", code: "EXD"}, {name: "Plymouth", code: "PLY"},
        {name: "Portsmouth Harbour", code: "PMH"}, {name: "Bournemouth", code: "BMH"},
        {name: "Gatwick Airport", code: "GTW"}, {name: "Stansted Airport", code: "SSD"},
        {name: "Luton Airport Parkway", code: "LTN"}, {name: "Heathrow Terminals 2 & 3", code: "HXX"},
        {name: "Aberdeen", code: "ABD"}, {name: "Inverness", code: "INV"},
        {name: "Swansea", code: "SWA"}, {name: "Norwich", code: "NRW"},
        {name: "Derby", code: "DBY"}, {name: "Leicester", code: "LEI"},
        {name: "Coventry", code: "COV"}, {name: "Durham", code: "DHM"},
        {name: "Chester", code: "CTR"}, {name: "Peterborough", code: "PBO"}
      ];
      
      // Try to load from external source
      const stationsURL = "https://raw.githubusercontent.com/davwheat/uk-railway-stations/main/stations.json";
      
      fetch(stationsURL)
        .then(function(response) { 
          if (!response.ok) throw new Error("API failed");
          return response.json(); 
        })
        .then(function(data) {
          // Transform data into searchable format
          stationData = data.map(function(station) {
            return {
              name: station.stationName || station.name,
              code: (station.crsCode || station.code || "").toUpperCase()
            };
          }).filter(function(station) {
            return station.code && station.code.length === 3;
          });
          stationDataLoaded = true;
          console.log("Loaded " + stationData.length + " stations from API");
        })
        .catch(function(error) {
          console.log("Using fallback station data");
          stationData = fallbackStations;
          stationDataLoaded = true;
        });
    }

    // Station Autocomplete
    function setupStationAutocomplete() {
      const input = document.getElementById("station");
      const results = document.getElementById("stationAutocomplete");
      
      input.addEventListener("input", function(e) {
        const query = e.target.value.toUpperCase().trim();
        
        if (query.length < 2 || !stationDataLoaded) {
          results.classList.remove("show");
          return;
        }
        
        // Filter stations
        const matches = stationData.filter(function(station) {
          return station.name.toUpperCase().includes(query) || 
                 station.code.includes(query);
        }).slice(0, 10); // Limit to 10 results
        
        if (matches.length === 0) {
          results.innerHTML = "<div class=\"autocomplete-no-results\">No stations found</div>";
          results.classList.add("show");
          return;
        }
        
        results.innerHTML = matches.map(function(station) {
          // Escape special characters for HTML and JavaScript
          var escapedName = station.name.replace(/&/g, '&amp;')
                                       .replace(/</g, '&lt;')
                                       .replace(/>/g, '&gt;')
                                       .replace(/"/g, '&quot;');
          var jsEscapedName = station.name.replace(/\\/g, '\\\\')
                                         .replace(/'/g, "\\'")
                                         .replace(/"/g, '\\"');
          
          return "<div class=\"autocomplete-item\" onmousedown=\"selectStationFromAutocomplete('" + 
                 station.code + "', '" + jsEscapedName + "'); return false;\">" +
                 "<span class=\"station-name\">" + escapedName + "</span>" +
                 "<span class=\"station-code\">" + station.code + "</span>" +
                 "</div>";
        }).join("");
        
        results.classList.add("show");
      });
      
      input.addEventListener("focus", function(e) {
        if (e.target.value.length >= 2 && stationDataLoaded) {
          e.target.dispatchEvent(new Event("input"));
        }
      });
      
      // Close autocomplete when clicking outside
      document.addEventListener("click", function(e) {
        if (!input.contains(e.target) && !results.contains(e.target)) {
          results.classList.remove("show");
        }
      });
    }

    function selectStationFromAutocomplete(code, name) {
      const input = document.getElementById("station");
      
      // Set flag to prevent change event from also applying
      if (window.setAutocompleteFlag) {
        window.setAutocompleteFlag();
      }
      
      // Set the value
      input.value = code;
      document.getElementById("stationAutocomplete").classList.remove("show");
      showToast("Selected: " + name + " (" + code + ")", "success");
      
      // Trigger validation
      input.classList.add("success");
      input.classList.remove("error");
      
      // Blur the input to close mobile keyboards
      input.blur();
      
      // Directly apply with the correct code
      autoApplySettings(code);
    }

    // Tab Switching
    function switchTab(index) {
      const buttons = document.querySelectorAll(".tab-button");
      const contents = document.querySelectorAll(".tab-content");
      
      buttons.forEach(function(btn, i) {
        if (i === index) {
          btn.classList.add("active");
        } else {
          btn.classList.remove("active");
        }
      });
      
      contents.forEach(function(content, i) {
        if (i === index) {
          content.classList.add("active");
        } else {
          content.classList.remove("active");
        }
      });
    }

    // WebSocket Connection
    function connectWebSocket() {
      const wsUrl = "ws://" + window.location.hostname + ":81";
      console.log("Connecting to WebSocket:", wsUrl);
      
      ws = new WebSocket(wsUrl);
      
      ws.onopen = function() {
        console.log("WebSocket connected");
        updateWSStatus(true);
        ws.send(JSON.stringify({command: "getState"}));
      };
      
      ws.onmessage = function(event) {
        try {
          const data = JSON.parse(event.data);
          
          if (data.type === "display_snapshot") {
            updateDisplayPreview(data);
          } else if (data.type === "status") {
            showToast(data.message, data.level);
          } else if (data.type === "train_update") {
            // Update current station in status bar
            if (data.station) {
              const stationEl = document.getElementById("currentStation");
              if (stationEl) {
                stationEl.textContent = data.station;
              }
            }
          } else if (data.type === "metrics") {
            // Update RSSI
            if (data.rssi) {
              const rssi = data.rssi;
              let signal = "Weak";
              if (rssi > -50) signal = "Excellent";
              else if (rssi > -60) signal = "Good";
              else if (rssi > -70) signal = "Fair";
              
              const rssiValueEl = document.getElementById("rssiValue");
              if (rssiValueEl) {
                rssiValueEl.textContent = rssi;
              }

              const wifiStrengthEl = document.getElementById("wifiStrength");
              if (wifiStrengthEl) {
                wifiStrengthEl.innerHTML = "📶 " + signal + " (" + rssi + " dBm)";
              }
            }
          } else if (data.type === "state") {
            // Update station name from state
            if (data.stationName) {
              const stationEl = document.getElementById("currentStation");
              if (stationEl) {
                stationEl.textContent = data.stationName;
              }
            }
            if (data.rssi) {
              const rssi = data.rssi;
              let signal = "Weak";
              if (rssi > -50) signal = "Excellent";
              else if (rssi > -60) signal = "Good";
              else if (rssi > -70) signal = "Fair";

              const rssiValueEl = document.getElementById("rssiValue");
              if (rssiValueEl) {
                rssiValueEl.textContent = rssi;
              }

              const wifiStrengthEl = document.getElementById("wifiStrength");
              if (wifiStrengthEl) {
                wifiStrengthEl.innerHTML = "📶 " + signal + " (" + rssi + " dBm)";
              }
            }
          }
        } catch (e) {
          console.error("Error parsing WebSocket message:", e);
        }
      };
      
      ws.onerror = function(error) {
        console.error("WebSocket error:", error);
        updateWSStatus(false);
      };
      
      ws.onclose = function() {
        console.log("WebSocket disconnected");
        updateWSStatus(false);
        
        if (reconnectTimer) clearTimeout(reconnectTimer);
        reconnectTimer = setTimeout(connectWebSocket, 3000);
      };
    }

    function updateWSStatus(connected) {
      const indicator = document.getElementById("wsIndicator");
      const statusText = document.getElementById("wsStatusText");

      if (!indicator || !statusText) {
        return;
      }

      if (connected) {
        indicator.classList.add("connected");
        statusText.textContent = "Connected";
        statusText.style.color = "#28a745";
      } else {
        indicator.classList.remove("connected");
        statusText.textContent = "Disconnected";
        statusText.style.color = "#dc3545";
      }
    }

    function updateDisplayPreview(data) {
      const preview = document.getElementById("livePreview");
      const now = new Date();
      document.getElementById("lastUpdate").textContent = now.toLocaleTimeString();
      
      // Get current display settings from the form
      const useCallingAt = document.getElementById("mode").value === "1";
      const showExtraService = document.getElementById("extra").value === "1";
      
      let html = "<div style=\"font-size: 14px; line-height: 1.8;\">";
      
      // Station name (centered)
      html += "<div style=\"text-align: center; font-weight: bold; margin-bottom: 15px; border-bottom: 1px solid #333; padding-bottom: 10px; font-size: 16px;\">";
      html += data.stationName || "Unknown Station";
      html += "</div>";
      
      // Services
      if (data.serviceCount > 0 && data.services && data.services.length > 0) {
        // Show first service
        if (data.services[0]) {
          html += "<div style=\"display: flex; justify-content: space-between; margin-bottom: 10px; padding: 10px; background: #111; border-radius: 4px;\">";
          html += "<span><strong>1st</strong> " + data.services[0].std + " " + data.services[0].destination + "</span>";
          html += "<span style=\"color: #ffa500; font-weight: bold;\">" + formatETD(data.services[0].etd) + "</span>";
          html += "</div>";
        }
        
        // Calling points if in calling at mode
        if (useCallingAt && data.callingPoints) {
          html += "<div style=\"margin-top: 5px; margin-bottom: 15px; padding: 12px; background: #111; border-radius: 4px; font-size: 11px; color: #ccc;\">";
          html += "<strong style=\"color: #fff;\">Calling at:</strong> " + data.callingPoints;
          html += "</div>";
        }
        
        // Show second service if not in calling at mode
        if (!useCallingAt && data.services[1]) {
          html += "<div style=\"display: flex; justify-content: space-between; margin-bottom: 10px; padding: 10px; background: #111; border-radius: 4px;\">";
          html += "<span><strong>2nd</strong> " + data.services[1].std + " " + data.services[1].destination + "</span>";
          html += "<span style=\"color: #ffa500; font-weight: bold;\">" + formatETD(data.services[1].etd) + "</span>";
          html += "</div>";
        }
        
        // Alternating service line (3rd/4th/5th depending on mode)
        const startIdx = useCallingAt ? 1 : 2;
        const maxIdx = useCallingAt ? (showExtraService ? 4 : 3) : (showExtraService ? 6 : 4);
        
        if (data.services.length > startIdx) {
          // Show current alternating service
          const currentIdx = data.alternatingService || startIdx;
          if (data.services[currentIdx]) {
            const service = data.services[currentIdx];
            const labels = ["1st", "2nd", "3rd", "4th", "5th", "6th"];
            
            html += "<div style=\"margin-top: 10px; padding: 10px; background: #0a0a0a; border-radius: 4px; border-left: 3px solid #667eea;\">";
            html += "<div style=\"display: flex; justify-content: space-between;\">";
            html += "<span><strong>" + labels[currentIdx] + "</strong> " + service.std + " " + service.destination + "</span>";
            html += "<span style=\"color: #ffa500; font-weight: bold;\">" + formatETD(service.etd) + "</span>";
            html += "</div>";
            
            // Show rotation info
            const numRotating = Math.min(data.services.length - startIdx, maxIdx - startIdx);
            if (numRotating > 1) {
              html += "<div style=\"margin-top: 5px; font-size: 10px; color: #888; font-style: italic;\">↻ Rotates with " + (numRotating - 1) + " more</div>";
            }
            html += "</div>";
          }
        }
      } else {
        html += "<div style=\"text-align: center; color: #999; padding: 40px 0;\">No services available</div>";
      }
      
      // Time (centered at bottom)
      if (data.time) {
        html += "<div style=\"text-align: center; margin-top: 15px; padding-top: 10px; border-top: 1px solid #333; font-size: 12px; color: #999;\">";
        html += data.time;
        html += "</div>";
      }
      
      html += "</div>";
      preview.innerHTML = html;
      
      // Schedule next update to simulate rotation
      if (previewUpdateTimer) clearTimeout(previewUpdateTimer);
      previewUpdateTimer = setTimeout(function() {
        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send(JSON.stringify({command: "getState"}));
        }
      }, 1000);  // Update every 1 second
    }

    function formatETD(etd) {
      if (etd.includes(":")) {
        return "Exp " + etd;
      }
      return etd;
    }

    // Toast Notifications
    function showToast(message, level) {
      level = level || "info";
      const icons = {
        info: "ℹ️",
        success: "✓",
        warning: "⚠️",
        error: "✕"
      };
      
      const toast = document.createElement("div");
      toast.className = "toast " + level;
      toast.innerHTML = "<span class=\"toast-icon\">" + icons[level] + "</span>" +
                        "<span class=\"toast-message\">" + message + "</span>";
      
      document.body.appendChild(toast);
      
      setTimeout(function() {
        toast.style.animation = "slideInRight 0.3s ease reverse";
        setTimeout(function() { toast.remove(); }, 300);
      }, 3000);
    }

    // Network Scanning (for Network Settings tab)
    function scanNetworks2() {
      const btn = event.target;
      btn.disabled = true;
      btn.classList.add("loading");
      
      fetch("/scan")
        .then(function(response) { return response.json(); })
        .then(function(data) {
          displayNetworks2(data.networks);
          showToast("Found " + data.networks.length + " networks", "success");
        })
        .catch(function(error) {
          console.error("Scan error:", error);
          showToast("Failed to scan networks", "error");
        })
        .finally(function() {
          btn.disabled = false;
          btn.classList.remove("loading");
        });
    }

    function displayNetworks2(networks) {
      const list = document.getElementById("networkList2");
      
      if (networks.length === 0) {
        list.innerHTML = "<div style=\"padding: 20px; text-align: center; color: #666;\">No networks found</div>";
        return;
      }

      list.innerHTML = networks.map(function(network) {
        const strength = network.rssi > -50 ? "***" : network.rssi > -70 ? "**" : "*";
        return "<div class=\"network-item\" onclick=\"selectNetwork2('" + network.ssid + "')\">" +
          "<span class=\"network-name\">" + network.ssid + "</span>" +
          "<span class=\"network-signal\">" + strength + " " + network.rssi + " dBm</span>" +
          "</div>";
      }).join("");
    }

    function selectNetwork2(ssid) {
      document.getElementById("ssid2").value = ssid;
      document.getElementById("password2").focus();
      showToast("Selected: " + ssid, "info");
    }

    // Station Presets
    function setStation(code) {
      const input = document.getElementById("station");
      
      // Set flag to prevent change event from also applying
      if (window.setAutocompleteFlag) {
        window.setAutocompleteFlag();
      }
      
      input.value = code;
      showToast("Station set to: " + code, "success");
      
      // Blur the input
      input.blur();
      
      // Pass the code directly to avoid reading from DOM
      autoApplySettings(code);
    }

    // Form Validation
    document.getElementById("station").addEventListener("input", function(e) {
      const val = e.target.value.trim();
      
      // Valid if it's exactly 3 characters (station code) or selected from autocomplete
      if (val.length === 3 && val.toUpperCase() === val) {
        e.target.classList.add("success");
        e.target.classList.remove("error");
      } else if (val.length > 0 && val.length <= 50) {
        // Typing station name or code - neutral state
        e.target.classList.remove("success");
        e.target.classList.remove("error");
      } else {
        e.target.classList.remove("success");
      }
    });

    document.getElementById("interval").addEventListener("input", function(e) {
      const val = parseInt(e.target.value);
      if (val >= 30 && val <= 600) {
        e.target.classList.add("success");
        e.target.classList.remove("error");
      } else {
        e.target.classList.add("error");
        e.target.classList.remove("success");
      }
    });

    // Update preview when display settings change
    document.getElementById("mode").addEventListener("change", function() {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({command: "getState"}));
      }
    });

    document.getElementById("extra").addEventListener("change", function() {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({command: "getState"}));
      }
    });

    // Form Submission
    document.getElementById("configForm").addEventListener("submit", function(e) {
      e.preventDefault();
      const btn = document.getElementById("applyBtn");
      btn.disabled = true;
      btn.classList.add("loading");
      
      // Submit the form
      const form = e.target;
      const formData = new FormData(form);
      
      fetch(form.action, {
        method: "POST",
        body: new URLSearchParams(formData)
      })
      .then(function(response) {
        if (!response.ok) {
          // Validation failed - extract error message
          return response.text().then(function(errorMsg) {
            throw new Error(errorMsg || "Validation failed");
          });
        }
        return response;
      })
      .then(function() {
        showToast("Settings applied successfully!", "success");
        btn.disabled = false;
        btn.classList.remove("loading");

        // Immediately request updated state
        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send(JSON.stringify({command: "getState"}));
        }
      })
      .catch(function(error) {
        showToast(error.message || "Failed to apply settings", "error");
        btn.disabled = false;
        btn.classList.remove("loading");
      });
    });

    // Network form submission
    document.getElementById("networkForm").addEventListener("submit", function(e) {
      e.preventDefault();

      if (!confirm("Save WiFi settings and restart?\n\nThe device will restart and may take 10-15 seconds to reconnect.")) {
        return false;
      }

      // Copy current settings to hidden fields
      document.getElementById("station-hidden").value = document.getElementById("station").value;
      document.getElementById("interval-hidden").value = document.getElementById("interval").value;
      document.getElementById("mode-hidden").value = document.getElementById("mode").value;
      document.getElementById("extra-hidden").value = document.getElementById("extra").value;
      document.getElementById("scrollspeed-hidden").value = document.getElementById("scrollspeed").value;

      const btn = document.getElementById("saveBtn");
      btn.disabled = true;
      btn.classList.add("loading");

      // Submit via fetch to handle validation errors
      const form = e.target;
      const formData = new FormData(form);

      fetch(form.action, {
        method: "POST",
        body: new URLSearchParams(formData)
      })
      .then(function(response) {
        if (!response.ok) {
          // Validation failed - extract error message
          return response.text().then(function(errorMsg) {
            btn.disabled = false;
            btn.classList.remove("loading");
            throw new Error(errorMsg || "Validation failed");
          });
        }
        // Success - device will restart, show message
        showToast("Settings saved! Device restarting...", "success");
      })
      .catch(function(error) {
        showToast(error.message || "Failed to save settings", "error");
        btn.disabled = false;
        btn.classList.remove("loading");
      });
    });

    // Reset Modal
    function showResetModal() {
      document.getElementById("resetModal").classList.add("show");
    }

    function hideResetModal() {
      document.getElementById("resetModal").classList.remove("show");
    }

    function confirmReset() {
      hideResetModal();
      showToast("Resetting device...", "warning");
      
      setTimeout(function() {
        window.location.href = "/reset";
      }, 1000);
    }

    // Initialize on page load
    document.addEventListener("DOMContentLoaded", function() {
      connectWebSocket();
      loadStationData();
      setupStationAutocomplete();
      
      // Update current station display
      const stationInput = document.getElementById("station");
      if (stationInput.value) {
        document.getElementById("currentStation").textContent = stationInput.value;
      }
      
      // Flag to prevent double-applying from autocomplete
      let autocompleteJustSelected = false;
      
      // Make the flag accessible to autocomplete function
      window.setAutocompleteFlag = function() {
        autocompleteJustSelected = true;
        setTimeout(function() {
          autocompleteJustSelected = false;
        }, 100);
      };
      
      // Auto-apply for all settings except WiFi (WiFi requires restart)
      const autoApplyFields = ["station", "interval", "mode", "showstation", "extra", "rotationspeed", "scrollspeed", "ytop", "y1", "y2", "y3"];
      autoApplyFields.forEach(function(fieldId) {
        const field = document.getElementById(fieldId);
        if (field) {
          // For station field, apply on change (like others) AND on Enter key
          if (fieldId === "station") {
            // Apply when user presses Enter
            field.addEventListener("keypress", function(e) {
              if (e.key === "Enter" && field.value.length >= 3) {
                e.preventDefault();
                autoApplySettings();
              }
            });
            // Also apply on change (when field loses focus or value changes)
            field.addEventListener("change", function() {
              // Don't auto-apply if autocomplete was just used
              if (!autocompleteJustSelected && field.value.length >= 3) {
                autoApplySettings();
              }
            });
          } else {
            // For other fields, apply on change
            field.addEventListener("change", function() {
              autoApplySettings();
            });
          }
        }
      });
    });
    
    // Auto-apply settings without restart
    function autoApplySettings(stationCodeOverride) {
      // Manually collect form data from the configForm div
      const formData = new URLSearchParams();
      // Use override if provided, otherwise read from field
      const stationValue = stationCodeOverride || document.getElementById('station').value;
      formData.append('station', stationValue);
      formData.append('interval', document.getElementById('interval').value);
      formData.append('mode', document.getElementById('mode').value);
      formData.append('showstation', document.getElementById('showstation').value);
      formData.append('extra', document.getElementById('extra').value);
      formData.append('scrollspeed', document.getElementById('scrollspeed').value);
      formData.append('rotationspeed', document.getElementById('rotationspeed').value);
      const ytopField = document.getElementById('ytop');
      if (ytopField) formData.append('ytop', ytopField.value);
      const y1Field = document.getElementById('y1');
      if (y1Field) formData.append('y1', y1Field.value);
      const y2Field = document.getElementById('y2');
      if (y2Field) formData.append('y2', y2Field.value);
      const y3Field = document.getElementById('y3');
      if (y3Field) formData.append('y3', y3Field.value);
      
      // Show subtle notification
      showToast("Applying changes...", "info");
      
      fetch("/apply", {
        method: "POST",
        body: formData
      })
      .then(function() {
        showToast("Settings updated!", "success");
        
        // Request updated state
        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send(JSON.stringify({command: "getState"}));
        }
      })
      .catch(function(error) {
        showToast("Failed to apply settings", "error");
      });
    }
  </script>
</body>
</html>
)HTMLCODE";

const char SAVE_SUCCESS_PAGE[] PROGMEM = R"HTMLCODE(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Saving Settings...</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    
    .card {
      background: white;
      border-radius: 16px;
      padding: 50px;
      text-align: center;
      box-shadow: 0 10px 40px rgba(0,0,0,0.2);
      max-width: 500px;
      animation: fadeIn 0.5s ease;
    }
    
    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(20px); }
      to { opacity: 1; transform: translateY(0); }
    }
    
    .spinner {
      width: 60px;
      height: 60px;
      border: 6px solid #f3f3f3;
      border-top: 6px solid #667eea;
      border-radius: 50%;
      animation: spin 1s linear infinite;
      margin: 0 auto 30px;
    }
    
    @keyframes spin {
      to { transform: rotate(360deg); }
    }
    
    h1 {
      color: #333;
      font-size: 28px;
      margin-bottom: 15px;
    }
    
    p {
      color: #666;
      font-size: 16px;
      line-height: 1.6;
    }
    
    .success-icon {
      font-size: 60px;
      margin-bottom: 20px;
    }
  </style>
</head>
<body>
  <div class="card">
    <div class="spinner"></div>
    <h1>💾 Saving Configuration</h1>
    <p>Your settings have been saved successfully.</p>
    <p style="margin-top: 10px;">The device is restarting...</p>
    <p style="margin-top: 20px; font-size: 14px; color: #999;">Please wait 10-15 seconds</p>
  </div>
  <script>
    setTimeout(function() {
      window.location.href = "/";
    }, 15000);
  </script>
</body>
</html>
)HTMLCODE";

const char APPLY_SUCCESS_PAGE[] PROGMEM = R"HTMLCODE(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Settings Applied</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    
    .card {
      background: white;
      border-radius: 16px;
      padding: 50px;
      text-align: center;
      box-shadow: 0 10px 40px rgba(0,0,0,0.2);
      max-width: 500px;
      animation: fadeIn 0.5s ease;
    }
    
    @keyframes fadeIn {
      from { opacity: 0; transform: scale(0.9); }
      to { opacity: 1; transform: scale(1); }
    }
    
    .success-icon {
      font-size: 80px;
      margin-bottom: 20px;
      animation: bounce 0.6s ease;
    }
    
    @keyframes bounce {
      0%, 100% { transform: translateY(0); }
      50% { transform: translateY(-20px); }
    }
    
    h1 {
      color: #28a745;
      font-size: 28px;
      margin-bottom: 15px;
    }
    
    p {
      color: #666;
      font-size: 16px;
      line-height: 1.6;
      margin-bottom: 10px;
    }
    
    .btn {
      display: inline-block;
      margin-top: 30px;
      padding: 14px 28px;
      background: #667eea;
      color: white;
      text-decoration: none;
      border-radius: 8px;
      font-weight: 600;
      transition: all 0.3s ease;
    }
    
    .btn:hover {
      background: #5568d3;
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
    }
  </style>
</head>
<body>
  <div class="card">
    <div class="success-icon">✓</div>
    <h1>Settings Applied!</h1>
    <p>Your changes have been applied successfully.</p>
    <p>The device is updating without restarting.</p>
    <a href="/" class="btn">← Back to Dashboard</a>
  </div>
  <script>
    setTimeout(function() {
      window.location.href = "/";
    }, 3000);
  </script>
</body>
</html>
)HTMLCODE";

const char RESET_SUCCESS_PAGE[] PROGMEM = R"HTMLCODE(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Factory Reset</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    
    .card {
      background: white;
      border-radius: 16px;
      padding: 50px;
      text-align: center;
      box-shadow: 0 10px 40px rgba(0,0,0,0.2);
      max-width: 500px;
      animation: fadeIn 0.5s ease;
    }
    
    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(20px); }
      to { opacity: 1; transform: translateY(0); }
    }
    
    .spinner {
      width: 60px;
      height: 60px;
      border: 6px solid #f3f3f3;
      border-top: 6px solid #dc3545;
      border-radius: 50%;
      animation: spin 1s linear infinite;
      margin: 0 auto 30px;
    }
    
    @keyframes spin {
      to { transform: rotate(360deg); }
    }
    
    h1 {
      color: #dc3545;
      font-size: 28px;
      margin-bottom: 15px;
    }
    
    p {
      color: #666;
      font-size: 16px;
      line-height: 1.6;
    }
  </style>
</head>
<body>
  <div class="card">
    <div class="spinner"></div>
    <h1>🔄 Factory Reset</h1>
    <p>All settings have been erased.</p>
    <p style="margin-top: 10px;">The device is restarting in setup mode...</p>
    <p style="margin-top: 20px; font-size: 14px; color: #999;">Connect to "TrainBoard_AP" WiFi network</p>
  </div>
</body>
</html>
)HTMLCODE";

#endif