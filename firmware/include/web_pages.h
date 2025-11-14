#ifndef WEB_PAGES_H
#define WEB_PAGES_H

const char CONFIG_PAGE_TEMPLATE[] PROGMEM = R"HTMLCODE(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="description" content="StationBoards.co.uk - Live Train Departure Board Configuration">
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

    /* Focus visible for accessibility */
    *:focus-visible {
      outline: 2px solid #667eea;
      outline-offset: 2px;
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
        <div style="margin-top: 8px; font-size: 12px; color: #666;" id="wifiStrength" aria-live="polite">
          📶 <span id="rssiValue">--</span> dBm
        </div>
      </div>
      <div class="status-item">
        <div class="label">Current Station</div>
        <div class="value" id="currentStation" aria-live="polite">{STATION_NAME}</div>
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
      <div class="tab-buttons" role="tablist" aria-label="Configuration sections">
        <button class="tab-button active" role="tab" aria-selected="true" aria-controls="tab-0" id="tab-btn-0" data-tab="0">Station Board</button>
        <button class="tab-button" role="tab" aria-selected="false" aria-controls="tab-1" id="tab-btn-1" data-tab="1">Network Settings</button>
      </div>

      <!-- Tab 1: Station Board Configuration -->
      <div class="tab-content active" id="tab-0" role="tabpanel" aria-labelledby="tab-btn-0">
        <div id="configForm">
      <div class="card">
        <h2>🚉 Station Configuration</h2>

        <div class="form-group">
          <label for="serviceType">Transport Service</label>
          <select id="serviceType" name="serviceType" aria-describedby="servicetype-help">
            <option value="0"{SERVICE_SEL_0}>National Rail</option>
            <option value="1"{SERVICE_SEL_1}>TFL Underground</option>
          </select>
          <span class="help-text" id="servicetype-help">Select which transport service to display</span>
        </div>

        <div class="form-group" id="tflApiKeyGroup" style="display:none;">
          <label for="tflApiKey">
            TFL API Key
            <span class="info-tooltip" title="Optional but recommended. Get from https://api.tfl.gov.uk" aria-label="Information: TFL API key">?</span>
          </label>
          <input type="password" id="tflApiKey" name="tflApiKey" value="{TFL_API_KEY}" placeholder="Enter TFL API key" aria-describedby="tflkey-help">
          <span class="help-text" id="tflkey-help">Free API key from <a href="https://api.tfl.gov.uk" target="_blank">api.tfl.gov.uk</a></span>
        </div>

        <div class="form-group">
          <label for="station">
            <span id="stationLabel">Station Code (CRS)</span>
            <span class="info-tooltip" id="stationTooltip" title="Three-letter National Rail station code" aria-label="Information: Three-letter National Rail station code">?</span>
          </label>
          <div class="preset-stations" id="railPresets">
            <button type="button" class="preset-btn" data-station="PAD" aria-label="Select Paddington station">PAD<br><small>Paddington</small></button>
            <button type="button" class="preset-btn" data-station="VIC" aria-label="Select Victoria station">VIC<br><small>Victoria</small></button>
            <button type="button" class="preset-btn" data-station="WAT" aria-label="Select Waterloo station">WAT<br><small>Waterloo</small></button>
            <button type="button" class="preset-btn" data-station="KGX" aria-label="Select Kings Cross station">KGX<br><small>Kings Cross</small></button>
            <button type="button" class="preset-btn" data-station="EUS" aria-label="Select Euston station">EUS<br><small>Euston</small></button>
            <button type="button" class="preset-btn" data-station="LST" aria-label="Select Liverpool Street station">LST<br><small>Liverpool St</small></button>
          </div>
          <div class="preset-stations" id="tflPresets" style="display:none;">
            <button type="button" class="preset-btn" data-station="940GZZLUPAC" aria-label="Select Paddington Underground">PAC<br><small>Paddington</small></button>
            <button type="button" class="preset-btn" data-station="940GZZLUOXC" aria-label="Select Oxford Circus">OXC<br><small>Oxford Circus</small></button>
            <button type="button" class="preset-btn" data-station="940GZZLUWLO" aria-label="Select Waterloo">WLO<br><small>Waterloo</small></button>
            <button type="button" class="preset-btn" data-station="940GZZLUKSX" aria-label="Select King's Cross">KSX<br><small>King's Cross</small></button>
            <button type="button" class="preset-btn" data-station="940GZZLUVIC" aria-label="Select Victoria">VIC<br><small>Victoria</small></button>
            <button type="button" class="preset-btn" data-station="940GZZLULNB" aria-label="Select London Bridge">LNB<br><small>London Bridge</small></button>
          </div>
          <div class="autocomplete-wrapper">
            <input type="text" id="station" name="station" value="{STATION}" placeholder="Type station name or code..." required maxlength="50" aria-label="Station code or name" aria-describedby="station-help">
            <div id="stationAutocomplete" class="autocomplete-results" role="listbox" aria-label="Station suggestions"></div>
          </div>
          <span class="help-text" id="station-help">Start typing to search for a station</span>
        </div>

        <div class="form-row">
          <div class="form-group">
            <label for="interval">
              Refresh Interval (seconds)
              <span class="info-tooltip" title="How often to fetch new departure data" aria-label="Information: How often to fetch new departure data">?</span>
            </label>
            <input type="number" id="interval" name="interval" value="{INTERVAL}" min="30" max="600" required aria-describedby="interval-help">
            <span class="help-text" id="interval-help">Recommended: 60-120 seconds</span>
          </div>

          <div class="form-group">
            <label for="scrollspeed">
              Scroll Speed (ms)
              <span class="info-tooltip" title="Lower = faster scrolling" aria-label="Information: Lower = faster scrolling">?</span>
            </label>
            <input type="number" id="scrollspeed" name="scrollspeed" value="{SCROLL}" min="10" max="200" required aria-describedby="scrollspeed-help">
            <span class="help-text" id="scrollspeed-help">Recommended: 50-100ms</span>
          </div>
        </div>
      </div>

      <div class="card">
        <h2>Display Options</h2>

        <div class="form-group">
          <label for="mode">Display Mode</label>
          <select id="mode" name="mode" aria-describedby="mode-help">
            <option value="0"{MODE_SEL_0}>Standard View</option>
            <option value="1"{MODE_SEL_1}>Calling At Mode (shows stops)</option>
          </select>
          <span class="help-text" id="mode-help">Calling At mode shows detailed stops for the first train</span>
        </div>

        <div class="form-group">
          <label for="showstation">Show Station Name at Top</label>
          <select id="showstation" name="showstation" aria-describedby="showstation-help">
            <option value="1"{SHOWSTATION_SEL_1}>Show Station Name</option>
            <option value="0"{SHOWSTATION_SEL_0}>Hide Station Name (adds extra service line)</option>
          </select>
          <span class="help-text" id="showstation-help">Hiding the station name adds an extra service at the top for more trains</span>
        </div>

        <div class="form-group">
          <label for="extra">Extra Services on Bottom Line</label>
          <select id="extra" name="extra" aria-describedby="extra-help">
            <option value="0"{EXTRA_SEL_0}>No Extra Services</option>
            <option value="1"{EXTRA_SEL_1}>1 Extra Service</option>
            <option value="2"{EXTRA_SEL_2}>2 Extra Services</option>
            <option value="3"{EXTRA_SEL_3}>3 Extra Services</option>
            <option value="4"{EXTRA_SEL_4}>4 Extra Services</option>
          </select>
          <span class="help-text" id="extra-help">Number of additional services that rotate on the bottom line</span>
        </div>

        <div class="form-group">
          <label for="rotationspeed">Bottom Line Rotation Speed (seconds)</label>
          <input type="number" id="rotationspeed" name="rotationspeed" value="{ROTATION}" min="5" max="60" required aria-describedby="rotationspeed-help">
          <span class="help-text" id="rotationspeed-help">How often the bottom line alternates between services (default: 15 seconds)</span>
        </div>

        <div class="form-row form-row-three">
          <div class="form-group">
            <label for="ytop">Top Line Position (No Station Name)</label>
            <input type="number" id="ytop" name="ytop" value="{YTOP}" min="0" max="64" required aria-describedby="ytop-help">
            <span class="help-text" id="ytop-help">Default 12. Position when station name is hidden.</span>
          </div>

          <div class="form-group">
            <label for="y1">First Line Vertical Position</label>
            <input type="number" id="y1" name="y1" value="{Y1}" min="0" max="64" required aria-describedby="y1-help">
            <span class="help-text" id="y1-help">Default 26. Higher values move the first line lower.</span>
          </div>

          <div class="form-group">
            <label for="y2">Second Line Vertical Position</label>
            <input type="number" id="y2" name="y2" value="{Y2}" min="0" max="64" required aria-describedby="y2-help">
            <span class="help-text" id="y2-help">Default 38. Keep lower than the bottom line for spacing.</span>
          </div>

          <div class="form-group">
            <label for="y3">Bottom Line Vertical Position</label>
            <input type="number" id="y3" name="y3" value="{Y3}" min="0" max="64" required aria-describedby="y3-help">
            <span class="help-text" id="y3-help">Default 50. Controls the alternating services baseline.</span>
          </div>
        </div>
      </div>

      <div class="card">
        <div class="button-group">
          <button type="button" class="btn btn-danger" id="resetButton" aria-label="Factory reset device">
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
        <div id="livePreview" class="display-frame" role="region" aria-live="polite" aria-label="Live display preview">
          <div class="display-waiting">Connecting to device...</div>
        </div>
        <div class="ws-status">
          <span><span class="ws-indicator" id="wsIndicator" aria-hidden="true"></span> <span id="wsStatusText">Disconnected</span></span>
          <span>Last Update: <span id="lastUpdate">Never</span></span>
        </div>
      </div>
        </div>
      </div>

      <!-- Tab 2: Network Settings -->
      <div class="tab-content" id="tab-1" role="tabpanel" aria-labelledby="tab-btn-1">
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
              <input type="text" id="ssid2" name="ssid" value="{SSID}" placeholder="Enter WiFi network name" required aria-describedby="ssid-help">
              <span class="help-text" id="ssid-help">Select from available networks below or enter manually</span>
            </div>

            <button type="button" class="btn btn-outline" id="scanNetworksBtn" aria-label="Scan for WiFi networks">
              <span class="spinner" aria-hidden="true"></span>
              <span class="btn-text">📡 Scan for Networks</span>
            </button>
            <div id="networkList2" class="network-list" role="list" aria-label="Available WiFi networks"></div>

            <div class="form-group" style="margin-top: 20px;">
              <label for="password2">WiFi Password</label>
              <input type="password" id="password2" name="password" placeholder="Enter WiFi password (leave blank to keep current)" aria-describedby="password-help">
              <span class="help-text" id="password-help">Leave blank if password has not changed</span>
            </div>
          </div>

          <div class="card">
            <div class="button-group">
              <button type="submit" class="btn btn-secondary" id="saveBtn" aria-label="Save WiFi settings and restart device">
                <span class="spinner" aria-hidden="true"></span>
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
  <div id="resetModal" class="modal" role="dialog" aria-labelledby="resetModalTitle" aria-modal="true">
    <div class="modal-content">
      <div class="modal-header">
        <h3 id="resetModalTitle">WARNING: Confirm Factory Reset</h3>
        <p>This will erase all settings and restart the device in setup mode.</p>
      </div>
      <p style="color: #dc3545; font-weight: 600;">This action cannot be undone!</p>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" id="cancelResetBtn" aria-label="Cancel factory reset">Cancel</button>
        <button type="button" class="btn btn-danger" id="confirmResetBtn" aria-label="Confirm factory reset">Reset Device</button>
      </div>
    </div>
  </div>

  <script>
    // ==================== Security & Utilities ====================

    /**
     * Escape HTML to prevent XSS attacks
     */
    const escapeHtml = (unsafe) => {
      if (unsafe === null || unsafe === undefined) return '';
      return String(unsafe)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
    };

    /**
     * Debounce function to limit execution frequency
     */
    const debounce = (func, wait) => {
      let timeout;
      return function executedFunction(...args) {
        const later = () => {
          clearTimeout(timeout);
          func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
      };
    };

    /**
     * Fetch with timeout
     */
    const fetchWithTimeout = (url, options = {}, timeout = 10000) => {
      return Promise.race([
        fetch(url, options),
        new Promise((_, reject) =>
          setTimeout(() => reject(new Error('Request timeout')), timeout)
        )
      ]);
    };

    // ==================== Application State ====================

    let ws = null;
    let reconnectTimer = null;
    let previewUpdateTimer = null;
    let railStationData = [];
    let tflStationData = [];
    let stationDataLoaded = false;
    let autocompleteJustSelected = false;

    // ==================== Station Data Loading ====================

    const loadStationData = () => {
      const fallbackRailStations = [
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

      // TFL Underground stations with NaPTAN IDs
      const tflUndergroundStations = [
        // Central Line
        {name: "Bank", code: "940GZZLUBNK", line: "Central"}, {name: "Bond Street", code: "940GZZLUBND", line: "Central"},
        {name: "Chancery Lane", code: "940GZZLUCHL", line: "Central"}, {name: "Ealing Broadway", code: "940GZZLUEBN", line: "Central"},
        {name: "Epping", code: "940GZZLUEPG", line: "Central"}, {name: "Holborn", code: "940GZZLUHBN", line: "Central"},
        {name: "Liverpool Street", code: "940GZZLULVT", line: "Central"}, {name: "Marble Arch", code: "940GZZLUMAR", line: "Central"},
        {name: "Mile End", code: "940GZZLUMND", line: "Central"}, {name: "Notting Hill Gate", code: "940GZZLUNHG", line: "Central"},
        {name: "Oxford Circus", code: "940GZZLUOXC", line: "Central"}, {name: "Tottenham Court Road", code: "940GZZLUTCR", line: "Central"},

        // Piccadilly Line
        {name: "Piccadilly Circus", code: "940GZZLUPCC", line: "Piccadilly"}, {name: "Leicester Square", code: "940GZZLULSQ", line: "Piccadilly"},
        {name: "Covent Garden", code: "940GZZLUCGN", line: "Piccadilly"}, {name: "Knightsbridge", code: "940GZZLUKNB", line: "Piccadilly"},
        {name: "South Kensington", code: "940GZZLUSKS", line: "Piccadilly"}, {name: "Heathrow Terminal 5", code: "940GZZLUHR5", line: "Piccadilly"},
        {name: "Heathrow Terminals 2 & 3", code: "940GZZLUHR3", line: "Piccadilly"},

        // Northern Line
        {name: "King's Cross St. Pancras", code: "940GZZLUKSX", line: "Northern"}, {name: "Camden Town", code: "940GZZLUCTN", line: "Northern"},
        {name: "Euston", code: "940GZZLUEUS", line: "Northern"}, {name: "Old Street", code: "940GZZLUOST", line: "Northern"},
        {name: "Moorgate", code: "940GZZLUMGT", line: "Northern"}, {name: "Angel", code: "940GZZLUAGL", line: "Northern"},
        {name: "Waterloo", code: "940GZZLUWLO", line: "Northern"}, {name: "Embankment", code: "940GZZLUEMB", line: "Northern"},

        // Victoria Line
        {name: "Victoria", code: "940GZZLUVIC", line: "Victoria"}, {name: "Green Park", code: "940GZZLUGPK", line: "Victoria"},
        {name: "Warren Street", code: "940GZZLUWRR", line: "Victoria"}, {name: "Highbury & Islington", code: "940GZZLUHSB", line: "Victoria"},

        // Circle, District, H&C Line
        {name: "Paddington", code: "940GZZLUPAC", line: "Circle/H&C"}, {name: "Westminster", code: "940GZZLUWSM", line: "Circle/District"},
        {name: "Tower Hill", code: "940GZZLUTWH", line: "Circle/District"}, {name: "Gloucester Road", code: "940GZZLUGTR", line: "Circle/District"},
        {name: "Sloane Square", code: "940GZZLUSSP", line: "Circle/District"}, {name: "Earl's Court", code: "940GZZLUECT", line: "District"},

        // Jubilee Line
        {name: "London Bridge", code: "940GZZLULNB", line: "Jubilee"}, {name: "Canary Wharf", code: "940GZZLUCYF", line: "Jubilee"},
        {name: "Stratford", code: "940GZZLUSTD", line: "Jubilee"}, {name: "Baker Street", code: "940GZZLUBST", line: "Jubilee"},

        // Bakerloo Line
        {name: "Charing Cross", code: "940GZZLUCHX", line: "Bakerloo"}, {name: "Regent's Park", code: "940GZZLURGP", line: "Bakerloo"},

        // Metropolitan Line
        {name: "Aldgate", code: "940GZZLUALD", line: "Metropolitan"}, {name: "Harrow-on-the-Hill", code: "940GZZLUHAR", line: "Metropolitan"}
      ];

      const stationsURL = "https://raw.githubusercontent.com/davwheat/uk-railway-stations/main/stations.json";

      // Load National Rail stations
      fetchWithTimeout(stationsURL, {}, 10000)
        .then(response => {
          if (!response.ok) throw new Error("API failed");
          return response.json();
        })
        .then(data => {
          railStationData = data.map(station => ({
            name: station.stationName || station.name,
            code: (station.crsCode || station.code || "").toUpperCase()
          })).filter(station => station.code && station.code.length === 3);
          console.log(`Loaded ${railStationData.length} National Rail stations`);
        })
        .catch(error => {
          console.log("Using fallback National Rail data:", error.message);
          railStationData = fallbackRailStations;
        });

      // Load TFL stations
      tflStationData = tflUndergroundStations;
      stationDataLoaded = true;
      console.log(`Loaded ${tflStationData.length} TFL Underground stations`);
    };

    // ==================== Station Autocomplete ====================

    const setupStationAutocomplete = () => {
      const input = document.getElementById("station");
      const results = document.getElementById("stationAutocomplete");

      // Debounced input handler
      const handleInput = debounce((e) => {
        const query = e.target.value.toUpperCase().trim();

        if (query.length < 2 || !stationDataLoaded) {
          results.classList.remove("show");
          return;
        }

        // Use appropriate dataset based on service type
        const serviceTypeSelect = document.getElementById("serviceType");
        const isUnderground = serviceTypeSelect && serviceTypeSelect.value === "1";
        const currentStationData = isUnderground ? tflStationData : railStationData;

        const matches = currentStationData.filter(station =>
          station.name.toUpperCase().includes(query) ||
          station.code.toUpperCase().includes(query)
        ).slice(0, 10);

        if (matches.length === 0) {
          results.innerHTML = '<div class="autocomplete-no-results">No stations found</div>';
          results.classList.add("show");
          return;
        }

        results.innerHTML = matches.map(station => {
          const escapedName = escapeHtml(station.name);
          const escapedCode = escapeHtml(station.code);
          const lineInfo = station.line ? ` <small>(${escapeHtml(station.line)})</small>` : '';

          return `<div class="autocomplete-item" role="option" data-code="${escapedCode}" data-name="${escapedName}" tabindex="0">
            <span class="station-name">${escapedName}${lineInfo}</span>
            <span class="station-code">${escapedCode}</span>
          </div>`;
        }).join("");

        results.classList.add("show");

        // Add event listeners to autocomplete items
        results.querySelectorAll('.autocomplete-item').forEach(item => {
          item.addEventListener('mousedown', (e) => {
            e.preventDefault();
            selectStationFromAutocomplete(item.dataset.code, item.dataset.name);
          });

          item.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
              e.preventDefault();
              selectStationFromAutocomplete(item.dataset.code, item.dataset.name);
            }
          });
        });
      }, 300); // 300ms debounce

      input.addEventListener("input", handleInput);

      input.addEventListener("focus", (e) => {
        if (e.target.value.length >= 2 && stationDataLoaded) {
          e.target.dispatchEvent(new Event("input"));
        }
      });

      document.addEventListener("click", (e) => {
        if (!input.contains(e.target) && !results.contains(e.target)) {
          results.classList.remove("show");
        }
      });
    };

    const selectStationFromAutocomplete = (code, name) => {
      const input = document.getElementById("station");

      autocompleteJustSelected = true;
      setTimeout(() => { autocompleteJustSelected = false; }, 100);

      input.value = code;
      document.getElementById("stationAutocomplete").classList.remove("show");
      showToast(`Selected: ${escapeHtml(name)} (${escapeHtml(code)})`, "success");

      input.classList.add("success");
      input.classList.remove("error");
      input.blur();

      autoApplySettings(code);
    };

    // ==================== Tab Switching ====================

    const switchTab = (index) => {
      const buttons = document.querySelectorAll(".tab-button");
      const contents = document.querySelectorAll(".tab-content");

      buttons.forEach((btn, i) => {
        const isActive = i === index;
        btn.classList.toggle("active", isActive);
        btn.setAttribute("aria-selected", isActive);
      });

      contents.forEach((content, i) => {
        content.classList.toggle("active", i === index);
      });
    };

    // ==================== WebSocket Connection ====================

    const connectWebSocket = () => {
      const wsUrl = `ws://${window.location.hostname}:81`;
      console.log("Connecting to WebSocket:", wsUrl);

      ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        console.log("WebSocket connected");
        updateWSStatus(true);
        ws.send(JSON.stringify({command: "getState"}));
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);

          if (data.type === "display_snapshot") {
            updateDisplayPreview(data);
          } else if (data.type === "status") {
            showToast(data.message, data.level);
          } else if (data.type === "train_update") {
            if (data.station) {
              const stationEl = document.getElementById("currentStation");
              if (stationEl) {
                stationEl.textContent = escapeHtml(data.station);
              }
            }
          } else if (data.type === "metrics") {
            updateRSSI(data.rssi);
          } else if (data.type === "state") {
            if (data.stationName) {
              const stationEl = document.getElementById("currentStation");
              if (stationEl) {
                stationEl.textContent = escapeHtml(data.stationName);
              }
            }
            updateRSSI(data.rssi);
          }
        } catch (e) {
          console.error("Error parsing WebSocket message:", e);
        }
      };

      ws.onerror = (error) => {
        console.error("WebSocket error:", error);
        updateWSStatus(false);
      };

      ws.onclose = () => {
        console.log("WebSocket disconnected");
        updateWSStatus(false);

        if (reconnectTimer) clearTimeout(reconnectTimer);
        reconnectTimer = setTimeout(connectWebSocket, 3000);
      };
    };

    const updateWSStatus = (connected) => {
      const indicator = document.getElementById("wsIndicator");
      const statusText = document.getElementById("wsStatusText");

      if (!indicator || !statusText) return;

      if (connected) {
        indicator.classList.add("connected");
        statusText.textContent = "Connected";
        statusText.style.color = "#28a745";
      } else {
        indicator.classList.remove("connected");
        statusText.textContent = "Disconnected";
        statusText.style.color = "#dc3545";
      }
    };

    const updateRSSI = (rssi) => {
      if (!rssi) return;

      let signal = "Weak";
      if (rssi > -50) signal = "Excellent";
      else if (rssi > -60) signal = "Good";
      else if (rssi > -70) signal = "Fair";

      const rssiValueEl = document.getElementById("rssiValue");
      if (rssiValueEl) {
        rssiValueEl.textContent = escapeHtml(rssi);
      }

      const wifiStrengthEl = document.getElementById("wifiStrength");
      if (wifiStrengthEl) {
        wifiStrengthEl.textContent = `📶 ${escapeHtml(signal)} (${escapeHtml(rssi)} dBm)`;
      }
    };

    const updateDisplayPreview = (data) => {
      const preview = document.getElementById("livePreview");
      const now = new Date();
      document.getElementById("lastUpdate").textContent = now.toLocaleTimeString();

      const useCallingAt = document.getElementById("mode").value === "1";
      const showExtraService = document.getElementById("extra").value === "1";

      let html = '<div style="font-size: 14px; line-height: 1.8;">';

      html += '<div style="text-align: center; font-weight: bold; margin-bottom: 15px; border-bottom: 1px solid #333; padding-bottom: 10px; font-size: 16px;">';
      html += escapeHtml(data.stationName || "Unknown Station");
      html += '</div>';

      if (data.serviceCount > 0 && data.services && data.services.length > 0) {
        if (data.services[0]) {
          html += '<div style="display: flex; justify-content: space-between; margin-bottom: 10px; padding: 10px; background: #111; border-radius: 4px;">';
          html += `<span><strong>1st</strong> ${escapeHtml(data.services[0].std)} ${escapeHtml(data.services[0].destination)}</span>`;
          html += `<span style="color: #ffa500; font-weight: bold;">${escapeHtml(formatETD(data.services[0].etd))}</span>`;
          html += '</div>';
        }

        if (useCallingAt && data.callingPoints) {
          html += '<div style="margin-top: 5px; margin-bottom: 15px; padding: 12px; background: #111; border-radius: 4px; font-size: 11px; color: #ccc;">';
          html += `<strong style="color: #fff;">Calling at:</strong> ${escapeHtml(data.callingPoints)}`;
          html += '</div>';
        }

        if (!useCallingAt && data.services[1]) {
          html += '<div style="display: flex; justify-content: space-between; margin-bottom: 10px; padding: 10px; background: #111; border-radius: 4px;">';
          html += `<span><strong>2nd</strong> ${escapeHtml(data.services[1].std)} ${escapeHtml(data.services[1].destination)}</span>`;
          html += `<span style="color: #ffa500; font-weight: bold;">${escapeHtml(formatETD(data.services[1].etd))}</span>`;
          html += '</div>';
        }

        const startIdx = useCallingAt ? 1 : 2;
        const maxIdx = useCallingAt ? (showExtraService ? 4 : 3) : (showExtraService ? 6 : 4);

        if (data.services.length > startIdx) {
          const currentIdx = data.alternatingService || startIdx;
          if (data.services[currentIdx]) {
            const service = data.services[currentIdx];
            const labels = ["1st", "2nd", "3rd", "4th", "5th", "6th"];

            html += '<div style="margin-top: 10px; padding: 10px; background: #0a0a0a; border-radius: 4px; border-left: 3px solid #667eea;">';
            html += '<div style="display: flex; justify-content: space-between;">';
            html += `<span><strong>${labels[currentIdx]}</strong> ${escapeHtml(service.std)} ${escapeHtml(service.destination)}</span>`;
            html += `<span style="color: #ffa500; font-weight: bold;">${escapeHtml(formatETD(service.etd))}</span>`;
            html += '</div>';

            const numRotating = Math.min(data.services.length - startIdx, maxIdx - startIdx);
            if (numRotating > 1) {
              html += `<div style="margin-top: 5px; font-size: 10px; color: #888; font-style: italic;">↻ Rotates with ${numRotating - 1} more</div>`;
            }
            html += '</div>';
          }
        }
      } else {
        html += '<div style="text-align: center; color: #999; padding: 40px 0;">No services available</div>';
      }

      if (data.time) {
        html += '<div style="text-align: center; margin-top: 15px; padding-top: 10px; border-top: 1px solid #333; font-size: 12px; color: #999;">';
        html += escapeHtml(data.time);
        html += '</div>';
      }

      html += '</div>';
      preview.innerHTML = html;

      // Reduced polling frequency from 1s to 5s
      if (previewUpdateTimer) clearTimeout(previewUpdateTimer);
      previewUpdateTimer = setTimeout(() => {
        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send(JSON.stringify({command: "getState"}));
        }
      }, 5000);
    };

    const formatETD = (etd) => {
      if (!etd) return '';
      return etd.includes(":") ? `Exp ${etd}` : etd;
    };

    // ==================== Toast Notifications ====================

    const showToast = (message, level = "info") => {
      const icons = {
        info: "ℹ️",
        success: "✓",
        warning: "⚠️",
        error: "✕"
      };

      const toast = document.createElement("div");
      toast.className = `toast ${level}`;
      toast.setAttribute("role", "alert");
      toast.setAttribute("aria-live", "polite");

      const iconSpan = document.createElement("span");
      iconSpan.className = "toast-icon";
      iconSpan.setAttribute("aria-hidden", "true");
      iconSpan.textContent = icons[level];

      const messageSpan = document.createElement("span");
      messageSpan.className = "toast-message";
      messageSpan.textContent = message; // Text content automatically escapes

      toast.appendChild(iconSpan);
      toast.appendChild(messageSpan);

      document.body.appendChild(toast);

      setTimeout(() => {
        toast.style.animation = "slideInRight 0.3s ease reverse";
        setTimeout(() => { toast.remove(); }, 300);
      }, 3000);
    };

    // ==================== Network Scanning ====================

    const scanNetworks = () => {
      const btn = document.getElementById("scanNetworksBtn");
      btn.disabled = true;
      btn.classList.add("loading");

      fetch("/scan")
        .then(response => response.json())
        .then(data => {
          displayNetworks(data.networks);
          showToast(`Found ${data.networks.length} networks`, "success");
        })
        .catch(error => {
          console.error("Scan error:", error);
          showToast("Failed to scan networks", "error");
        })
        .finally(() => {
          btn.disabled = false;
          btn.classList.remove("loading");
        });
    };

    const displayNetworks = (networks) => {
      const list = document.getElementById("networkList2");

      if (networks.length === 0) {
        list.innerHTML = '<div style="padding: 20px; text-align: center; color: #666;">No networks found</div>';
        return;
      }

      list.innerHTML = networks.map(network => {
        const strength = network.rssi > -50 ? "***" : network.rssi > -70 ? "**" : "*";
        const escapedSSID = escapeHtml(network.ssid);
        const escapedRSSI = escapeHtml(network.rssi);

        return `<div class="network-item" role="listitem" data-ssid="${escapedSSID}" tabindex="0">
          <span class="network-name">${escapedSSID}</span>
          <span class="network-signal">${strength} ${escapedRSSI} dBm</span>
        </div>`;
      }).join("");

      // Add event listeners
      list.querySelectorAll('.network-item').forEach(item => {
        item.addEventListener('click', () => selectNetwork(item.dataset.ssid));
        item.addEventListener('keydown', (e) => {
          if (e.key === 'Enter' || e.key === ' ') {
            e.preventDefault();
            selectNetwork(item.dataset.ssid);
          }
        });
      });
    };

    const selectNetwork = (ssid) => {
      document.getElementById("ssid2").value = ssid;
      document.getElementById("password2").focus();
      showToast(`Selected: ${ssid}`, "info");
    };

    // ==================== Station Presets ====================

    const setStation = (code) => {
      const input = document.getElementById("station");

      autocompleteJustSelected = true;
      setTimeout(() => { autocompleteJustSelected = false; }, 100);

      input.value = code;
      showToast(`Station set to: ${code}`, "success");
      input.blur();

      autoApplySettings(code);
    };

    // ==================== Form Validation ====================

    const setupValidation = () => {
      const stationInput = document.getElementById("station");
      stationInput.addEventListener("input", (e) => {
        const val = e.target.value.trim();

        if (val.length === 3 && val.toUpperCase() === val) {
          e.target.classList.add("success");
          e.target.classList.remove("error");
        } else if (val.length > 0 && val.length <= 50) {
          e.target.classList.remove("success");
          e.target.classList.remove("error");
        } else {
          e.target.classList.remove("success");
        }
      });

      const intervalInput = document.getElementById("interval");
      intervalInput.addEventListener("input", (e) => {
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
      ["mode", "extra"].forEach(id => {
        document.getElementById(id).addEventListener("change", () => {
          if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({command: "getState"}));
          }
        });
      });
    };

    // ==================== Form Submission ====================

    const setupForms = () => {
      // Network form submission
      document.getElementById("networkForm").addEventListener("submit", (e) => {
        if (!confirm("Save WiFi settings and restart?\n\nThe device will restart and may take 10-15 seconds to reconnect.")) {
          e.preventDefault();
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
      });
    };

    // ==================== Reset Modal ====================

    const showResetModal = () => {
      document.getElementById("resetModal").classList.add("show");
    };

    const hideResetModal = () => {
      document.getElementById("resetModal").classList.remove("show");
    };

    const confirmReset = () => {
      hideResetModal();
      showToast("Resetting device...", "warning");

      setTimeout(() => {
        window.location.href = "/reset";
      }, 1000);
    };

    // ==================== Auto-Apply Settings ====================

    const autoApplySettings = (stationCodeOverride) => {
      const formData = new URLSearchParams();
      const stationValue = stationCodeOverride || document.getElementById('station').value;
      formData.append('serviceType', document.getElementById('serviceType').value);
      formData.append('tflApiKey', document.getElementById('tflApiKey').value);
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

      showToast("Applying changes...", "info");

      fetch("/apply", {
        method: "POST",
        body: formData
      })
      .then(() => {
        showToast("Settings updated!", "success");

        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send(JSON.stringify({command: "getState"}));
        }
      })
      .catch(error => {
        showToast("Failed to apply settings", "error");
      });
    };

    // ==================== Initialization ====================

    document.addEventListener("DOMContentLoaded", () => {
      connectWebSocket();
      loadStationData();
      setupStationAutocomplete();
      setupValidation();
      setupForms();

      // Service type selection handler
      const serviceTypeSelect = document.getElementById("serviceType");
      const tflApiKeyGroup = document.getElementById("tflApiKeyGroup");
      const stationLabel = document.getElementById("stationLabel");
      const stationTooltip = document.getElementById("stationTooltip");
      const railPresets = document.getElementById("railPresets");
      const tflPresets = document.getElementById("tflPresets");

      const updateServiceTypeUI = () => {
        const isUnderground = serviceTypeSelect.value === "1";
        tflApiKeyGroup.style.display = isUnderground ? "block" : "none";
        railPresets.style.display = isUnderground ? "none" : "flex";
        tflPresets.style.display = isUnderground ? "flex" : "none";

        if (isUnderground) {
          stationLabel.textContent = "TFL Station ID (NaPTAN)";
          stationTooltip.title = "TFL Station NaPTAN ID (e.g., 940GZZLUPAC for Paddington)";
        } else {
          stationLabel.textContent = "Station Code (CRS)";
          stationTooltip.title = "Three-letter National Rail station code";
        }
      };

      serviceTypeSelect.addEventListener("change", () => {
        updateServiceTypeUI();
        autoApplySettings();
      });

      // Initialize UI on load
      updateServiceTypeUI();

      // Update current station display
      const stationInput = document.getElementById("station");
      if (stationInput.value) {
        document.getElementById("currentStation").textContent = stationInput.value;
      }

      // Auto-apply for all settings except WiFi
      const autoApplyFields = ["station", "interval", "mode", "showstation", "extra", "rotationspeed", "scrollspeed", "ytop", "y1", "y2", "y3"];
      autoApplyFields.forEach(fieldId => {
        const field = document.getElementById(fieldId);
        if (field) {
          if (fieldId === "station") {
            field.addEventListener("keypress", (e) => {
              if (e.key === "Enter" && field.value.length >= 3) {
                e.preventDefault();
                autoApplySettings();
              }
            });

            field.addEventListener("change", () => {
              if (!autocompleteJustSelected && field.value.length >= 3) {
                autoApplySettings();
              }
            });
          } else {
            field.addEventListener("change", () => {
              autoApplySettings();
            });
          }
        }
      });

      // Tab button event listeners
      document.querySelectorAll('.tab-button').forEach((btn, index) => {
        btn.addEventListener('click', () => switchTab(index));
      });

      // Preset station buttons
      document.querySelectorAll('.preset-btn').forEach(btn => {
        btn.addEventListener('click', () => setStation(btn.dataset.station));
      });

      // Reset button
      document.getElementById('resetButton').addEventListener('click', showResetModal);
      document.getElementById('cancelResetBtn').addEventListener('click', hideResetModal);
      document.getElementById('confirmResetBtn').addEventListener('click', confirmReset);

      // Scan networks button
      document.getElementById('scanNetworksBtn').addEventListener('click', scanNetworks);
    });
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
  <meta name="description" content="Saving settings - StationBoards">
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
    <div class="spinner" role="status" aria-label="Loading"></div>
    <h1>💾 Saving Configuration</h1>
    <p>Your settings have been saved successfully.</p>
    <p style="margin-top: 10px;">The device is restarting...</p>
    <p style="margin-top: 20px; font-size: 14px; color: #999;">Please wait 10-15 seconds</p>
  </div>
  <script>
    setTimeout(() => {
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
  <meta name="description" content="Settings applied - StationBoards">
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
    <div class="success-icon" aria-hidden="true">✓</div>
    <h1>Settings Applied!</h1>
    <p>Your changes have been applied successfully.</p>
    <p>The device is updating without restarting.</p>
    <a href="/" class="btn">← Back to Dashboard</a>
  </div>
  <script>
    setTimeout(() => {
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
  <meta name="description" content="Factory reset - StationBoards">
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
    <div class="spinner" role="status" aria-label="Loading"></div>
    <h1>🔄 Factory Reset</h1>
    <p>All settings have been erased.</p>
    <p style="margin-top: 10px;">The device is restarting in setup mode...</p>
    <p style="margin-top: 20px; font-size: 14px; color: #999;">Connect to "TrainBoard_AP" WiFi network</p>
  </div>
</body>
</html>
)HTMLCODE";

const char SETUP_WIZARD_PAGE[] PROGMEM = R"HTMLCODE(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="description" content="StationBoards Setup Wizard - First Time Configuration">
  <title>StationBoards Setup Wizard</title>
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
      max-width: 700px;
      margin: 0 auto;
    }

    .wizard-card {
      background: white;
      border-radius: 16px;
      box-shadow: 0 10px 40px rgba(0,0,0,0.2);
      overflow: hidden;
      animation: slideUp 0.5s ease;
    }

    @keyframes slideUp {
      from {
        opacity: 0;
        transform: translateY(30px);
      }
      to {
        opacity: 1;
        transform: translateY(0);
      }
    }

    .wizard-header {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 30px;
      text-align: center;
    }

    .wizard-header h1 {
      font-size: 28px;
      margin-bottom: 10px;
    }

    .wizard-header p {
      font-size: 14px;
      opacity: 0.9;
    }

    .progress-bar {
      background: rgba(255,255,255,0.2);
      height: 4px;
      margin-top: 20px;
      border-radius: 2px;
      overflow: hidden;
    }

    .progress-fill {
      background: white;
      height: 100%;
      transition: width 0.3s ease;
    }

    .step-indicator {
      display: flex;
      justify-content: center;
      gap: 10px;
      margin-top: 15px;
    }

    .step-dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: rgba(255,255,255,0.3);
      transition: all 0.3s ease;
    }

    .step-dot.active {
      background: white;
      width: 30px;
      border-radius: 5px;
    }

    .wizard-content {
      padding: 40px 30px;
    }

    .step {
      display: none;
    }

    .step.active {
      display: block;
      animation: fadeIn 0.4s ease;
    }

    @keyframes fadeIn {
      from { opacity: 0; transform: translateX(20px); }
      to { opacity: 1; transform: translateX(0); }
    }

    .step h2 {
      color: #667eea;
      font-size: 24px;
      margin-bottom: 10px;
    }

    .step-description {
      color: #666;
      font-size: 14px;
      margin-bottom: 30px;
      line-height: 1.6;
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

    .form-group input[type="text"],
    .form-group input[type="password"],
    .form-group input[type="number"],
    .form-group select {
      width: 100%;
      padding: 14px 16px;
      border: 2px solid #e0e0e0;
      border-radius: 8px;
      font-size: 15px;
      transition: all 0.3s ease;
      font-family: inherit;
    }

    .form-group input:focus,
    .form-group select:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }

    .help-text {
      font-size: 12px;
      color: #666;
      margin-top: 5px;
    }

    .btn {
      padding: 14px 28px;
      border: none;
      border-radius: 8px;
      font-size: 15px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      font-family: inherit;
    }

    .btn-primary {
      background: #667eea;
      color: white;
    }

    .btn-primary:hover:not(:disabled) {
      background: #5568d3;
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
    }

    .btn-secondary {
      background: #e0e0e0;
      color: #333;
    }

    .btn-secondary:hover:not(:disabled) {
      background: #d0d0d0;
    }

    .btn-outline {
      background: white;
      border: 2px solid #667eea;
      color: #667eea;
    }

    .btn-outline:hover:not(:disabled) {
      background: #667eea;
      color: white;
    }

    .btn:disabled {
      opacity: 0.6;
      cursor: not-allowed;
    }

    .button-group {
      display: flex;
      gap: 15px;
      margin-top: 30px;
    }

    .button-group .btn {
      flex: 1;
    }

    .network-list {
      max-height: 300px;
      overflow-y: auto;
      border: 2px solid #e0e0e0;
      border-radius: 8px;
      margin-bottom: 20px;
    }

    .network-item {
      padding: 14px 16px;
      border-bottom: 1px solid #e0e0e0;
      cursor: pointer;
      transition: all 0.2s ease;
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

    .network-item.selected {
      background: #e7f3ff;
      border-left: 4px solid #667eea;
    }

    .network-name {
      font-weight: 600;
      color: #333;
    }

    .network-signal {
      font-size: 12px;
      color: #666;
    }

    .signal-bars {
      display: inline-block;
      margin-right: 8px;
    }

    .spinner {
      width: 40px;
      height: 40px;
      border: 4px solid #f3f3f3;
      border-top: 4px solid #667eea;
      border-radius: 50%;
      animation: spin 1s linear infinite;
      margin: 20px auto;
    }

    @keyframes spin {
      to { transform: rotate(360deg); }
    }

    .loading-text {
      text-align: center;
      color: #666;
      margin-top: 10px;
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
      padding: 12px 16px;
      cursor: pointer;
      border-bottom: 1px solid #f0f0f0;
      transition: background 0.2s ease;
    }

    .autocomplete-item:hover {
      background: #f8f9fa;
    }

    .station-name {
      font-weight: 600;
      color: #333;
      display: block;
    }

    .station-code {
      font-size: 12px;
      color: #667eea;
      font-weight: 500;
    }

    .preset-stations {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(100px, 1fr));
      gap: 10px;
      margin-bottom: 20px;
    }

    .preset-btn {
      padding: 12px 8px;
      background: #f8f9fa;
      border: 2px solid #e0e0e0;
      border-radius: 8px;
      font-size: 13px;
      cursor: pointer;
      transition: all 0.2s ease;
      font-weight: 500;
      text-align: center;
    }

    .preset-btn:hover {
      background: #667eea;
      color: white;
      border-color: #667eea;
    }

    .preset-btn.selected {
      background: #667eea;
      color: white;
      border-color: #667eea;
      box-shadow: 0 2px 8px rgba(102, 126, 234, 0.4);
    }

    .completion-icon {
      font-size: 80px;
      text-align: center;
      margin: 20px 0;
      animation: bounce 0.6s ease;
    }

    @keyframes bounce {
      0%, 100% { transform: translateY(0); }
      50% { transform: translateY(-20px); }
    }

    .info-box {
      background: #e7f3ff;
      border-left: 4px solid #2196F3;
      padding: 15px;
      border-radius: 6px;
      margin: 20px 0;
    }

    .info-box p {
      color: #004085;
      font-size: 14px;
      line-height: 1.6;
      margin: 0;
    }

    .summary-item {
      display: flex;
      justify-content: space-between;
      padding: 12px 0;
      border-bottom: 1px solid #e0e0e0;
    }

    .summary-item:last-child {
      border-bottom: none;
    }

    .summary-label {
      font-weight: 600;
      color: #666;
    }

    .summary-value {
      color: #333;
      font-weight: 500;
    }

    .error-message {
      background: #f8d7da;
      border-left: 4px solid #dc3545;
      padding: 12px;
      border-radius: 6px;
      margin: 15px 0;
      color: #721c24;
      font-size: 14px;
      display: none;
    }

    .error-message.show {
      display: block;
    }

    @media (max-width: 768px) {
      .wizard-content {
        padding: 30px 20px;
      }

      .preset-stations {
        grid-template-columns: repeat(2, 1fr);
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="wizard-card">
      <div class="wizard-header">
        <h1>🚆 Welcome to StationBoards</h1>
        <p>Let's set up your live train departure board</p>
        <div class="progress-bar">
          <div class="progress-fill" id="progressFill"></div>
        </div>
        <div class="step-indicator">
          <div class="step-dot" data-step="0"></div>
          <div class="step-dot" data-step="1"></div>
          <div class="step-dot" data-step="2"></div>
          <div class="step-dot" data-step="3"></div>
        </div>
      </div>

      <div class="wizard-content">
        <!-- Step 0: Welcome -->
        <div class="step active" id="step-0">
          <h2>Welcome! 👋</h2>
          <p class="step-description">
            Thank you for choosing StationBoards! This quick setup wizard will help you configure your
            live train departure board in just a few steps.
          </p>

          <div class="info-box">
            <p>
              <strong>What you'll need:</strong><br>
              • Your WiFi network name and password<br>
              • The station you want to display departures for<br>
              • About 2 minutes of your time
            </p>
          </div>

          <div class="button-group">
            <button class="btn btn-primary" onclick="nextStep()">Let's Get Started →</button>
          </div>
        </div>

        <!-- Step 1: WiFi Setup -->
        <div class="step" id="step-1">
          <h2>WiFi Connection 📡</h2>
          <p class="step-description">
            Connect your StationBoard to your WiFi network to receive live train data.
          </p>

          <div class="error-message" id="wifiError"></div>

          <button type="button" class="btn btn-secondary" onclick="scanNetworks()" id="scanBtn" style="width: 100%; margin-bottom: 20px;">
            <span id="scanBtnText">🔍 Scan for Networks</span>
            <span id="scanBtnSpinner" style="display: none;">Scanning...</span>
          </button>

          <div id="networkListContainer" style="display: none;">
            <div class="network-list" id="networkList"></div>
          </div>

          <div class="form-group">
            <label for="ssid">WiFi Network Name (SSID)</label>
            <input type="text" id="ssid" placeholder="Enter network name" required>
            <span class="help-text">Or select from scanned networks above</span>
          </div>

          <div class="form-group">
            <label for="password">WiFi Password</label>
            <input type="password" id="password" placeholder="Enter password">
            <span class="help-text">Leave blank if network has no password</span>
          </div>

          <div class="info-box" id="wifiTestResult" style="display: none; margin-top: 20px;">
            <p id="wifiTestMessage"></p>
          </div>

          <button type="button" class="btn btn-outline" onclick="testWiFiConnection()" id="testWifiBtn" style="width: 100%; margin-top: 10px; margin-bottom: 10px;">
            <span id="testWifiBtnText">🔌 Test Connection</span>
            <span id="testWifiBtnSpinner" style="display: none;">Testing...</span>
          </button>

          <div class="button-group">
            <button class="btn btn-secondary" onclick="prevStep()">← Back</button>
            <button class="btn btn-primary" onclick="nextStep()" id="wifiNextBtn">Next →</button>
          </div>
        </div>

        <!-- Step 2: Station Selection -->
        <div class="step" id="step-2">
          <h2>Choose Your Station 🚉</h2>
          <p class="step-description">
            Select one of the popular stations below, or skip to configure it later.
          </p>

          <div class="preset-stations">
            <button type="button" class="preset-btn" onclick="selectStation('PAD', 'Paddington')">PAD<br><small>Paddington</small></button>
            <button type="button" class="preset-btn" onclick="selectStation('VIC', 'Victoria')">VIC<br><small>Victoria</small></button>
            <button type="button" class="preset-btn" onclick="selectStation('WAT', 'Waterloo')">WAT<br><small>Waterloo</small></button>
            <button type="button" class="preset-btn" onclick="selectStation('KGX', 'Kings Cross')">KGX<br><small>Kings Cross</small></button>
            <button type="button" class="preset-btn" onclick="selectStation('EUS', 'Euston')">EUS<br><small>Euston</small></button>
            <button type="button" class="preset-btn" onclick="selectStation('LST', 'Liverpool St')">LST<br><small>Liverpool St</small></button>
          </div>

          <input type="hidden" id="station" value="">

          <div id="selectedStationDisplay" style="display: none; margin-top: 20px; padding: 15px; background: #e7f3ff; border-left: 4px solid #667eea; border-radius: 6px;">
            <p style="margin: 0; color: #004085; font-weight: 600;">
              ✓ Selected: <span id="selectedStationName"></span>
            </p>
          </div>

          <div class="info-box" style="margin-top: 20px;">
            <p style="margin: 0;">
              <strong>ℹ️ Note:</strong> You can change your station or select any other UK station from the web interface after setup is complete.
            </p>
          </div>

          <div class="button-group">
            <button class="btn btn-secondary" onclick="prevStep()">← Back</button>
            <button class="btn btn-primary" onclick="nextStep()" id="stationNextBtn">Next →</button>
          </div>
        </div>

        <!-- Step 3: Display Preferences -->
        <div class="step" id="step-3">
          <h2>Display Preferences ⚙️</h2>
          <p class="step-description">
            Customize how your departure board looks. You can change these settings later.
          </p>

          <div class="form-group">
            <label for="mode">Display Mode</label>
            <select id="mode">
              <option value="0">Standard View - Show multiple trains</option>
              <option value="1">Calling At - Show stops for first train</option>
            </select>
          </div>

          <div class="form-group">
            <label for="showstation">Show Station Name</label>
            <select id="showstation">
              <option value="1">Show station name at top</option>
              <option value="0">Hide station name (adds extra service line)</option>
            </select>
            <span class="help-text">Hiding the station name adds an extra service at the top for more trains</span>
          </div>

          <div class="form-group">
            <label for="extra">Extra Services</label>
            <select id="extra">
              <option value="0">No extra services</option>
              <option value="1">1 extra service (rotating)</option>
              <option value="2">2 extra services (rotating)</option>
              <option value="3">3 extra services (rotating)</option>
              <option value="4">4 extra services (rotating)</option>
            </select>
            <span class="help-text">Additional trains shown on the bottom line</span>
          </div>

          <div style="margin-top: 30px;">
            <h3 style="color: #667eea; font-size: 18px; margin-bottom: 15px;">🖥️ Display Preview</h3>
            <div id="displayPreview" style="background: #000; color: #fff; padding: 20px; border-radius: 8px; font-family: 'Courier New', monospace; min-height: 180px; border: 3px solid #333;">
              <div style="text-align: center; color: #666; padding: 40px 0; font-size: 14px;">
                Configure your settings above to see a preview
              </div>
            </div>
            <button type="button" class="btn btn-outline" onclick="updatePreview()" id="previewBtn" style="width: 100%; margin-top: 15px;">
              <span id="previewBtnText">🔄 Update Preview</span>
            </button>
          </div>

          <div class="button-group">
            <button class="btn btn-secondary" onclick="prevStep()">← Back</button>
            <button class="btn btn-primary" onclick="showSummary()">Review Settings →</button>
          </div>
        </div>

        <!-- Step 4: Summary & Complete -->
        <div class="step" id="step-4">
          <h2>Summary & Complete ✓</h2>
          <p class="step-description">
            Please review your settings before completing setup.
          </p>

          <div id="summaryContent"></div>

          <div class="info-box" style="margin-top: 30px;">
            <p>
              <strong>What happens next:</strong><br>
              When you click "Complete Setup", your device will save these settings and restart.
              It will then connect to your WiFi network and start displaying live train departures!
            </p>
          </div>

          <div class="error-message" id="completeError"></div>

          <div class="button-group">
            <button class="btn btn-secondary" onclick="prevStep()">← Back</button>
            <button class="btn btn-primary" onclick="completeSetup()" id="completeBtn">Complete Setup 🎉</button>
          </div>
        </div>

        <!-- Step 5: Completion -->
        <div class="step" id="step-5">
          <div class="completion-icon">🎉</div>
          <h2 style="text-align: center;">Setup Complete!</h2>
          <p class="step-description" style="text-align: center;">
            Your StationBoard is now configured and restarting...
          </p>

          <div class="spinner"></div>
          <p class="loading-text">Connecting to <strong id="finalSSID"></strong>...</p>
          <p class="loading-text">This may take 15-20 seconds</p>

          <div class="info-box" style="margin-top: 30px;">
            <p style="text-align: center; margin-bottom: 15px;">
              <strong>Next Steps:</strong>
            </p>
            <ol style="text-align: left; margin: 0 auto; max-width: 400px; line-height: 1.8;">
              <li>Reconnect your device to your WiFi network (<strong id="finalSSID2"></strong>)</li>
              <li>Open your browser and visit:<br>
                  <strong style="color: #667eea; font-size: 16px;">http://stationboard.local</strong>
              </li>
              <li>Or check the OLED display for the IP address</li>
            </ol>
          </div>
        </div>
      </div>
    </div>
  </div>

  <script>
    let currentStep = 0;
    const totalSteps = 4;
    let stationData = [];
    let stationDataLoaded = false;
    let selectedNetworkSSID = '';

    // Utility: Escape HTML
    const escapeHtml = (unsafe) => {
      if (unsafe === null || unsafe === undefined) return '';
      return String(unsafe)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
    };

    // Utility: Debounce
    const debounce = (func, wait) => {
      let timeout;
      return function executedFunction(...args) {
        const later = () => {
          clearTimeout(timeout);
          func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
      };
    };

    // Update progress bar and step indicators
    function updateProgress() {
      const progress = (currentStep / totalSteps) * 100;
      document.getElementById('progressFill').style.width = progress + '%';

      document.querySelectorAll('.step-dot').forEach((dot, index) => {
        dot.classList.toggle('active', index === currentStep);
      });
    }

    // Navigate to next step
    function nextStep() {
      // Validation
      if (currentStep === 1) {
        const ssid = document.getElementById('ssid').value.trim();
        if (!ssid) {
          showError('wifiError', 'Please enter a WiFi network name');
          return;
        }
        hideError('wifiError');
      }

      // Station is optional - no validation needed for step 2

      if (currentStep >= totalSteps) return;

      document.getElementById('step-' + currentStep).classList.remove('active');
      currentStep++;
      document.getElementById('step-' + currentStep).classList.add('active');
      updateProgress();

      // Scroll to top
      window.scrollTo({ top: 0, behavior: 'smooth' });
    }

    // Navigate to previous step
    function prevStep() {
      if (currentStep <= 0) return;

      document.getElementById('step-' + currentStep).classList.remove('active');
      currentStep--;
      document.getElementById('step-' + currentStep).classList.add('active');
      updateProgress();

      window.scrollTo({ top: 0, behavior: 'smooth' });
    }

    // Show error message
    function showError(elementId, message) {
      const errorEl = document.getElementById(elementId);
      errorEl.textContent = message;
      errorEl.classList.add('show');
    }

    // Hide error message
    function hideError(elementId) {
      const errorEl = document.getElementById(elementId);
      errorEl.classList.remove('show');
    }

    // Scan WiFi networks
    function scanNetworks() {
      const btn = document.getElementById('scanBtn');
      const btnText = document.getElementById('scanBtnText');
      const btnSpinner = document.getElementById('scanBtnSpinner');

      btn.disabled = true;
      btnText.style.display = 'none';
      btnSpinner.style.display = 'inline';

      fetch('/scan')
        .then(response => response.json())
        .then(data => {
          displayNetworks(data.networks);
        })
        .catch(error => {
          console.error('Scan error:', error);
          showError('wifiError', 'Failed to scan networks. Please enter WiFi details manually.');
        })
        .finally(() => {
          btn.disabled = false;
          btnText.style.display = 'inline';
          btnSpinner.style.display = 'none';
        });
    }

    // Display scanned networks
    function displayNetworks(networks) {
      const container = document.getElementById('networkListContainer');
      const list = document.getElementById('networkList');

      if (networks.length === 0) {
        showError('wifiError', 'No networks found. Please enter WiFi details manually.');
        return;
      }

      hideError('wifiError');
      container.style.display = 'block';

      list.innerHTML = networks.map(network => {
        const strength = network.rssi > -50 ? '▂▄▆█' : network.rssi > -70 ? '▂▄▆' : '▂▄';
        return `<div class="network-item" onclick="selectNetwork('${escapeHtml(network.ssid)}')">
          <span class="network-name">${escapeHtml(network.ssid)}</span>
          <span class="network-signal">
            <span class="signal-bars">${strength}</span>
            ${escapeHtml(network.rssi)} dBm
          </span>
        </div>`;
      }).join('');
    }

    // Select a network from the list
    function selectNetwork(ssid) {
      selectedNetworkSSID = ssid;
      document.getElementById('ssid').value = ssid;
      document.getElementById('password').focus();

      // Highlight selected network
      document.querySelectorAll('.network-item').forEach(item => {
        item.classList.toggle('selected', item.textContent.includes(ssid));
      });

      hideError('wifiError');
    }

    // Load station data for autocomplete
    function loadStationData() {
      const fallbackStations = [
        {name: "London Paddington", code: "PAD"}, {name: "London Victoria", code: "VIC"},
        {name: "London Waterloo", code: "WAT"}, {name: "London Kings Cross", code: "KGX"},
        {name: "London Euston", code: "EUS"}, {name: "London Liverpool Street", code: "LST"},
        {name: "London St Pancras International", code: "STP"}, {name: "London Bridge", code: "LBG"},
        {name: "Birmingham New Street", code: "BHM"}, {name: "Manchester Piccadilly", code: "MAN"},
        {name: "Edinburgh Waverley", code: "EDB"}, {name: "Glasgow Central", code: "GLC"},
        {name: "Leeds", code: "LDS"}, {name: "Liverpool Lime Street", code: "LIV"},
        {name: "Bristol Temple Meads", code: "BRI"}, {name: "Cardiff Central", code: "CDF"}
      ];

      const stationsURL = "https://raw.githubusercontent.com/davwheat/uk-railway-stations/main/stations.json";

      fetch(stationsURL, { timeout: 10000 })
        .then(response => response.ok ? response.json() : Promise.reject())
        .then(data => {
          stationData = data.map(station => ({
            name: station.stationName || station.name,
            code: (station.crsCode || station.code || "").toUpperCase()
          })).filter(station => station.code && station.code.length === 3);
          stationDataLoaded = true;
          console.log(`Loaded ${stationData.length} stations`);
        })
        .catch(() => {
          console.log("Using fallback station data");
          stationData = fallbackStations;
          stationDataLoaded = true;
        });
    }

    // Setup station autocomplete
    function setupStationAutocomplete() {
      const input = document.getElementById('station');
      const results = document.getElementById('stationAutocomplete');

      const handleInput = debounce((e) => {
        const query = e.target.value.toUpperCase().trim();

        if (query.length < 2 || !stationDataLoaded) {
          results.classList.remove('show');
          return;
        }

        const matches = stationData.filter(station =>
          station.name.toUpperCase().includes(query) ||
          station.code.includes(query)
        ).slice(0, 10);

        if (matches.length === 0) {
          results.innerHTML = '<div style="padding: 20px; text-align: center; color: #999;">No stations found</div>';
          results.classList.add('show');
          return;
        }

        results.innerHTML = matches.map(station => `
          <div class="autocomplete-item" onclick="selectStation('${escapeHtml(station.code)}', '${escapeHtml(station.name)}')">
            <span class="station-name">${escapeHtml(station.name)}</span>
            <span class="station-code">${escapeHtml(station.code)}</span>
          </div>
        `).join('');

        results.classList.add('show');
      }, 300);

      input.addEventListener('input', handleInput);

      document.addEventListener('click', (e) => {
        if (!input.contains(e.target) && !results.contains(e.target)) {
          results.classList.remove('show');
        }
      });
    }

    // Select a station
    function selectStation(code, name) {
      document.getElementById('station').value = code;

      // Remove selected class from all buttons
      document.querySelectorAll('.preset-btn').forEach(btn => {
        btn.classList.remove('selected');
      });

      // Add selected class to clicked button (find by text content)
      document.querySelectorAll('.preset-btn').forEach(btn => {
        if (btn.textContent.includes(code)) {
          btn.classList.add('selected');
        }
      });

      // Show selected station display
      const display = document.getElementById('selectedStationDisplay');
      const nameEl = document.getElementById('selectedStationName');
      nameEl.textContent = `${code} - ${name}`;
      display.style.display = 'block';
    }

    // Show summary before completion
    function showSummary() {
      const ssid = document.getElementById('ssid').value;
      const station = document.getElementById('station').value;
      const mode = document.getElementById('mode').options[document.getElementById('mode').selectedIndex].text;
      const showstation = document.getElementById('showstation').options[document.getElementById('showstation').selectedIndex].text;
      const extra = document.getElementById('extra').value;

      const summary = `
        <div class="summary-item">
          <span class="summary-label">WiFi Network:</span>
          <span class="summary-value">${escapeHtml(ssid)}</span>
        </div>
        <div class="summary-item">
          <span class="summary-label">Station:</span>
          <span class="summary-value">${station ? escapeHtml(station) : '<em style="color: #999;">Will be configured from web interface</em>'}</span>
        </div>
        <div class="summary-item">
          <span class="summary-label">Display Mode:</span>
          <span class="summary-value">${escapeHtml(mode)}</span>
        </div>
        <div class="summary-item">
          <span class="summary-label">Station Name:</span>
          <span class="summary-value">${escapeHtml(showstation)}</span>
        </div>
        <div class="summary-item">
          <span class="summary-label">Extra Services:</span>
          <span class="summary-value">${escapeHtml(extra)}</span>
        </div>
      `;

      document.getElementById('summaryContent').innerHTML = summary;
      nextStep();
    }

    // Test WiFi connection
    function testWiFiConnection() {
      const ssid = document.getElementById('ssid').value.trim();
      const password = document.getElementById('password').value;

      if (!ssid) {
        showError('wifiError', 'Please enter a WiFi network name');
        return;
      }

      const btn = document.getElementById('testWifiBtn');
      const btnText = document.getElementById('testWifiBtnText');
      const btnSpinner = document.getElementById('testWifiBtnSpinner');
      const resultBox = document.getElementById('wifiTestResult');
      const resultMsg = document.getElementById('wifiTestMessage');

      btn.disabled = true;
      btnText.style.display = 'none';
      btnSpinner.style.display = 'inline';
      resultBox.style.display = 'none';

      const formData = new URLSearchParams();
      formData.append('ssid', ssid);
      formData.append('password', password);

      fetch('/test-wifi', {
        method: 'POST',
        body: formData
      })
      .then(response => response.json())
      .then(data => {
        resultBox.style.display = 'block';
        if (data.success) {
          resultBox.style.background = '#d4edda';
          resultBox.style.borderColor = '#28a745';
          resultMsg.style.color = '#155724';
          resultMsg.innerHTML = `<strong>✓ Connection Successful!</strong><br>Signal strength: ${data.rssi} dBm (${data.quality})`;
        } else {
          resultBox.style.background = '#f8d7da';
          resultBox.style.borderColor = '#dc3545';
          resultMsg.style.color = '#721c24';
          resultMsg.innerHTML = `<strong>✗ Connection Failed</strong><br>${escapeHtml(data.message)}`;
        }
      })
      .catch(error => {
        console.error('Test error:', error);
        resultBox.style.display = 'block';
        resultBox.style.background = '#f8d7da';
        resultBox.style.borderColor = '#dc3545';
        resultMsg.style.color = '#721c24';
        resultMsg.innerHTML = '<strong>✗ Test Failed</strong><br>Could not connect to device';
      })
      .finally(() => {
        btn.disabled = false;
        btnText.style.display = 'inline';
        btnSpinner.style.display = 'none';
      });
    }

    // Validate station code
    function validateStationCode() {
      const station = document.getElementById('station').value.trim().toUpperCase();

      if (!station || station.length !== 3) {
        alert('Please enter a valid 3-letter station code');
        return;
      }

      const btn = document.getElementById('validateStationBtn');
      const btnText = document.getElementById('validateStationBtnText');
      const btnSpinner = document.getElementById('validateStationBtnSpinner');
      const resultBox = document.getElementById('stationValidResult');
      const resultMsg = document.getElementById('stationValidMessage');

      btn.disabled = true;
      btnText.style.display = 'none';
      btnSpinner.style.display = 'inline';
      resultBox.style.display = 'none';

      fetch('/validate-station?code=' + encodeURIComponent(station))
        .then(response => response.json())
        .then(data => {
          resultBox.style.display = 'block';
          if (data.valid && !data.warning && data.name) {
            // Success - verified with API
            resultBox.style.background = '#d4edda';
            resultBox.style.borderColor = '#28a745';
            resultMsg.style.color = '#155724';
            resultMsg.innerHTML = `<strong>✓ Valid Station!</strong><br>${escapeHtml(data.name)}<br>Services: ${data.serviceCount || 'Unknown'}`;
          } else if (data.valid && data.warning) {
            // Warning - couldn't verify but format is valid
            resultBox.style.background = '#fff3cd';
            resultBox.style.borderColor = '#ffc107';
            resultMsg.style.color = '#856404';
            resultMsg.innerHTML = `<strong>⚠ Could Not Verify</strong><br>${escapeHtml(data.message)}<br><small>You can still continue with this station code</small>`;
          } else {
            // Invalid station
            resultBox.style.background = '#f8d7da';
            resultBox.style.borderColor = '#dc3545';
            resultMsg.style.color = '#721c24';
            resultMsg.innerHTML = `<strong>✗ Invalid Station</strong><br>${escapeHtml(data.message || 'Station code not found')}`;
          }
        })
        .catch(error => {
          console.error('Validation error:', error);
          resultBox.style.display = 'block';
          resultBox.style.background = '#fff3cd';
          resultBox.style.borderColor = '#ffc107';
          resultMsg.style.color = '#856404';
          resultMsg.innerHTML = '<strong>⚠ Validation Unavailable</strong><br>Could not verify station code, but you can still continue';
        })
        .finally(() => {
          btn.disabled = false;
          btnText.style.display = 'inline';
          btnSpinner.style.display = 'none';
        });
    }

    // Update display preview
    function updatePreview() {
      const station = document.getElementById('station').value.trim().toUpperCase();
      const mode = document.getElementById('mode').value;
      const showstation = document.getElementById('showstation').value;
      const extra = document.getElementById('extra').value;

      if (!station || station.length !== 3) {
        alert('Please select a station first');
        return;
      }

      const preview = document.getElementById('displayPreview');
      const isCallingAt = mode === '1';

      // Create mock preview
      let html = '<div style="font-size: 13px; line-height: 1.6;">';

      // Station name (only if showstation is '1')
      if (showstation === '1') {
        html += '<div style="text-align: center; font-weight: bold; margin-bottom: 12px; border-bottom: 1px solid #333; padding-bottom: 8px; font-size: 15px;">';
        html += escapeHtml(station);
        html += '</div>';
      }

      // Mock services
      const mockServices = [
        { time: '14:30', dest: 'Reading', etd: 'On time' },
        { time: '14:45', dest: 'Oxford', etd: 'On time' },
        { time: '15:00', dest: 'Bristol TM', etd: 'Exp 15:05' }
      ];

      if (isCallingAt) {
        // Show first service with calling points
        html += '<div style="margin-bottom: 10px; padding: 8px; background: #111; border-radius: 4px;">';
        html += `<div style="display: flex; justify-content: space-between; margin-bottom: 5px;">`;
        html += `<span><strong>1st</strong> ${mockServices[0].time} ${mockServices[0].dest}</span>`;
        html += `<span style="color: #ffa500; font-weight: bold;">${mockServices[0].etd}</span>`;
        html += `</div>`;
        html += `<div style="font-size: 10px; color: #ccc; margin-top: 5px;">`;
        html += `<strong>Calling at:</strong> Slough, Maidenhead, Twyford`;
        html += `</div></div>`;
      } else {
        // Standard mode - show two services
        for (let i = 0; i < 2; i++) {
          html += '<div style="margin-bottom: 8px; padding: 8px; background: #111; border-radius: 4px;">';
          html += `<div style="display: flex; justify-content: space-between;">`;
          html += `<span><strong>${i === 0 ? '1st' : '2nd'}</strong> ${mockServices[i].time} ${mockServices[i].dest}</span>`;
          html += `<span style="color: #ffa500; font-weight: bold;">${mockServices[i].etd}</span>`;
          html += `</div></div>`;
        }
      }

      // Extra services
      if (extra > 0) {
        html += '<div style="margin-top: 8px; padding: 8px; background: #0a0a0a; border-radius: 4px; border-left: 3px solid #667eea;">';
        html += `<div style="display: flex; justify-content: space-between;">`;
        html += `<span><strong>3rd</strong> ${mockServices[2].time} ${mockServices[2].dest}</span>`;
        html += `<span style="color: #ffa500; font-weight: bold;">${mockServices[2].etd}</span>`;
        html += `</div>`;
        if (extra > 1) {
          html += `<div style="margin-top: 5px; font-size: 10px; color: #888; font-style: italic;">↻ Rotates with ${extra - 1} more</div>`;
        }
        html += '</div>';
      }

      // Current time
      const now = new Date();
      const timeStr = now.toLocaleTimeString('en-GB', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
      html += '<div style="text-align: center; margin-top: 12px; padding-top: 8px; border-top: 1px solid #333; font-size: 11px; color: #999;">';
      html += timeStr;
      html += '</div>';

      html += '</div>';
      preview.innerHTML = html;
    }

    // Complete setup and submit
    function completeSetup() {
      const btn = document.getElementById('completeBtn');
      btn.disabled = true;
      btn.textContent = 'Saving...';

      hideError('completeError');

      const formData = new URLSearchParams();
      const selectedStation = document.getElementById('station').value;

      formData.append('ssid', document.getElementById('ssid').value);
      formData.append('password', document.getElementById('password').value);
      formData.append('station', selectedStation || 'PAD');  // Default to PAD if not selected
      formData.append('mode', document.getElementById('mode').value);
      formData.append('showstation', document.getElementById('showstation').value);
      formData.append('extra', document.getElementById('extra').value);
      formData.append('interval', '60');  // Default
      formData.append('scrollspeed', '50');  // Default
      formData.append('rotationspeed', '15');  // Default

      fetch('/wizard-complete', {
        method: 'POST',
        body: formData
      })
      .then(response => {
        if (!response.ok) throw new Error('Setup failed');

        // Show completion screen
        const ssidName = document.getElementById('ssid').value;
        document.getElementById('step-' + currentStep).classList.remove('active');
        document.getElementById('step-5').classList.add('active');
        document.getElementById('finalSSID').textContent = ssidName;
        document.getElementById('finalSSID2').textContent = ssidName;

        // Update progress to 100%
        document.getElementById('progressFill').style.width = '100%';

        // No redirect - user needs to reconnect to their WiFi first
      })
      .catch(error => {
        console.error('Setup error:', error);
        showError('completeError', 'Failed to save settings. Please try again.');
        btn.disabled = false;
        btn.textContent = 'Complete Setup 🎉';
      });
    }

    // Initialize on page load
    document.addEventListener('DOMContentLoaded', () => {
      updateProgress();
      // Station autocomplete not needed - using preset buttons only
    });
  </script>
</body>
</html>
)HTMLCODE";

#endif
