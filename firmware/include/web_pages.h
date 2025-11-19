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

    /* Collapsible sections */
    .collapsible-header {
      transition: all 0.3s ease;
    }

    .collapsible-header:hover {
      opacity: 0.8;
    }

    .collapsible-header .chevron {
      transition: transform 0.3s ease;
      font-size: 14px;
      color: #667eea;
    }

    .collapsible-header[aria-expanded="false"] .chevron {
      transform: rotate(-90deg);
    }

    .collapsible-content {
      max-height: 2000px;
      overflow: hidden;
      transition: max-height 0.3s ease, opacity 0.3s ease, margin-top 0.3s ease;
      opacity: 1;
      margin-top: 0;
    }

    .collapsible-content.collapsed {
      max-height: 0;
      opacity: 0;
      margin-top: -20px;
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
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.3);
    }

    .preset-category-tabs {
      display: flex;
      gap: 8px;
      margin-bottom: 12px;
      flex-wrap: wrap;
    }

    .preset-category-tab {
      padding: 8px 16px;
      background: white;
      border: 2px solid #e0e0e0;
      border-radius: 20px;
      font-size: 13px;
      font-weight: 600;
      color: #666;
      cursor: pointer;
      transition: all 0.2s ease;
    }

    .preset-category-tab:hover {
      border-color: #667eea;
      color: #667eea;
    }

    .preset-category-tab.active {
      background: #667eea;
      border-color: #667eea;
      color: white;
    }

    .preset-btn.selected {
      background: #667eea;
      color: white;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.2);
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

    .range-slider-group {
      background: #f8f9fa;
      padding: 15px;
      border-radius: 8px;
      border: 2px solid #e0e0e0;
      margin-top: 8px;
    }

    .range-slider-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 12px;
      color: #666;
      margin-bottom: 5px;
    }

    .range-label {
      font-weight: 500;
    }

    .range-value {
      display: flex;
      align-items: center;
      gap: 5px;
      padding: 4px 12px;
      background: white;
      border-radius: 6px;
      border: 2px solid #667eea;
      font-weight: 600;
      color: #667eea;
    }

    .range-slider {
      height: 8px;
      border-radius: 4px;
      background: linear-gradient(to right, #28a745 0%, #ffc107 50%, #dc3545 100%);
      outline: none;
      cursor: pointer;
      -webkit-appearance: none;
      appearance: none;
    }

    .range-slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: #667eea;
      cursor: pointer;
      border: 3px solid white;
      box-shadow: 0 2px 8px rgba(0,0,0,0.2);
      transition: all 0.2s ease;
    }

    .range-slider::-webkit-slider-thumb:hover {
      transform: scale(1.15);
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
    }

    .range-slider::-moz-range-thumb {
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: #667eea;
      cursor: pointer;
      border: 3px solid white;
      box-shadow: 0 2px 8px rgba(0,0,0,0.2);
      transition: all 0.2s ease;
    }

    .range-slider::-moz-range-thumb:hover {
      transform: scale(1.15);
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
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
      transition: top 0.3s ease, transform 0.3s ease;
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

    /* Signal Bars */
    .signal-bars {
      display: inline-flex;
      gap: 3px;
      align-items: flex-end;
      height: 20px;
      vertical-align: middle;
    }

    .signal-bar {
      width: 4px;
      background: #e0e0e0;
      border-radius: 2px;
      transition: background 0.3s ease;
    }

    .signal-bar:nth-child(1) { height: 25%; }
    .signal-bar:nth-child(2) { height: 50%; }
    .signal-bar:nth-child(3) { height: 75%; }
    .signal-bar:nth-child(4) { height: 100%; }

    .signal-bar.active {
      background: #28a745;
    }

    .signal-bar.active.weak {
      background: #dc3545;
    }

    .signal-bar.active.fair {
      background: #ffc107;
    }

    .signal-bar.active.good,
    .signal-bar.active.excellent {
      background: #28a745;
    }

    /* Device Info Panel */
    .device-info-panel {
      margin-top: 10px;
      background: #f8f9fa;
      border-radius: 8px;
      overflow: hidden;
      transition: all 0.3s ease;
    }

    .device-info-toggle {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      padding: 8px;
      cursor: pointer;
      font-size: 11px;
      color: #667eea;
      font-weight: 600;
      transition: background 0.2s ease;
    }

    .device-info-toggle:hover {
      background: #e9ecef;
    }

    .device-info-content {
      display: none;
      padding: 12px;
      font-size: 11px;
      color: #666;
      border-top: 1px solid #e0e0e0;
    }

    .device-info-content.show {
      display: block;
    }

    .device-info-row {
      display: flex;
      justify-content: space-between;
      padding: 6px 0;
      border-bottom: 1px solid #f0f0f0;
    }

    .device-info-row:last-child {
      border-bottom: none;
    }

    .device-info-label {
      font-weight: 600;
      color: #333;
    }

    .uptime-counter {
      font-family: 'Courier New', monospace;
      color: #667eea;
      font-weight: 600;
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
        <div class="label">Status</div>
        <div class="value">
          <span class="status-badge offline" id="deviceStatus">Connecting...</span>
        </div>
      </div>
      <div class="status-item">
        <div class="label">WiFi Signal</div>
        <div class="value" style="display: flex; align-items: center; justify-content: center; gap: 8px;" id="wifiStrength" aria-live="polite">
          <div class="signal-bars" id="signalBars" aria-label="WiFi signal strength">
            <span class="signal-bar"></span>
            <span class="signal-bar"></span>
            <span class="signal-bar"></span>
            <span class="signal-bar"></span>
          </div>
          <span style="font-size: 11px; color: #666;" id="rssiText">--</span>
        </div>
      </div>
      <div class="status-item">
        <div class="label">Current Station</div>
        <div class="value" id="currentStation" aria-live="polite">{STATION_NAME}</div>
      </div>
      <div class="status-item">
        <div class="label">Last Data Fetch</div>
        <div class="value" id="lastFetchTime" aria-live="polite">Never</div>
      </div>
      <div class="status-item">
        <div class="label">Uptime</div>
        <div class="value uptime-counter" id="deviceUptime" aria-live="polite">--:--:--</div>
      </div>
      <div class="status-item" style="grid-column: span 2; display: flex; justify-content: center; align-items: center;">
        <button type="button" id="refreshNowBtn" class="btn" style="padding: 10px 20px; font-size: 14px;" aria-label="Manually refresh departure data now">
          <span class="btn-text">🔄 Refresh Data Now</span>
        </button>
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

          <!-- National Rail Presets with Category Tabs -->
          <div id="nationalRailPresets">
            <div style="margin-bottom: 15px;">
              <div class="preset-category-tabs" id="nationalRailTabs">
                <button type="button" class="preset-category-tab active" data-category="london" aria-label="Show London stations">London</button>
                <button type="button" class="preset-category-tab" data-category="major" aria-label="Show major city stations">Major Cities</button>
                <button type="button" class="preset-category-tab" data-category="airports" aria-label="Show airport stations">Airports</button>
                <button type="button" class="preset-category-tab" data-category="recent" aria-label="Show recently used stations">Recent</button>
              </div>
            </div>

            <div class="preset-stations-container">
              <!-- London Stations (default visible) -->
              <div class="preset-stations" id="presets-london">
                <button type="button" class="preset-btn" data-station="PAD" data-name="Paddington" aria-label="Select Paddington station">Paddington</button>
                <button type="button" class="preset-btn" data-station="VIC" data-name="Victoria" aria-label="Select Victoria station">Victoria</button>
                <button type="button" class="preset-btn" data-station="WAT" data-name="Waterloo" aria-label="Select Waterloo station">Waterloo</button>
                <button type="button" class="preset-btn" data-station="KGX" data-name="Kings Cross" aria-label="Select Kings Cross station">Kings Cross</button>
                <button type="button" class="preset-btn" data-station="EUS" data-name="Euston" aria-label="Select Euston station">Euston</button>
                <button type="button" class="preset-btn" data-station="LST" data-name="Liverpool Street" aria-label="Select Liverpool Street station">Liverpool St</button>
              </div>

              <!-- Major Cities (hidden by default) -->
              <div class="preset-stations" id="presets-major" style="display: none;">
                <button type="button" class="preset-btn" data-station="MAN" data-name="Manchester Piccadilly" aria-label="Select Manchester Piccadilly station">Manchester</button>
                <button type="button" class="preset-btn" data-station="BHM" data-name="Birmingham New Street" aria-label="Select Birmingham New Street station">Birmingham</button>
                <button type="button" class="preset-btn" data-station="EDB" data-name="Edinburgh Waverley" aria-label="Select Edinburgh Waverley station">Edinburgh</button>
                <button type="button" class="preset-btn" data-station="GLC" data-name="Glasgow Central" aria-label="Select Glasgow Central station">Glasgow</button>
                <button type="button" class="preset-btn" data-station="LDS" data-name="Leeds" aria-label="Select Leeds station">Leeds</button>
                <button type="button" class="preset-btn" data-station="LIV" data-name="Liverpool Lime Street" aria-label="Select Liverpool Lime Street station">Liverpool</button>
              </div>

              <!-- Airports -->
              <div class="preset-stations" id="presets-airports" style="display: none;">
                <button type="button" class="preset-btn" data-station="GTW" data-name="Gatwick Airport" aria-label="Select Gatwick Airport station">Gatwick</button>
                <button type="button" class="preset-btn" data-station="SRA" data-name="Stansted Airport" aria-label="Select Stansted Airport station">Stansted</button>
                <button type="button" class="preset-btn" data-station="LTN" data-name="Luton Airport Parkway" aria-label="Select Luton Airport station">Luton</button>
                <button type="button" class="preset-btn" data-station="HWV" data-name="Heathrow Terminals 2 & 3" aria-label="Select Heathrow terminals station">Heathrow</button>
                <button type="button" class="preset-btn" data-station="BHX" data-name="Birmingham International" aria-label="Select Birmingham Airport station">Birmingham Arpt</button>
                <button type="button" class="preset-btn" data-station="MIA" data-name="Manchester Airport" aria-label="Select Manchester Airport station">Manchester Arpt</button>
              </div>

              <!-- Recent Stations (populated from localStorage) -->
              <div class="preset-stations" id="presets-recent" style="display: none;">
                <div style="text-align: center; padding: 40px; color: #999; font-size: 14px;">
                  No recently used stations
                </div>
              </div>
            </div>
          </div>

          <!-- TFL Underground Presets -->
          <div class="preset-stations" id="tflPresets" style="display:none;">
            <button type="button" class="preset-btn" data-station="940GZZLUPAC" data-name="Paddington" aria-label="Select Paddington Underground">Paddington</button>
            <button type="button" class="preset-btn" data-station="940GZZLUVIC" data-name="Victoria" aria-label="Select Victoria Underground">Victoria</button>
            <button type="button" class="preset-btn" data-station="940GZZLUWLO" data-name="Waterloo" aria-label="Select Waterloo Underground">Waterloo</button>
            <button type="button" class="preset-btn" data-station="940GZZLUKSX" data-name="King's Cross" aria-label="Select Kings Cross Underground">King's Cross</button>
            <button type="button" class="preset-btn" data-station="940GZZLUBST" data-name="Baker Street" aria-label="Select Baker Street Underground">Baker Street</button>
            <button type="button" class="preset-btn" data-station="940GZZLULVT" data-name="Liverpool Street" aria-label="Select Liverpool Street Underground">Liverpool St</button>
          </div>

          <!-- Manual Search Divider -->
          <div style="text-align: center; margin: 20px 0 15px 0; color: #999; font-size: 13px; position: relative;">
            <span style="background: white; padding: 0 10px; position: relative; z-index: 1;">OR search manually</span>
            <div style="position: absolute; top: 50%; left: 0; right: 0; height: 1px; background: #e0e0e0; z-index: 0;"></div>
          </div>

          <div class="autocomplete-wrapper">
            <input type="text" id="station" name="station" value="{STATION}" placeholder="Type station name or code..." required maxlength="15" aria-label="Station code or name" aria-describedby="station-help">
            <div id="stationAutocomplete" class="autocomplete-results" role="listbox" aria-label="Station suggestions"></div>
          </div>
          <span class="help-text" id="station-help">Start typing to search for a station</span>
        </div>

        <div class="form-group" id="tflLineFilterGroup" style="display:none;">
          <label for="tflLineFilter">
            Filter by Tube Line
            <span class="info-tooltip" title="Show only arrivals for selected tube line" aria-label="Information: Filter by tube line">?</span>
          </label>
          <select id="tflLineFilter" name="tflLineFilter" aria-describedby="tflline-help">
            <option value="">All Lines</option>
          </select>
          <span class="help-text" id="tflline-help">Select a specific tube line to display, or show all lines</span>
        </div>

        <div class="form-group" id="tflPlatformFilterGroup" style="display:none;">
          <label for="tflPlatformFilter">
            Filter by Platform
            <span class="info-tooltip" title="Show only arrivals for selected platform" aria-label="Information: Filter by platform">?</span>
          </label>
          <select id="tflPlatformFilter" name="tflPlatformFilter" aria-describedby="tflplatform-help">
            <option value="">All Platforms</option>
          </select>
          <span class="help-text" id="tflplatform-help">Select a specific platform to display, or show all platforms</span>
        </div>

        <div class="button-group">
          <button type="button" id="applyStationBtn" class="btn btn-primary" aria-label="Apply station settings">
            <span class="btn-text">Apply Station Settings</span>
          </button>
        </div>
      </div>

      <div class="card">
        <h2>Display Options</h2>
        <p style="margin-top: 0; margin-bottom: 20px; color: #666; font-size: 14px;">These settings apply automatically when changed</p>

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
              Scroll Speed
              <span class="info-tooltip" title="Controls how fast text scrolls across the display" aria-label="Information: Controls how fast text scrolls across the display">?</span>
            </label>
            <select id="scrollspeed" name="scrollspeed" aria-describedby="scrollspeed-help">
              <option value="10"{SCROLL_SEL_10}>10ms (fastest)</option>
              <option value="25"{SCROLL_SEL_25}>25ms (fast)</option>
              <option value="50"{SCROLL_SEL_50}>50ms (moderate)</option>
              <option value="100"{SCROLL_SEL_100}>100ms (smooth)</option>
            </select>
            <span class="help-text" id="scrollspeed-help">Delay between scroll steps - lower is faster, higher is smoother</span>
          </div>
        </div>

        <div class="form-group">
          <label for="mode">Display Mode</label>
          <select id="mode" name="mode" aria-describedby="mode-help">
            <option value="0"{MODE_SEL_0}>Standard View</option>
            <option value="1"{MODE_SEL_1}>Calling At Mode (show all intermediate stops)</option>
          </select>
          <span class="help-text" id="mode-help">Calling At mode displays all intermediate station stops for the first departure</span>
        </div>

        <div class="form-group">
          <label for="showstation">Show Station Name at Top</label>
          <select id="showstation" name="showstation" aria-describedby="showstation-help">
            <option value="1"{SHOWSTATION_SEL_1}>Show Station Name</option>
            <option value="0"{SHOWSTATION_SEL_0}>Hide Station Name (frees space for one more departure)</option>
          </select>
          <span class="help-text" id="showstation-help">Hiding the station name frees up the top line to display an additional departure</span>
        </div>

        <div class="form-group">
          <label for="extra">Rotating Bottom Line Services</label>
          <select id="extra" name="extra" aria-describedby="extra-help">
            <option value="0"{EXTRA_SEL_0}>No rotation (fixed last service)</option>
            <option value="1"{EXTRA_SEL_1}>Rotate 1 extra service</option>
            <option value="2"{EXTRA_SEL_2}>Rotate 2 extra services</option>
            <option value="3"{EXTRA_SEL_3}>Rotate 3 extra services</option>
            <option value="4"{EXTRA_SEL_4}>Rotate 4 extra services</option>
          </select>
          <span class="help-text" id="extra-help">The bottom line of the display automatically cycles through additional departures at the rotation speed below</span>
        </div>

        <div class="form-group">
          <label for="rotationspeed">
            Bottom Line Rotation Speed (seconds)
            <span class="info-tooltip" title="How long each service is displayed before rotating to the next" aria-label="Information: How long each service is displayed">?</span>
          </label>
          <input type="number" id="rotationspeed" name="rotationspeed" value="{ROTATION}" min="5" max="60" required aria-describedby="rotationspeed-help">
          <span class="help-text" id="rotationspeed-help">Recommended: 12-18 seconds. How often the bottom line alternates between services</span>
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
        <h2 class="collapsible-header" id="previewHeader" style="cursor: pointer; user-select: none; display: flex; justify-content: space-between; align-items: center;" role="button" tabindex="0" aria-expanded="true" aria-controls="previewContent">
          <span>🖥️ Live Display Preview</span>
          <span class="chevron" aria-hidden="true">▼</span>
        </h2>
        <div id="previewContent" class="collapsible-content">
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
     * Decode HTML entities for display
     */
    const decodeHtml = (html) => {
      if (html === null || html === undefined) return '';
      const txt = document.createElement('textarea');
      txt.innerHTML = html;
      return txt.value;
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
    let stationData = [];
    let stationDataLoaded = false;
    let autocompleteJustSelected = false;

    // ==================== Station Data Loading ====================

    const loadStationData = () => {
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

      const stationsURL = "https://raw.githubusercontent.com/davwheat/uk-railway-stations/main/stations.json";

      fetchWithTimeout(stationsURL, {}, 10000)
        .then(response => {
          if (!response.ok) throw new Error("API failed");
          return response.json();
        })
        .then(data => {
          stationData = data.map(station => ({
            name: station.stationName || station.name,
            code: (station.crsCode || station.code || "").toUpperCase()
          })).filter(station => station.code && station.code.length === 3);
          stationDataLoaded = true;
          console.log(`Loaded ${stationData.length} stations from API`);
        })
        .catch(error => {
          console.log("Using fallback station data:", error.message);
          stationData = fallbackStations;
          stationDataLoaded = true;
        });
    };

    // ==================== TFL Station Search ====================

    /**
     * Search TFL tube stations from the API
     */
    const searchTflStations = async (query) => {
      if (query.length < 2) return [];

      try {
        const tflApiKey = document.getElementById('tflApiKey')?.value || '';
        const apiUrl = `https://api.tfl.gov.uk/StopPoint/Search?query=${encodeURIComponent(query)}&modes=tube,elizabeth-line${tflApiKey ? '&app_key=' + encodeURIComponent(tflApiKey) : ''}`;

        const response = await fetch(apiUrl);
        if (!response.ok) return [];

        const data = await response.json();

        // Extract station matches (both tube and Elizabeth line)
        const stations = [];
        if (data.matches) {
          data.matches.forEach(match => {
            if (match.modes && (match.modes.includes('tube') || match.modes.includes('elizabeth-line'))) {
              // Remove station type suffixes
              let name = match.name;
              name = name.replace(/ Underground Station$/i, '');
              name = name.replace(/ Station$/i, '');
              name = name.replace(/ Rail Station$/i, '');

              stations.push({
                name: name,
                code: match.id
              });
            }
          });
        }

        return stations.slice(0, 10); // Limit to 10 results
      } catch (error) {
        console.error('Error searching TFL stations:', error);
        return [];
      }
    };

    // ==================== Station Autocomplete ====================

    const setupStationAutocomplete = () => {
      const input = document.getElementById("station");
      const results = document.getElementById("stationAutocomplete");

      // Debounced input handler
      const handleInput = debounce(async (e) => {
        const query = e.target.value.trim();
        const serviceType = document.getElementById('serviceType').value;
        const isTfl = serviceType === '1';

        if (query.length < 2) {
          results.classList.remove("show");
          return;
        }

        // Show loading state
        results.innerHTML = '<div class="autocomplete-loading">Searching...</div>';
        results.classList.add("show");

        let matches = [];

        if (isTfl) {
          // Search TFL stations from API
          matches = await searchTflStations(query);
        } else {
          // Search National Rail stations from local data
          if (!stationDataLoaded) {
            results.classList.remove("show");
            return;
          }

          const queryUpper = query.toUpperCase();
          matches = stationData.filter(station =>
            station.name.toUpperCase().includes(queryUpper) ||
            station.code.includes(queryUpper)
          ).slice(0, 10);
        }

        if (matches.length === 0) {
          results.innerHTML = '<div class="autocomplete-no-results">No stations found</div>';
          results.classList.add("show");
          return;
        }

        results.innerHTML = matches.map(station => {
          const escapedName = escapeHtml(station.name);
          const escapedCode = escapeHtml(station.code);

          return `<div class="autocomplete-item" role="option" data-code="${escapedCode}" data-name="${escapedName}" tabindex="0">
            <span class="station-name">${escapedName}</span>
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
        if (e.target.value.length >= 2) {
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
      console.log('selectStationFromAutocomplete called with code:', code, 'name:', name);
      const input = document.getElementById("station");

      autocompleteJustSelected = true;
      setTimeout(() => { autocompleteJustSelected = false; }, 100);

      // Store the code in a data attribute and show the name in the input
      input.dataset.stationCode = code;
      input.value = name;
      console.log('Set input.value to:', input.value, 'stored code:', code);
      document.getElementById("stationAutocomplete").classList.remove("show");
      showToast(`Selected: ${escapeHtml(name)}. Click "Apply Station Settings" to apply.`, "info");

      input.classList.add("success");
      input.classList.remove("error");
      input.blur();

      // Highlight the selected preset button if it matches, or clear all if custom station
      let matched = false;
      document.querySelectorAll('.preset-btn').forEach(btn => {
        if (btn.dataset.station === code) {
          btn.classList.add('selected');
          matched = true;
        } else {
          btn.classList.remove('selected');
        }
      });

      // Clear line and platform filters when station changes
      const serviceType = document.getElementById('serviceType').value;
      if (serviceType === '1') {
        document.getElementById('tflLineFilter').value = '';
        document.getElementById('tflPlatformFilter').innerHTML = '<option value="">All Platforms</option>';
        document.getElementById('tflPlatformFilter').value = '';
        console.log('Cleared line and platform filters for new station (not yet applied)');

        // Fetch tube lines for the new station (but don't apply yet)
        if (code.length >= 4) {
          setTimeout(() => {
            showTflLineSelector(code.trim());
          }, 100);
        }
      }

      // Note: Don't auto-apply - user must click the Apply Station Settings button
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
            updateLastFetchTime(); // Update when we get new data
          } else if (data.type === "status") {
            showToast(data.message, data.level);
          } else if (data.type === "train_update") {
            if (data.station) {
              const stationEl = document.getElementById("currentStation");
              if (stationEl) {
                stationEl.textContent = decodeHtml(data.station);
              }
            }
            updateLastFetchTime(); // Update when we get new train data
          } else if (data.type === "metrics") {
            updateRSSI(data.rssi);
          } else if (data.type === "state") {
            // Handle station name display
            const stationName = data.stationName || data['displayState.stationName'];
            if (stationName) {
              const stationEl = document.getElementById("currentStation");
              if (stationEl) {
                stationEl.textContent = decodeHtml(stationName);
              }
            }

            // Highlight active preset button based on station code
            if (data.station) {
              highlightActivePresetButton(data.station);
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
      const deviceStatus = document.getElementById("deviceStatus");

      if (indicator && statusText) {
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

      // Update main device status badge
      if (deviceStatus) {
        if (connected) {
          deviceStatus.textContent = "Online";
          deviceStatus.classList.remove("offline");
          deviceStatus.classList.add("online");
        } else {
          deviceStatus.textContent = "Offline";
          deviceStatus.classList.remove("online");
          deviceStatus.classList.add("offline");
        }
      }
    };

    const updateRSSI = (rssi) => {
      if (!rssi) return;

      let signal = "Weak";
      let activeBars = 1;
      let signalClass = "weak";

      if (rssi > -50) {
        signal = "Excellent";
        activeBars = 4;
        signalClass = "excellent";
      } else if (rssi > -60) {
        signal = "Good";
        activeBars = 3;
        signalClass = "good";
      } else if (rssi > -70) {
        signal = "Fair";
        activeBars = 2;
        signalClass = "fair";
      }

      // Update signal bars
      const signalBars = document.querySelectorAll('#signalBars .signal-bar');
      signalBars.forEach((bar, index) => {
        bar.classList.remove('active', 'weak', 'fair', 'good', 'excellent');
        if (index < activeBars) {
          bar.classList.add('active', signalClass);
        }
      });

      // Update RSSI text
      const rssiTextEl = document.getElementById("rssiText");
      if (rssiTextEl) {
        rssiTextEl.textContent = `${escapeHtml(signal)}`;
      }

      // Update RSSI value in device info
      const rssiValueEl = document.getElementById("rssiValue");
      if (rssiValueEl) {
        rssiValueEl.textContent = `${escapeHtml(rssi)} dBm`;
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

    const updateToastPositions = () => {
      const toasts = document.querySelectorAll('.toast');
      let topOffset = 20;
      toasts.forEach(toast => {
        toast.style.top = topOffset + 'px';
        topOffset += toast.offsetHeight + 10; // 10px gap between toasts
      });
    };

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

      // Calculate and set initial position
      setTimeout(() => updateToastPositions(), 10);

      const removeToast = () => {
        toast.style.animation = "slideInRight 0.3s ease reverse";
        setTimeout(() => {
          toast.remove();
          updateToastPositions(); // Reposition remaining toasts
        }, 300);
      };

      setTimeout(removeToast, 3000);
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

    const setStation = (code, name) => {
      const input = document.getElementById("station");

      autocompleteJustSelected = true;
      setTimeout(() => { autocompleteJustSelected = false; }, 100);

      // Store the code in a data attribute and show the name in the input
      input.dataset.stationCode = code;
      input.value = name || code; // Fallback to code if name not provided
      showToast(`Station set to: ${name || code}. Click "Apply Station Settings" to apply.`, "info");
      input.blur();

      // Highlight the selected preset button
      document.querySelectorAll('.preset-btn').forEach(btn => {
        if (btn.dataset.station === code) {
          btn.classList.add('selected');
        } else {
          btn.classList.remove('selected');
        }
      });

      // Clear line and platform filters when station changes
      const serviceType = document.getElementById('serviceType').value;
      if (serviceType === '1') {
        document.getElementById('tflLineFilter').value = '';
        document.getElementById('tflPlatformFilter').innerHTML = '<option value="">All Platforms</option>';
        document.getElementById('tflPlatformFilter').value = '';
        console.log('Cleared line and platform filters for new station (not yet applied)');

        // Fetch tube lines for the new station (but don't apply yet)
        if (code.length >= 4) {
          setTimeout(() => {
            showTflLineSelector(code.trim());
          }, 100);
        }
      }

      // Note: Don't auto-apply - user must click the Apply Station Settings button

      // Track recent stations
      addToRecentStations(code, name || code);
    };

    // ==================== Form Validation ====================

    const setupValidation = () => {
      const stationInput = document.getElementById("station");
      stationInput.addEventListener("input", (e) => {
        const val = e.target.value.trim();
        const serviceType = document.getElementById('serviceType').value;
        const isTfl = serviceType === '1';

        // National Rail: 3 uppercase letters
        // TFL: 4-12 alphanumeric characters (hub codes or NaPTAN IDs)
        const isValidNationalRail = val.length === 3 && /^[A-Z]{3}$/.test(val);
        const isValidTfl = val.length >= 4 && val.length <= 12 && /^[A-Z0-9]+$/.test(val);

        if ((isTfl && isValidTfl) || (!isTfl && isValidNationalRail)) {
          e.target.classList.add("success");
          e.target.classList.remove("error");
        } else if (val.length > 0) {
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
      }, 1500);
    };

    // ==================== Auto-Apply Settings ====================

    const autoApplySettings = (stationCodeOverride) => {
      const stationInput = document.getElementById('station');
      // Get station code from data attribute if set, otherwise from input value or override
      const stationValue = stationCodeOverride || stationInput.dataset.stationCode || stationInput.value;
      const serviceType = document.getElementById('serviceType').value;
      const isTfl = serviceType === '1';

      // Validate station code before sending
      const isValidNationalRail = stationValue.length === 3 && /^[A-Z]{3}$/.test(stationValue);
      const isValidTfl = stationValue.length >= 4 && stationValue.length <= 12 && /^[A-Z0-9]+$/.test(stationValue);

      if ((isTfl && !isValidTfl) || (!isTfl && !isValidNationalRail)) {
        console.log('autoApplySettings: Skipping - invalid station code:', stationValue);
        showToast("Please select a valid station before applying settings", "warning");
        return;
      }

      const formData = new URLSearchParams();
      console.log('autoApplySettings: stationCodeOverride=', stationCodeOverride, 'station name=', stationInput.value, 'station code=', stationValue);
      formData.append('serviceType', serviceType);
      formData.append('tflApiKey', document.getElementById('tflApiKey').value);
      formData.append('tflLineFilter', document.getElementById('tflLineFilter').value);
      formData.append('tflPlatformFilter', document.getElementById('tflPlatformFilter').value);
      formData.append('station', stationValue);
      console.log('autoApplySettings: Sending station=', stationValue, 'tflLineFilter=', document.getElementById('tflLineFilter').value, 'tflPlatformFilter=', document.getElementById('tflPlatformFilter').value);
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
      .then(async response => {
        if (!response.ok) {
          const errorText = await response.text();
          throw new Error(errorText || `HTTP ${response.status}`);
        }
        return response;
      })
      .then(() => {
        showToast("Settings updated!", "success");

        if (ws && ws.readyState === WebSocket.OPEN) {
          ws.send(JSON.stringify({command: "getState"}));
        }
      })
      .catch(error => {
        console.error('Apply settings error:', error);
        showToast("Failed to apply settings: " + error.message, "error");
      });
    };

    // ==================== TFL Line Selection ====================

    /**
     * Fetch available tube lines and platforms for a TFL station
     */
    const fetchTflStationLines = async (stationId, stationName = '') => {
      console.log('fetchTflStationLines called with:', stationId, stationName);

      const tflApiKey = document.getElementById('tflApiKey')?.value || '';
      // Use Arrivals endpoint to get lines that actually have services at this station
      const apiUrl = `https://api.tfl.gov.uk/StopPoint/${stationId}/Arrivals${tflApiKey ? '?app_key=' + encodeURIComponent(tflApiKey) : ''}`;

      console.log('Fetching arrivals from TFL API:', apiUrl.replace(tflApiKey, 'XXX'));

      try {
        const response = await fetch(apiUrl);
        console.log('Response status:', response.status);

        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }

        const arrivals = await response.json();
        console.log('Received TFL arrivals data:', arrivals.length, 'arrivals');

        // Extract unique tube lines and platforms per line from actual arrivals
        const lineMap = new Map();
        const platformsByLine = new Map();

        arrivals.forEach(arrival => {
          if (arrival.lineId && arrival.lineName && arrival.modeName === 'tube') {
            // Only include tube mode arrivals (excludes Elizabeth line, which uses separate station IDs)
            if (!lineMap.has(arrival.lineId)) {
              lineMap.set(arrival.lineId, arrival.lineName);
              platformsByLine.set(arrival.lineId, new Set());
            }

            // Add platform to this line's set if it exists
            if (arrival.platformName) {
              platformsByLine.get(arrival.lineId).add(arrival.platformName);
            }
          }
        });

        // Convert map to array of objects with platforms
        const tubeLines = Array.from(lineMap.entries()).map(([id, name]) => ({
          id: id,
          name: name,
          platforms: Array.from(platformsByLine.get(id) || []).sort()
        }));

        // Sort alphabetically by name
        tubeLines.sort((a, b) => a.name.localeCompare(b.name));

        console.log('Station has', tubeLines.length, 'tube lines with active services:', tubeLines.map(l => `${l.name} (${l.platforms.length} platforms)`));
        return tubeLines;

      } catch (error) {
        console.error('Error fetching TFL station lines:', error);
        showToast('Failed to fetch tube lines: ' + error.message, 'error');
        return [];
      }
    };

    /**
     * Display tube line selector
     */
    const showTflLineSelector = async (stationId) => {
      const lineFilter = document.getElementById('tflLineFilter');
      const platformFilter = document.getElementById('tflPlatformFilter');

      // Show loading state - disable dropdowns and show "Loading..."
      lineFilter.innerHTML = '<option value="">Loading lines...</option>';
      lineFilter.disabled = true;
      platformFilter.innerHTML = '<option value="">Loading platforms...</option>';
      platformFilter.disabled = true;

      try {
        const lines = await fetchTflStationLines(stationId);

        if (lines.length === 0) {
          // No lines found - reset to default state
          lineFilter.innerHTML = '<option value="">All Lines</option>';
          lineFilter.disabled = false;
          platformFilter.innerHTML = '<option value="">All Platforms</option>';
          platformFilter.disabled = false;
          showToast('No tube lines found for this station', 'warning');
          return;
        }

        // Store lines data globally for platform filtering
        window.tflLinesData = lines;

        // Populate the line filter dropdown
        lineFilter.innerHTML = '<option value="">All Lines</option>' +
          lines.map(line => `<option value="${escapeHtml(line.id)}">${escapeHtml(line.name)}</option>`).join('');
        lineFilter.disabled = false;

        // Reset platform filter and re-enable
        platformFilter.innerHTML = '<option value="">All Platforms</option>';
        platformFilter.disabled = false;

        // Don't auto-select any line - let the user choose
        lineFilter.value = '';

        // Show toast with available lines
        const lineNames = lines.map(l => l.name).join(', ');
        showToast(`Available lines: ${lineNames}`, 'info');
      } catch (error) {
        // Error handling - reset to default state
        lineFilter.innerHTML = '<option value="">All Lines</option>';
        lineFilter.disabled = false;
        platformFilter.innerHTML = '<option value="">All Platforms</option>';
        platformFilter.disabled = false;
      }
    };

    // ==================== Advanced Controls Toggle ====================

    // ==================== Device Uptime Counter ====================

    let deviceStartTime = Date.now();

    /**
     * Update the device uptime display
     */
    function updateUptime() {
      const uptime = Date.now() - deviceStartTime;
      const seconds = Math.floor(uptime / 1000);
      const minutes = Math.floor(seconds / 60);
      const hours = Math.floor(minutes / 60);
      const days = Math.floor(hours / 24);

      let uptimeStr = '';
      if (days > 0) {
        uptimeStr = `${days}d ${hours % 24}h ${minutes % 60}m`;
      } else if (hours > 0) {
        uptimeStr = `${hours}h ${minutes % 60}m ${seconds % 60}s`;
      } else if (minutes > 0) {
        uptimeStr = `${minutes}m ${seconds % 60}s`;
      } else {
        uptimeStr = `${seconds}s`;
      }

      const uptimeEl = document.getElementById('deviceUptime');
      if (uptimeEl) {
        uptimeEl.textContent = uptimeStr;
      }
    }

    // Update uptime every second
    setInterval(updateUptime, 1000);

    /**
     * Update the last fetch time display
     */
    let lastFetchTimestamp = null;

    function updateLastFetchTime() {
      lastFetchTimestamp = Date.now();
      const lastFetchEl = document.getElementById('lastFetchTime');
      if (lastFetchEl) {
        lastFetchEl.textContent = 'Just now';
      }
    }

    // Update "time ago" display for last fetch
    setInterval(() => {
      if (!lastFetchTimestamp) return;

      const lastFetchEl = document.getElementById('lastFetchTime');
      if (!lastFetchEl) return;

      const secondsAgo = Math.floor((Date.now() - lastFetchTimestamp) / 1000);

      if (secondsAgo < 60) {
        lastFetchEl.textContent = secondsAgo < 10 ? 'Just now' : `${secondsAgo}s ago`;
      } else if (secondsAgo < 3600) {
        const minutesAgo = Math.floor(secondsAgo / 60);
        lastFetchEl.textContent = `${minutesAgo}m ago`;
      } else {
        const hoursAgo = Math.floor(secondsAgo / 3600);
        lastFetchEl.textContent = `${hoursAgo}h ago`;
      }
    }, 1000);

    // ==================== Preset Button Highlighting ====================

    /**
     * Highlight the active preset button based on station code
     */
    function highlightActivePresetButton(stationCode) {
      if (!stationCode) return;

      // Normalize the station code for comparison
      const normalizedCode = stationCode.trim().toUpperCase();

      document.querySelectorAll('.preset-btn[data-station]').forEach(btn => {
        const btnCode = btn.dataset.station.trim().toUpperCase();
        if (btnCode === normalizedCode) {
          btn.classList.add('selected');
        } else {
          btn.classList.remove('selected');
        }
      });
    }

    // ==================== Recent Stations Management ====================

    /**
     * Add station to recent stations list
     */
    function addToRecentStations(code, name) {
      try {
        let recent = JSON.parse(localStorage.getItem('recentStations') || '[]');
        // Remove duplicates
        recent = recent.filter(s => s.code !== code);
        // Add to front
        recent.unshift({ code, name, timestamp: Date.now() });
        // Keep last 6
        recent = recent.slice(0, 6);
        localStorage.setItem('recentStations', JSON.stringify(recent));
      } catch (e) {
        console.error('Error saving recent stations:', e);
      }
    }

    /**
     * Load and display recent stations
     */
    function loadRecentStations() {
      const container = document.getElementById('presets-recent');
      if (!container) return;

      try {
        const recent = JSON.parse(localStorage.getItem('recentStations') || '[]');

        if (recent.length === 0) {
          container.innerHTML = '<div style="text-align: center; padding: 40px; color: #999; font-size: 14px;">No recently used stations</div>';
          return;
        }

        const stationButtons = recent.map(station =>
          `<button type="button" class="preset-btn" data-station="${escapeHtml(station.code)}" aria-label="Select ${escapeHtml(station.name || station.code)} station">
            ${escapeHtml(station.name || station.code)}
          </button>`
        ).join('');

        // Add Clear All button
        const clearButton = `
          <button type="button" class="preset-btn" id="clearRecentBtn" style="background: #dc3545; color: white; border-color: #dc3545;" aria-label="Clear all recent stations">
            🗑️ Clear All
          </button>
        `;

        container.innerHTML = stationButtons + clearButton;

        // Re-attach click handlers for station buttons
        container.querySelectorAll('.preset-btn[data-station]').forEach(btn => {
          btn.addEventListener('click', () => setStation(btn.dataset.station));
        });

        // Attach handler for Clear All button
        const clearBtn = document.getElementById('clearRecentBtn');
        if (clearBtn) {
          clearBtn.addEventListener('click', () => {
            if (confirm('Clear all recently used stations?')) {
              localStorage.removeItem('recentStations');
              loadRecentStations(); // Reload to show empty state
              showToast('Recent stations cleared', 'success');
            }
          });
        }
      } catch (e) {
        console.error('Error loading recent stations:', e);
        container.innerHTML = '<div style="text-align: center; padding: 40px; color: #999; font-size: 14px;">Error loading recent stations</div>';
      }
    }

    /**
     * Display platform selector for a selected tube line
     */
    const showTflPlatformSelector = (lineId) => {
      const platformFilter = document.getElementById('tflPlatformFilter');

      // Clear platform filter first
      platformFilter.innerHTML = '<option value="">All Platforms</option>';
      platformFilter.value = '';

      // If no line selected or no lines data, return
      if (!lineId || !window.tflLinesData) {
        return;
      }

      // Find the selected line
      const selectedLine = window.tflLinesData.find(line => line.id === lineId);

      if (!selectedLine || !selectedLine.platforms || selectedLine.platforms.length === 0) {
        console.log('No platforms found for line:', lineId);
        return;
      }

      // Populate platform dropdown
      platformFilter.innerHTML = '<option value="">All Platforms</option>' +
        selectedLine.platforms.map(platform =>
          `<option value="${escapeHtml(platform)}">${escapeHtml(platform)}</option>`
        ).join('');

      console.log('Populated', selectedLine.platforms.length, 'platforms for', selectedLine.name);
    };

    // ==================== Initialization ====================

    document.addEventListener("DOMContentLoaded", () => {
      connectWebSocket();
      loadStationData();
      setupStationAutocomplete();
      setupValidation();

      // Initialize uptime counter
      updateUptime();

      // Setup Refresh Now button
      const refreshNowBtn = document.getElementById('refreshNowBtn');
      if (refreshNowBtn) {
        refreshNowBtn.addEventListener('click', () => {
          if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({command: "refresh"}));
            showToast("Refreshing departure data...", "info");
          } else {
            showToast("Not connected to device", "error");
          }
        });
      }

      // Setup auto-apply for scroll speed dropdown
      const scrollspeedSelect = document.getElementById('scrollspeed');
      if (scrollspeedSelect) {
        scrollspeedSelect.addEventListener('change', () => {
          autoApplySettings();
        });
      }

      // Setup auto-apply for rotation speed input
      const rotationspeedInput = document.getElementById('rotationspeed');
      if (rotationspeedInput) {
        rotationspeedInput.addEventListener('input', () => {
          autoApplySettings();
        });
      }

      // Setup preset category tabs
      document.querySelectorAll('.preset-category-tab').forEach(tab => {
        tab.addEventListener('click', function() {
          // Update active tab
          document.querySelectorAll('.preset-category-tab').forEach(t => t.classList.remove('active'));
          this.classList.add('active');

          // Show corresponding presets
          const category = this.dataset.category;
          document.querySelectorAll('.preset-stations').forEach(group => {
            group.style.display = group.id === `presets-${category}` ? 'grid' : 'none';
          });

          // Load recent stations if needed
          if (category === 'recent') {
            loadRecentStations();
          }
        });
      });

      setupForms();

      // Service type selection handler
      const serviceTypeSelect = document.getElementById("serviceType");
      const tflApiKeyGroup = document.getElementById("tflApiKeyGroup");
      const stationLabel = document.getElementById("stationLabel");
      const stationTooltip = document.getElementById("stationTooltip");

      const updateServiceTypeUI = () => {
        const isUnderground = serviceTypeSelect.value === "1";
        const nationalRailPresets = document.getElementById("nationalRailPresets");
        const tflPresets = document.getElementById("tflPresets");
        const tflLineFilterGroup = document.getElementById("tflLineFilterGroup");
        const tflPlatformFilterGroup = document.getElementById("tflPlatformFilterGroup");
        const modeSelect = document.getElementById("mode");

        // Show/hide appropriate elements
        tflApiKeyGroup.style.display = isUnderground ? "block" : "none";
        tflLineFilterGroup.style.display = isUnderground ? "block" : "none";
        tflPlatformFilterGroup.style.display = isUnderground ? "block" : "none";
        nationalRailPresets.style.display = isUnderground ? "none" : "grid";
        tflPresets.style.display = isUnderground ? "grid" : "none";

        // Disable "Calling At Mode" for TFL (option value="1")
        const callingAtOption = modeSelect.querySelector('option[value="1"]');
        if (callingAtOption) {
          callingAtOption.disabled = isUnderground;
          // If currently on Calling At mode and switching to TFL, change to Standard View
          if (isUnderground && modeSelect.value === "1") {
            modeSelect.value = "0";
          }
        }

        // Update labels
        if (isUnderground) {
          stationLabel.textContent = "TFL Station";
          stationTooltip.title = "TFL Station code (e.g., HUBSOK for South Kensington or 940GZZLUPAC for Piccadilly Circus)";
        } else {
          stationLabel.textContent = "Station Code (CRS)";
          stationTooltip.title = "Three-letter National Rail station code";
        }
      };

      serviceTypeSelect.addEventListener("change", () => {
        updateServiceTypeUI();
        // Note: Don't auto-apply - user must click Apply button

        // If switching to TFL and a station is already entered, fetch tube lines
        const stationInput = document.getElementById("station");
        if (serviceTypeSelect.value === "1" && stationInput.value.length >= 4) {
          setTimeout(() => {
            showTflLineSelector(stationInput.value.trim());
          }, 500);
        }
      });

      // Initialize UI on load
      updateServiceTypeUI();

      // Update current station display
      const stationInput = document.getElementById("station");
      if (stationInput.value) {
        document.getElementById("currentStation").textContent = stationInput.value;
      }

      // Auto-apply for display options only (station settings require Apply button)
      const autoApplyFields = ["interval", "mode", "showstation", "extra", "rotationspeed", "scrollspeed", "ytop", "y1", "y2", "y3"];
      autoApplyFields.forEach(fieldId => {
        const field = document.getElementById(fieldId);
        if (field) {
          field.addEventListener("change", () => {
            autoApplySettings();
          });
        }
      });

      // Handle TFL line filter changes - populate platform dropdown but don't auto-apply
      const tflLineFilter = document.getElementById('tflLineFilter');
      if (tflLineFilter) {
        tflLineFilter.addEventListener("change", () => {
          const lineId = tflLineFilter.value;

          // Clear and repopulate platform dropdown when line changes
          if (lineId) {
            showTflPlatformSelector(lineId);
          } else {
            // Clear platform filter if no line selected
            const platformFilter = document.getElementById('tflPlatformFilter');
            platformFilter.innerHTML = '<option value="">All Platforms</option>';
            platformFilter.value = '';
          }
          // Note: Don't auto-apply - wait for user to click Apply button
        });
      }

      // Apply Station Settings button handler
      document.getElementById('applyStationBtn').addEventListener('click', () => {
        const stationInput = document.getElementById('station');

        if (stationInput.value.length < 3) {
          showToast("Station code must be at least 3 characters", "error");
          return;
        }

        autoApplySettings();

        // Note: Don't refetch lines - they're already loaded when station was selected
        // Refetching would clear the user's line and platform selections
      });

      // Tab button event listeners
      document.querySelectorAll('.tab-button').forEach((btn, index) => {
        btn.addEventListener('click', () => switchTab(index));
      });

      // Preset station buttons
      document.querySelectorAll('.preset-btn').forEach(btn => {
        btn.addEventListener('click', () => setStation(btn.dataset.station, btn.dataset.name));
      });

      // Live Preview collapsible toggle
      const previewHeader = document.getElementById('previewHeader');
      const previewContent = document.getElementById('previewContent');

      if (previewHeader && previewContent) {
        // Load saved state from localStorage
        const isCollapsed = localStorage.getItem('previewCollapsed') === 'true';
        if (isCollapsed) {
          previewContent.classList.add('collapsed');
          previewHeader.setAttribute('aria-expanded', 'false');
        }

        // Toggle function
        const togglePreview = () => {
          const isCurrentlyCollapsed = previewContent.classList.contains('collapsed');

          if (isCurrentlyCollapsed) {
            previewContent.classList.remove('collapsed');
            previewHeader.setAttribute('aria-expanded', 'true');
            localStorage.setItem('previewCollapsed', 'false');
          } else {
            previewContent.classList.add('collapsed');
            previewHeader.setAttribute('aria-expanded', 'false');
            localStorage.setItem('previewCollapsed', 'true');
          }
        };

        // Add click handler
        previewHeader.addEventListener('click', togglePreview);

        // Add keyboard support (Enter or Space)
        previewHeader.addEventListener('keydown', (e) => {
          if (e.key === 'Enter' || e.key === ' ') {
            e.preventDefault();
            togglePreview();
          }
        });
      }

      // Reset button
      document.getElementById('resetButton').addEventListener('click', showResetModal);
      document.getElementById('cancelResetBtn').addEventListener('click', hideResetModal);
      document.getElementById('confirmResetBtn').addEventListener('click', confirmReset);

      // Scan networks button
      document.getElementById('scanNetworksBtn').addEventListener('click', scanNetworks);

      // Update firmware info in footer
      const firmwareInfo = document.getElementById('firmwareInfo');
      if (firmwareInfo) {
        firmwareInfo.textContent = 'v2.1.0';
      }

      // Highlight the active preset button based on initial station value
      const stationInput = document.getElementById('station');
      if (stationInput && stationInput.value) {
        // Use stationCode from dataset if available, otherwise use the input value
        const initialStationCode = stationInput.dataset.stationCode || stationInput.value;
        highlightActivePresetButton(initialStationCode);
      }
    });
  </script>

  <!-- Footer -->
  <div style="text-align: center; padding: 20px; color: #999; font-size: 12px; background: rgba(255,255,255,0.5); margin-top: 20px;">
    <div style="margin-bottom: 5px;">
      <strong>StationBoards</strong> • Firmware <span id="firmwareInfo">v2.1.0</span> • IP <span id="deviceIP">{IP}</span>
    </div>
    <div>
      Device ID: <span id="deviceID">{DEVICE_ID}</span>
    </div>
  </div>

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

#endif
