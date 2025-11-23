// ==================== Configuration & Constants ====================
const CONFIG = {
  TOAST_DURATION: 5000,
  DEBOUNCE_DELAY: 300,
  RECONNECT_DELAYS: [1000, 2000, 5000, 10000, 30000], // Exponential backoff
  TIME_UNITS: {
    MINUTE: 60000,
    HOUR: 3600000,
    DAY: 86400000
  },
  VALIDATION: {
    STATION_CODE_PATTERN: /^[A-Z]{3}$/,
    STATION_CODE_MAX_LENGTH: 3,
    REFRESH_INTERVAL_MIN: 30,
    REFRESH_INTERVAL_MAX: 300,
    SCROLL_SPEED_MIN: 10,
    SCROLL_SPEED_MAX: 200,
    ROTATION_SPEED_MIN: 5,
    ROTATION_SPEED_MAX: 60
  }
};

// ==================== Application State ====================
const AppState = {
  devices: new Map(), // Using Map for O(1) lookups
  currentDeviceId: null,
  firmwareList: [],
  serverInfo: null,
  reconnectAttempts: 0,
  isReconnecting: false,
  activeRequests: new Set() // Track in-flight requests
};

// ==================== Socket Connection ====================
let socket = null;

// ==================== HTML Sanitization ====================
/**
 * Escapes HTML to prevent XSS attacks
 */
function escapeHtml(unsafe) {
  if (unsafe === null || unsafe === undefined) return '';
  return String(unsafe)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');
}

/**
 * Safely escape attribute values
 */
function escapeAttr(unsafe) {
  if (unsafe === null || unsafe === undefined) return '';
  return String(unsafe)
    .replace(/&/g, '&amp;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');
}

// ==================== Utility Functions ====================
/**
 * Debounce function to limit how often a function can fire
 */
function debounce(func, wait) {
  let timeout;
  return function executedFunction(...args) {
    const later = () => {
      clearTimeout(timeout);
      func(...args);
    };
    clearTimeout(timeout);
    timeout = setTimeout(later, wait);
  };
}

/**
 * Format last seen timestamp
 */
function formatLastSeen(timestamp) {
  if (!timestamp) return 'Never';
  const date = new Date(timestamp * 1000);
  const now = Date.now();
  const diff = now - date.getTime();

  if (diff < CONFIG.TIME_UNITS.MINUTE) return 'Just now';
  if (diff < CONFIG.TIME_UNITS.HOUR) return `${Math.floor(diff / CONFIG.TIME_UNITS.MINUTE)}m ago`;
  if (diff < CONFIG.TIME_UNITS.DAY) return `${Math.floor(diff / CONFIG.TIME_UNITS.HOUR)}h ago`;
  return `${Math.floor(diff / CONFIG.TIME_UNITS.DAY)}d ago`;
}

/**
 * Format timestamp to locale string
 */
function formatTimestamp(timestamp) {
  if (!timestamp) return 'N/A';
  const date = new Date(timestamp * 1000);
  return date.toLocaleString();
}

/**
 * Format uptime in seconds to readable string
 */
function formatUptime(seconds) {
  if (!seconds) return 'N/A';
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);

  if (days > 0) return `${days}d ${hours}h`;
  if (hours > 0) return `${hours}h ${minutes}m`;
  return `${minutes}m`;
}

/**
 * Format bytes to human-readable string
 */
function formatBytes(bytes) {
  if (!bytes) return 'N/A';
  if (bytes < 1024) return bytes + ' B';
  if (bytes < 1048576) return (bytes / 1024).toFixed(1) + ' KB';
  return (bytes / 1048576).toFixed(1) + ' MB';
}

/**
 * Get RSSI icon based on signal strength
 */
function getRSSIIcon(rssi) {
  if (!rssi) return '<i class="bi bi-wifi text-muted" aria-label="No signal"></i>';
  if (rssi > -50) return '<i class="bi bi-wifi rssi-excellent" aria-label="Excellent signal"></i>';
  if (rssi > -60) return '<i class="bi bi-wifi rssi-good" aria-label="Good signal"></i>';
  if (rssi > -70) return '<i class="bi bi-wifi rssi-fair" aria-label="Fair signal"></i>';
  return '<i class="bi bi-wifi rssi-poor" aria-label="Poor signal"></i>';
}

/**
 * Show toast notification
 */
function showToast(message, type = 'info') {
  const toast = document.createElement('div');
  toast.className = `alert alert-${type} alert-dismissible fade show`;
  toast.setAttribute('role', 'alert');
  toast.style.cssText = 'position: fixed; top: 70px; right: 20px; z-index: 9999; min-width: 300px;';
  toast.innerHTML = `
    ${escapeHtml(message)}
    <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
  `;
  document.body.appendChild(toast);

  setTimeout(() => {
    toast.remove();
  }, CONFIG.TOAST_DURATION);
}

/**
 * Show loading state on element
 */
function setLoadingState(element, isLoading, originalText = '') {
  if (isLoading) {
    element.disabled = true;
    element.dataset.originalText = element.innerHTML;
    element.innerHTML = '<span class="spinner-border spinner-border-sm me-2" role="status" aria-hidden="true"></span>Loading...';
  } else {
    element.disabled = false;
    element.innerHTML = element.dataset.originalText || originalText;
  }
}

// ==================== Validation Functions ====================
/**
 * Validate station code
 */
function validateStationCode(code) {
  if (!code || code.trim() === '') {
    return { valid: false, error: 'Station code is required' };
  }
  if (!CONFIG.VALIDATION.STATION_CODE_PATTERN.test(code.toUpperCase())) {
    return { valid: false, error: 'Station code must be exactly 3 uppercase letters' };
  }
  return { valid: true };
}

/**
 * Validate number range
 */
function validateRange(value, min, max, name) {
  const num = parseInt(value);
  if (isNaN(num)) {
    return { valid: false, error: `${name} must be a number` };
  }
  if (num < min || num > max) {
    return { valid: false, error: `${name} must be between ${min} and ${max}` };
  }
  return { valid: true, value: num };
}

/**
 * Show validation error on input
 */
function showValidationError(inputId, message) {
  const input = document.getElementById(inputId);
  if (!input) return;

  input.classList.add('is-invalid');

  // Remove existing feedback
  const existingFeedback = input.parentElement.querySelector('.invalid-feedback');
  if (existingFeedback) {
    existingFeedback.remove();
  }

  // Add new feedback
  const feedback = document.createElement('div');
  feedback.className = 'invalid-feedback';
  feedback.textContent = message;
  input.parentElement.appendChild(feedback);
}

/**
 * Clear validation error on input
 */
function clearValidationError(inputId) {
  const input = document.getElementById(inputId);
  if (!input) return;

  input.classList.remove('is-invalid');
  const feedback = input.parentElement.querySelector('.invalid-feedback');
  if (feedback) {
    feedback.remove();
  }
}

// ==================== WebSocket Management ====================
/**
 * Initialize WebSocket connection with reconnection support
 */
function setupWebSocket() {
  socket = io({
    reconnection: true,
    reconnectionDelay: 1000,
    reconnectionDelayMax: 5000,
    reconnectionAttempts: Infinity
  });

  socket.on('connect', () => {
    console.log('Connected to server');
    AppState.reconnectAttempts = 0;
    AppState.isReconnecting = false;
    showToast('Connected to monitoring server', 'success');
    loadDevices(); // Refresh devices on reconnect
  });

  socket.on('disconnect', () => {
    console.log('Disconnected from server');
    AppState.isReconnecting = true;
    showToast('Disconnected from server. Reconnecting...', 'warning');
  });

  socket.on('connect_error', (error) => {
    console.error('Connection error:', error);
    AppState.reconnectAttempts++;

    if (AppState.reconnectAttempts > 5) {
      showToast('Unable to connect to server. Please refresh the page.', 'danger');
    }
  });

  socket.on('deviceUpdate', (data) => {
    console.log('Device update:', data);
    updateDeviceInList(data);
  });

  socket.on('configResponse', (data) => {
    console.log('Config response from device:', data);

    // Update config form if modal is open for this device
    if (AppState.currentDeviceId === data.deviceId) {
      updateConfigForm(data.config);
      showToast('Configuration synced from device', 'success');
    }
  });

  socket.on('logUpdate', (data) => {
    // Only display logs for currently viewed device
    if (AppState.currentDeviceId === data.deviceId) {
      appendLiveLog(data);
    }
  });

  socket.on('reconnect', (attemptNumber) => {
    console.log('Reconnected after', attemptNumber, 'attempts');
    showToast('Reconnected to server', 'success');
    loadDevices(); // Refresh devices after reconnection
  });
}

// ==================== Server Info ====================
/**
 * Load server information
 */
async function loadServerInfo() {
  try {
    const response = await fetch('/api/server-info');
    AppState.serverInfo = await response.json();
    console.log('Server info loaded:', AppState.serverInfo);
  } catch (err) {
    console.error('Error loading server info:', err);
    // Fallback to window.location.origin
    AppState.serverInfo = {
      baseUrl: window.location.origin
    };
  }
}

// ==================== Device Management ====================
/**
 * Load all devices
 */
async function loadDevices() {
  try {
    const response = await fetch('/api/devices');
    const deviceArray = await response.json();

    // Update devices Map
    AppState.devices.clear();
    deviceArray.forEach(device => {
      AppState.devices.set(device.id, device);
    });

    renderDevices();
    updateCounts();
  } catch (err) {
    console.error('Error loading devices:', err);
    showToast('Error loading devices', 'danger');
  }
}

/**
 * Render devices grid with incremental updates
 */
function renderDevices() {
  const grid = document.getElementById('devicesGrid');
  const searchTerm = document.getElementById('searchInput').value.toLowerCase();
  const statusFilter = document.getElementById('statusFilter').value;

  const deviceArray = Array.from(AppState.devices.values());
  const filtered = deviceArray.filter(device => {
    const matchesSearch = device.name.toLowerCase().includes(searchTerm) ||
                         device.id.toLowerCase().includes(searchTerm) ||
                         (device.station_code && device.station_code.toLowerCase().includes(searchTerm));
    const matchesStatus = statusFilter === 'all' || device.status === statusFilter;
    return matchesSearch && matchesStatus;
  });

  if (filtered.length === 0) {
    grid.innerHTML = `
      <div class="col-12">
        <div class="empty-state">
          <i class="bi bi-inbox" aria-hidden="true"></i>
          <h4>No devices found</h4>
          <p>No devices match your search criteria</p>
        </div>
      </div>
    `;
    return;
  }

  // Build new grid
  grid.innerHTML = filtered.map(device => createDeviceCard(device)).join('');

  // Add event listeners to device cards
  filtered.forEach(device => {
    const card = grid.querySelector(`[data-device-id="${escapeAttr(device.id)}"]`);
    if (card) {
      card.addEventListener('click', () => showDeviceDetails(device.id));
      card.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          showDeviceDetails(device.id);
        }
      });
    }
  });
}

/**
 * Create device card HTML (XSS safe)
 */
function createDeviceCard(device) {
  const statusClass = device.status === 'online' ? 'online' : 'offline';
  const statusBadge = device.status === 'online'
    ? '<span class="badge bg-success status-badge">Online</span>'
    : '<span class="badge bg-secondary status-badge">Offline</span>';

  const lastSeen = formatLastSeen(device.last_seen);
  const rssiIcon = getRSSIIcon(device.rssi);
  const uptime = formatUptime(device.uptime);

  return `
    <div class="col-12 col-md-6 col-lg-4 col-xl-3 mb-3">
      <div class="card device-card shadow-sm ${statusClass}"
           data-device-id="${escapeAttr(device.id)}"
           role="button"
           tabindex="0"
           aria-label="Device ${escapeAttr(device.name)}">
        <div class="card-header bg-white">
          <div>
            <h6 class="mb-0">${escapeHtml(device.name)}</h6>
            <small class="text-muted">${escapeHtml(device.station_code || 'N/A')}</small>
          </div>
          ${statusBadge}
        </div>
        <div class="card-body">
          <div class="device-icon text-center" aria-hidden="true">
            <i class="bi bi-display"></i>
          </div>

          <div class="metric-item">
            <span class="metric-label">Station:</span>
            <span class="metric-value">${escapeHtml(device.station_name || 'Unknown')}</span>
          </div>

          <div class="metric-item">
            <span class="metric-label">Service:</span>
            <span class="metric-value">${escapeHtml(device.service_type || 'National Rail')}</span>
          </div>

          <div class="metric-item">
            <span class="metric-label">Signal:</span>
            <span class="metric-value">${rssiIcon} ${escapeHtml(device.rssi || 0)} dBm</span>
          </div>

          <div class="metric-item">
            <span class="metric-label">Services:</span>
            <span class="metric-value">${escapeHtml(device.services_count || 0)}</span>
          </div>

          <div class="metric-item">
            <span class="metric-label">Last Seen:</span>
            <span class="metric-value">${escapeHtml(lastSeen)}</span>
          </div>

          <div class="metric-item">
            <span class="metric-label">Firmware:</span>
            <span class="metric-value">${escapeHtml(device.firmware_version || 'Unknown')}</span>
          </div>
        </div>
      </div>
    </div>
  `;
}

/**
 * Update single device in list
 */
function updateDeviceInList(data) {
  const deviceId = data.deviceId;

  if (AppState.devices.has(deviceId)) {
    // Update existing device
    const existing = AppState.devices.get(deviceId);
    AppState.devices.set(deviceId, { ...existing, ...data, id: deviceId });
  } else {
    // Add new device
    AppState.devices.set(deviceId, { id: deviceId, ...data });
  }

  renderDevices();
  updateCounts();

  // Update modal if it's open for this device
  if (AppState.currentDeviceId === deviceId) {
    loadDeviceDetails(deviceId);
  }
}

/**
 * Update device counts in navbar
 */
function updateCounts() {
  const deviceArray = Array.from(AppState.devices.values());
  document.getElementById('deviceCount').textContent = deviceArray.length;
  const onlineCount = deviceArray.filter(d => d.status === 'online').length;
  document.getElementById('onlineCount').textContent = `${onlineCount} Online`;
}

/**
 * Refresh devices manually
 */
function refreshDevices() {
  loadDevices();
  showToast('Refreshing devices...', 'info');
}

// ==================== Device Details Modal ====================
/**
 * Show device details modal
 */
async function showDeviceDetails(deviceId) {
  AppState.currentDeviceId = deviceId;

  const modal = new bootstrap.Modal(document.getElementById('deviceModal'));
  modal.show();

  await loadDeviceDetails(deviceId);
}

/**
 * Load device details
 */
async function loadDeviceDetails(deviceId) {
  try {
    const response = await fetch(`/api/devices/${deviceId}`);
    const data = await response.json();
    populateDeviceModal(data);
  } catch (err) {
    console.error('Error loading device details:', err);
    showToast('Error loading device details', 'danger');
  }
}

/**
 * Populate device modal with data (XSS safe)
 */
function populateDeviceModal(data) {
  const { device, config, logs } = data;

  // Set modal title
  document.getElementById('modalDeviceName').textContent = device.name;
  const statusBadge = document.getElementById('modalDeviceStatus');
  statusBadge.textContent = device.status;
  statusBadge.className = `badge ms-2 ${device.status === 'online' ? 'bg-success' : 'bg-secondary'}`;

  // Info tab
  document.getElementById('infoDeviceId').textContent = device.id;
  document.getElementById('infoIpAddress').textContent = device.ip_address || 'Unknown';
  document.getElementById('infoFirmware').textContent = device.firmware_version || 'Unknown';
  document.getElementById('infoStation').textContent = `${device.station_code || 'N/A'} - ${device.station_name || 'Unknown'}`;
  document.getElementById('infoServiceType').textContent = device.service_type || 'National Rail';
  document.getElementById('infoLastSeen').textContent = formatLastSeen(device.last_seen);
  document.getElementById('infoRSSI').innerHTML = `${getRSSIIcon(device.rssi)} ${device.rssi || 0} dBm`;
  document.getElementById('infoUptime').textContent = formatUptime(device.uptime);
  document.getElementById('infoHeap').textContent = formatBytes(device.free_heap);
  document.getElementById('infoServices').textContent = device.services_count || 0;

  // Config tab
  if (config) {
    document.getElementById('configDeviceId').value = device.id;
    document.getElementById('configServiceType').value = config.service_type || 'National Rail';
    document.getElementById('configStationCode').value = config.station_code || '';
    document.getElementById('configUseCallingAt').value = config.use_calling_at ? '1' : '0';
    document.getElementById('configShowStationName').value = config.show_station_name ? '1' : '0';
    document.getElementById('configExtraServices').value = config.extra_services || 0;

    // Apply service-type specific UI adjustments
    const isTFL = config.service_type === 'TFL';

    console.log('Populating config - Service type:', config.service_type, 'isTFL:', isTFL);

    // Update station label based on service type
    document.getElementById('configStationLabel').textContent = isTFL ? 'Station NaPTAN Code' : 'Station Code (CRS)';

    // Show/hide TFL filter fields
    const lineFilterGroup = document.getElementById('configTflLineFilterGroup');
    const platformFilterGroup = document.getElementById('configTflPlatformFilterGroup');

    if (lineFilterGroup) lineFilterGroup.style.display = isTFL ? 'block' : 'none';
    if (platformFilterGroup) platformFilterGroup.style.display = isTFL ? 'block' : 'none';

    if (isTFL) {
      // Fetch TFL lines and platforms from API if we have a station code
      if (config.station_code) {
        populateTflLines(config.station_code, config.tfl_line_filter || '', config.tfl_platform_filter || '');
      } else {
        // No station code, just set the values if they exist
        document.getElementById('configTflLineFilter').value = config.tfl_line_filter || '';
        document.getElementById('configTflPlatformFilter').value = config.tfl_platform_filter || '';
      }
    }

    // Disable "Calling At" mode for TFL services (not supported)
    const callingAtSelect = document.getElementById('configUseCallingAt');
    const callingAtWrapper = callingAtSelect.closest('.mb-3');

    if (isTFL) {
      callingAtSelect.disabled = true;
      callingAtSelect.value = '0'; // Force to Standard mode

      // Add visual indication
      if (!callingAtWrapper.querySelector('.form-text')) {
        const helpText = document.createElement('small');
        helpText.className = 'form-text text-muted';
        helpText.textContent = 'Calling At mode not available for TFL services';
        callingAtWrapper.appendChild(helpText);
      }
    } else {
      callingAtSelect.disabled = false;
      // Remove help text if it exists
      const helpText = callingAtWrapper.querySelector('.form-text');
      if (helpText) helpText.remove();
    }
  }

  // Logs tab
  const logsTable = document.getElementById('logsTableBody');
  if (logs && logs.length > 0) {
    logsTable.innerHTML = logs.map(log => `
      <tr class="log-event ${escapeAttr(log.event_type)}">
        <td>${escapeHtml(formatTimestamp(log.timestamp))}</td>
        <td><span class="badge bg-secondary">${escapeHtml(log.event_type)}</span></td>
        <td>${escapeHtml(log.message)}</td>
      </tr>
    `).join('');
  } else {
    logsTable.innerHTML = '<tr><td colspan="3" class="text-center">No logs available</td></tr>';
  }
}

// ==================== Configuration ====================
/**
 * Save device configuration with validation
 */
async function saveDeviceConfig(event) {
  event.preventDefault();

  const deviceId = document.getElementById('configDeviceId').value;
  const saveButton = event.submitter;

  // Prevent duplicate submissions
  if (AppState.activeRequests.has('saveConfig')) {
    return;
  }

  // Get all configuration values
  const serviceType = document.getElementById('configServiceType').value;
  const stationCode = document.getElementById('configStationCode').value.trim().toUpperCase();
  const isTFL = serviceType === 'TFL';

  // Build config object
  const config = {
    serviceType: serviceType,
    stationCode: stationCode,
    useCallingAt: document.getElementById('configUseCallingAt').value === '1',
    showStationName: document.getElementById('configShowStationName').value === '1',
    extraServices: parseInt(document.getElementById('configExtraServices').value)
  };

  // Add TFL filters if applicable
  if (isTFL) {
    config.tflLineFilter = document.getElementById('configTflLineFilter').value;
    config.tflPlatformFilter = document.getElementById('configTflPlatformFilter').value;
  } else {
    config.tflLineFilter = '';
    config.tflPlatformFilter = '';
  }

  // Validate station code
  if (!stationCode) {
    showToast('Station code is required', 'warning');
    return;
  }

  try {
    AppState.activeRequests.add('saveConfig');
    setLoadingState(saveButton, true);

    const response = await fetch(`/api/devices/${deviceId}/config`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(config)
    });

    if (response.ok) {
      showToast('Configuration saved successfully', 'success');

      // Refresh the device list to show updated station code
      await loadDevices();

      // If the modal is still open, refresh it too
      if (AppState.currentDeviceId === deviceId) {
        await loadDeviceDetails(deviceId);
      }
    } else {
      throw new Error('Failed to save configuration');
    }
  } catch (err) {
    console.error('Error saving config:', err);
    showToast('Error saving configuration', 'danger');
  } finally {
    AppState.activeRequests.delete('saveConfig');
    setLoadingState(saveButton, false);
  }
}

/**
 * Sync config from device (read actual device settings)
 */
async function syncConfigFromDevice() {
  if (!AppState.currentDeviceId) {
    showToast('No device selected', 'warning');
    return;
  }

  try {
    const response = await fetch(`/api/devices/${AppState.currentDeviceId}/getConfig`, {
      method: 'POST'
    });

    if (response.ok) {
      showToast('Requesting config from device...', 'info');
      // Config will be updated via socket.io when device responds
    } else {
      throw new Error('Device not connected');
    }
  } catch (err) {
    console.error('Error syncing config:', err);
    showToast('Error: Device not connected', 'danger');
  }
}

/**
 * Common TFL station codes for reference
 */
const TFL_EXAMPLES = {
  '940GZZLUPAC': 'Piccadilly Circus',
  '940GZZLULVT': 'Liverpool Street',
  '940GZZLUKSX': 'Kings Cross St Pancras',
  '940GZZLUWSM': 'Westminster',
  '940GZZLUBND': 'Bond Street',
  '940GZZLUOXC': 'Oxford Circus'
};

/**
 * Fetch available tube lines and platforms from TFL API for a station
 */
async function fetchTflStationLines(stationId) {
  console.log('fetchTflStationLines called with:', stationId);

  // Use Arrivals endpoint to get lines that actually have services at this station
  const apiUrl = `https://api.tfl.gov.uk/StopPoint/${stationId}/Arrivals`;

  console.log('Fetching arrivals from TFL API:', apiUrl);

  try {
    const response = await fetch(apiUrl);
    console.log('Response status:', response.status);

    if (!response.ok) {
      if (response.status === 404) {
        const examples = Object.entries(TFL_EXAMPLES).slice(0, 3).map(([code, name]) => `${code} (${name})`).join(', ');
        throw new Error(`Station '${stationId}' not found. Use a TFL NaPTAN code like: ${examples}`);
      }
      throw new Error(`HTTP ${response.status}: ${response.statusText}`);
    }

    const arrivals = await response.json();
    console.log(`Received ${arrivals.length} arrivals`);

    if (arrivals.length === 0) {
      console.warn('No arrivals found for station:', stationId);
      throw new Error(`No tube services found for '${stationId}'. The station may not be served by Underground lines.`);
    }

    // Extract unique lines and platforms from arrivals
    const linesByIdMap = new Map();
    const platformsByLine = new Map();

    arrivals.forEach(arrival => {
      const lineId = arrival.lineId;
      const lineName = arrival.lineName;
      const platform = arrival.platformName;

      if (lineId && lineName) {
        linesByIdMap.set(lineId, lineName);

        if (platform) {
          if (!platformsByLine.has(lineId)) {
            platformsByLine.set(lineId, new Set());
          }
          platformsByLine.get(lineId).add(platform);
        }
      }
    });

    // Convert to array format
    const tubeLines = Array.from(linesByIdMap.entries()).map(([id, name]) => ({
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
    showToast('Failed to fetch tube lines: ' + error.message, 'danger');
    return [];
  }
}

/**
 * Populate line dropdown with lines from TFL API
 */
async function populateTflLines(stationId, currentLine = '', currentPlatform = '') {
  const lineSelect = document.getElementById('configTflLineFilter');
  const platformSelect = document.getElementById('configTflPlatformFilter');

  if (!lineSelect || !platformSelect) return;

  // Show loading state
  lineSelect.innerHTML = '<option value="">Loading lines...</option>';
  lineSelect.disabled = true;
  platformSelect.innerHTML = '<option value="">Loading platforms...</option>';
  platformSelect.disabled = true;

  try {
    const lines = await fetchTflStationLines(stationId);

    if (lines.length === 0) {
      // No lines found - reset to default state
      lineSelect.innerHTML = '<option value="">-- No lines found --</option>';
      lineSelect.disabled = false;
      platformSelect.innerHTML = '<option value="">-- Select Platform --</option>';
      platformSelect.disabled = false;
      showToast('No tube lines found for this station', 'warning');
      return;
    }

    // Populate line dropdown
    lineSelect.innerHTML = '<option value="">-- Select Line --</option>';
    lines.forEach(line => {
      const option = document.createElement('option');
      option.value = line.id;
      option.textContent = line.name;
      option.dataset.platforms = JSON.stringify(line.platforms);
      if (currentLine === line.id) {
        option.selected = true;
      }
      lineSelect.appendChild(option);
    });
    lineSelect.disabled = false;

    // If a line was selected, populate its platforms
    if (currentLine) {
      const selectedLine = lines.find(l => l.id === currentLine);
      if (selectedLine) {
        populatePlatformOptionsFromData(selectedLine.platforms, currentPlatform);
      }
    } else {
      platformSelect.innerHTML = '<option value="">-- Select Line First --</option>';
      platformSelect.disabled = false;
    }

    console.log('Populated', lines.length, 'lines for station', stationId);

  } catch (error) {
    console.error('Error populating TFL lines:', error);
    lineSelect.innerHTML = '<option value="">-- Error loading lines --</option>';
    lineSelect.disabled = false;
    platformSelect.innerHTML = '<option value="">-- Select Platform --</option>';
    platformSelect.disabled = false;
  }
}

/**
 * Populate platform options from API data
 */
function populatePlatformOptionsFromData(platforms, currentPlatform = '') {
  const platformSelect = document.getElementById('configTflPlatformFilter');
  if (!platformSelect) return;

  platformSelect.innerHTML = '<option value="">-- Select Platform --</option>';

  platforms.forEach(platform => {
    const option = document.createElement('option');
    option.value = platform;
    option.textContent = platform;
    if (currentPlatform === platform) {
      option.selected = true;
    }
    platformSelect.appendChild(option);
  });

  platformSelect.disabled = false;

  console.log('Populated', platforms.length, 'platforms');
}

/**
 * Update config form with values from device
 */
function updateConfigForm(config) {
  if (config.service_type) {
    document.getElementById('configServiceType').value = config.service_type;
  }
  if (config.station_code) {
    document.getElementById('configStationCode').value = config.station_code;
  }
  if (config.use_calling_at !== undefined) {
    document.getElementById('configUseCallingAt').value = config.use_calling_at ? '1' : '0';
  }
  if (config.show_station_name !== undefined) {
    document.getElementById('configShowStationName').value = config.show_station_name ? '1' : '0';
  }
  if (config.extra_services !== undefined) {
    document.getElementById('configExtraServices').value = config.extra_services;
  }

  // Handle TFL filters
  const isTFL = config.service_type === 'TFL';

  // Update station label
  const stationLabel = document.getElementById('configStationLabel');
  if (stationLabel) {
    stationLabel.textContent = isTFL ? 'Station NaPTAN Code' : 'Station Code (CRS)';
  }

  // Show/hide TFL filter groups
  const lineFilterGroup = document.getElementById('configTflLineFilterGroup');
  const platformFilterGroup = document.getElementById('configTflPlatformFilterGroup');

  if (lineFilterGroup) lineFilterGroup.style.display = isTFL ? 'block' : 'none';
  if (platformFilterGroup) platformFilterGroup.style.display = isTFL ? 'block' : 'none';

  if (isTFL && config.station_code) {
    // Fetch TFL lines and platforms from API
    populateTflLines(config.station_code, config.tfl_line_filter || '', config.tfl_platform_filter || '');
  }
}

// ==================== Device Actions ====================
/**
 * Restart device
 */
async function restartDevice() {
  if (!confirm('Are you sure you want to restart this device?')) {
    return;
  }

  try {
    const response = await fetch(`/api/devices/${AppState.currentDeviceId}/restart`, {
      method: 'POST'
    });

    if (response.ok) {
      showToast('Restart command sent', 'success');
    } else {
      throw new Error('Device not connected');
    }
  } catch (err) {
    console.error('Error restarting device:', err);
    showToast('Error: Device not connected', 'danger');
  }
}

/**
 * Show OTA dialog
 */
function showOTADialog() {
  const dialog = document.getElementById('otaDialog');
  if (!dialog) return;

  dialog.style.display = 'block';

  const select = document.getElementById('firmwareSelect');
  select.innerHTML = AppState.firmwareList.map(fw =>
    `<option value="${escapeAttr(fw.filename)}">${escapeHtml(fw.version)} (${escapeHtml(formatBytes(fw.size))})</option>`
  ).join('');
}

/**
 * Start OTA update
 */
async function startOTA() {
  const filename = document.getElementById('firmwareSelect').value;

  if (!filename) {
    showToast('Please select a firmware file', 'warning');
    return;
  }

  if (!confirm('Are you sure you want to update the firmware? The device will restart.')) {
    return;
  }

  // Use server's actual IP address (not localhost) so ESP32 can reach it
  const baseUrl = AppState.serverInfo ? AppState.serverInfo.baseUrl : window.location.origin;
  const firmwareUrl = `${baseUrl}/firmware/${encodeURIComponent(filename)}`;

  console.log('Starting OTA update:');
  console.log('  Device:', AppState.currentDeviceId);
  console.log('  Firmware:', filename);
  console.log('  URL:', firmwareUrl);

  try {
    const response = await fetch(`/api/devices/${AppState.currentDeviceId}/ota`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ firmwareUrl })
    });

    if (response.ok) {
      const result = await response.json();
      showToast(`OTA update initiated. Firmware URL: ${firmwareUrl}`, 'success');
      console.log('OTA response:', result);
    } else {
      throw new Error('Device not connected');
    }
  } catch (err) {
    console.error('Error starting OTA:', err);
    showToast('Error: Device not connected', 'danger');
  }
}

// ==================== Firmware Management ====================
/**
 * Load firmware list
 */
async function loadFirmware() {
  try {
    const response = await fetch('/api/firmware');
    AppState.firmwareList = await response.json();
    renderFirmwareList();
  } catch (err) {
    console.error('Error loading firmware:', err);
  }
}

/**
 * Render firmware list
 */
function renderFirmwareList() {
  const list = document.getElementById('firmwareList');

  if (AppState.firmwareList.length === 0) {
    list.innerHTML = '<div class="text-center py-3 text-muted">No firmware uploaded yet</div>';
    return;
  }

  list.innerHTML = AppState.firmwareList.map(fw => `
    <div class="list-group-item">
      <div class="d-flex justify-content-between align-items-center">
        <div class="firmware-info">
          <div class="firmware-version">Version ${escapeHtml(fw.version)}</div>
          <div class="firmware-date">${escapeHtml(formatTimestamp(fw.upload_date))} • ${escapeHtml(formatBytes(fw.size))}</div>
        </div>
        <div class="d-flex align-items-center gap-2">
          <span class="badge bg-primary">${escapeHtml(fw.filename)}</span>
          <button class="btn btn-sm btn-danger" onclick="deleteFirmware(${fw.id}, '${escapeAttr(fw.version)}')">
            <i class="bi bi-trash"></i> Delete
          </button>
        </div>
      </div>
    </div>
  `).join('');
}

/**
 * Upload firmware
 */
async function uploadFirmware(event) {
  event.preventDefault();

  const version = document.getElementById('firmwareVersion').value.trim();
  const fileInput = document.getElementById('firmwareFile');
  const file = fileInput.files[0];
  const submitButton = event.submitter;

  if (!file) {
    showToast('Please select a firmware file', 'warning');
    return;
  }

  if (!version) {
    showToast('Please enter a version number', 'warning');
    return;
  }

  const formData = new FormData();
  formData.append('firmware', file);
  formData.append('version', version);

  try {
    setLoadingState(submitButton, true);
    showToast('Uploading firmware...', 'info');

    const response = await fetch('/api/firmware/upload', {
      method: 'POST',
      body: formData
    });

    if (response.ok) {
      showToast('Firmware uploaded successfully', 'success');
      fileInput.value = '';
      document.getElementById('firmwareVersion').value = '';
      await loadFirmware();
    } else {
      throw new Error('Upload failed');
    }
  } catch (err) {
    console.error('Error uploading firmware:', err);
    showToast('Error uploading firmware', 'danger');
  } finally {
    setLoadingState(submitButton, false);
  }
}

/**
 * Delete firmware
 */
async function deleteFirmware(firmwareId, version) {
  if (!confirm(`Are you sure you want to delete firmware version ${version}?\n\nThis action cannot be undone.`)) {
    return;
  }

  try {
    const response = await fetch(`/api/firmware/${firmwareId}`, {
      method: 'DELETE'
    });

    if (response.ok) {
      showToast(`Firmware ${version} deleted successfully`, 'success');
      await loadFirmware();
    } else {
      const error = await response.json();
      throw new Error(error.error || 'Delete failed');
    }
  } catch (err) {
    console.error('Error deleting firmware:', err);
    showToast(`Error deleting firmware: ${err.message}`, 'danger');
  }
}

/**
 * Show firmware modal
 */
function showFirmwareModal() {
  const modal = new bootstrap.Modal(document.getElementById('firmwareModal'));
  modal.show();
  loadFirmware(); // Refresh firmware list when modal opens
}

// ==================== Event Listeners Setup ====================
/**
 * Setup all event listeners
 */
function setupEventListeners() {
  // Search with debouncing
  const searchInput = document.getElementById('searchInput');
  if (searchInput) {
    const debouncedRender = debounce(renderDevices, CONFIG.DEBOUNCE_DELAY);
    searchInput.addEventListener('input', debouncedRender);
  }

  // Status filter
  const statusFilter = document.getElementById('statusFilter');
  if (statusFilter) {
    statusFilter.addEventListener('change', renderDevices);
  }

  // Refresh button
  const refreshButton = document.querySelector('[data-action="refresh"]');
  if (refreshButton) {
    refreshButton.addEventListener('click', refreshDevices);
  }

  // Config form
  const configForm = document.getElementById('configForm');
  if (configForm) {
    configForm.addEventListener('submit', saveDeviceConfig);
  }

  // Firmware upload form
  const firmwareUploadForm = document.getElementById('firmwareUploadForm');
  if (firmwareUploadForm) {
    firmwareUploadForm.addEventListener('submit', uploadFirmware);
  }

  // Firmware button
  const firmwareButton = document.querySelector('[data-action="firmware"]');
  if (firmwareButton) {
    firmwareButton.addEventListener('click', showFirmwareModal);
  }

  // Restart device button
  const restartButton = document.querySelector('[data-action="restart"]');
  if (restartButton) {
    restartButton.addEventListener('click', restartDevice);
  }

  // OTA button
  const otaButton = document.querySelector('[data-action="show-ota"]');
  if (otaButton) {
    otaButton.addEventListener('click', showOTADialog);
  }

  // Start OTA button
  const startOtaButton = document.querySelector('[data-action="start-ota"]');
  if (startOtaButton) {
    startOtaButton.addEventListener('click', startOTA);
  }

  // Service type change handler
  const serviceTypeSelect = document.getElementById('configServiceType');
  if (serviceTypeSelect) {
    serviceTypeSelect.addEventListener('change', (e) => {
      const isTFL = e.target.value === 'TFL';

      console.log('Service type changed to:', e.target.value, 'isTFL:', isTFL);

      // Update station label
      const stationLabel = document.getElementById('configStationLabel');
      if (stationLabel) {
        stationLabel.textContent = isTFL ? 'Station NaPTAN Code' : 'Station Code (CRS)';
      }

      // Show/hide TFL filter groups
      const lineFilterGroup = document.getElementById('configTflLineFilterGroup');
      const platformFilterGroup = document.getElementById('configTflPlatformFilterGroup');

      if (lineFilterGroup) {
        lineFilterGroup.style.display = isTFL ? 'block' : 'none';
        console.log('Line filter group display:', lineFilterGroup.style.display);
      }
      if (platformFilterGroup) {
        platformFilterGroup.style.display = isTFL ? 'block' : 'none';
        console.log('Platform filter group display:', platformFilterGroup.style.display);
      }

      // Disable calling at mode for TFL
      const callingAtSelect = document.getElementById('configUseCallingAt');
      if (callingAtSelect) {
        if (isTFL) {
          callingAtSelect.disabled = true;
          callingAtSelect.value = '0';
        } else {
          callingAtSelect.disabled = false;
        }
      }

      // Fetch TFL lines when switching to TFL (if we have a station code)
      if (isTFL) {
        const stationCode = document.getElementById('configStationCode').value.trim();
        if (stationCode.length >= 3) {
          console.log('Fetching TFL lines for station:', stationCode);
          populateTflLines(stationCode);
        }
      } else {
        // Clear TFL filters when switching to National Rail
        const lineFilter = document.getElementById('configTflLineFilter');
        const platformFilter = document.getElementById('configTflPlatformFilter');
        if (lineFilter) lineFilter.value = '';
        if (platformFilter) platformFilter.value = '';
      }
    });
  }

  // Station code change handler - fetch TFL lines when station changes
  const stationCodeInput = document.getElementById('configStationCode');
  if (stationCodeInput) {
    let stationCodeTimeout = null;
    stationCodeInput.addEventListener('input', (e) => {
      const serviceType = document.getElementById('configServiceType').value;
      const stationCode = e.target.value.trim();

      // Only fetch TFL lines if service type is TFL and we have a station code
      if (serviceType === 'TFL' && stationCode.length >= 3) {
        // Debounce the API call
        clearTimeout(stationCodeTimeout);
        stationCodeTimeout = setTimeout(() => {
          console.log('Station code changed, fetching TFL lines for:', stationCode);
          populateTflLines(stationCode);
        }, 500); // Wait 500ms after user stops typing
      }
    });
  }

  // TFL Line filter change handler - populate platforms from dataset
  const tflLineFilter = document.getElementById('configTflLineFilter');
  if (tflLineFilter) {
    tflLineFilter.addEventListener('change', (e) => {
      const selectedOption = e.target.selectedOptions[0];
      console.log('TFL line changed to:', e.target.value);

      if (selectedOption && selectedOption.dataset.platforms) {
        // Parse platforms from the data attribute
        const platforms = JSON.parse(selectedOption.dataset.platforms);
        populatePlatformOptionsFromData(platforms);
      } else {
        // Clear platforms if no line selected
        const platformSelect = document.getElementById('configTflPlatformFilter');
        if (platformSelect) {
          platformSelect.innerHTML = '<option value="">-- Select Platform --</option>';
        }
      }
    });
  }
}

// ==================== Log Streaming ====================
/**
 * Enable log streaming from device
 */
async function enableLogStreaming() {
  if (!AppState.currentDeviceId) {
    showToast('No device selected', 'warning');
    return;
  }

  try {
    const response = await fetch(`/api/devices/${AppState.currentDeviceId}/enableLogs`, {
      method: 'POST'
    });

    if (response.ok) {
      showToast('Log streaming started', 'success');
      clearLiveLogs();
      document.getElementById('liveLogsContent').innerHTML = '<div style="color: #4ec9b0;">Waiting for logs...</div>';
    } else {
      throw new Error('Device not connected');
    }
  } catch (err) {
    console.error('Error enabling logs:', err);
    showToast('Error: Device not connected', 'danger');
  }
}

/**
 * Disable log streaming from device
 */
async function disableLogStreaming() {
  if (!AppState.currentDeviceId) {
    return;
  }

  try {
    const response = await fetch(`/api/devices/${AppState.currentDeviceId}/disableLogs`, {
      method: 'POST'
    });

    if (response.ok) {
      showToast('Log streaming stopped', 'info');
    }
  } catch (err) {
    console.error('Error disabling logs:', err);
  }
}

/**
 * Append live log entry
 */
function appendLiveLog(data) {
  const container = document.getElementById('liveLogsContent');
  const logEntry = document.createElement('div');
  logEntry.style.marginBottom = '2px';

  // Color based on log level
  const colors = {
    error: '#f48771',
    warn: '#dcdcaa',
    info: '#4ec9b0',
    debug: '#9cdcfe'
  };
  const color = colors[data.level] || '#d4d4d4';

  const timestamp = new Date(data.timestamp).toLocaleTimeString();
  logEntry.innerHTML = `<span style="color: #858585;">[${timestamp}]</span> <span style="color: ${color};">${escapeHtml(data.message)}</span>`;

  container.appendChild(logEntry);

  // Auto-scroll to bottom
  const scrollContainer = document.getElementById('liveLogsContainer');
  scrollContainer.scrollTop = scrollContainer.scrollHeight;

  // Limit to 500 lines
  while (container.children.length > 500) {
    container.removeChild(container.firstChild);
  }
}

/**
 * Clear live logs
 */
function clearLiveLogs() {
  document.getElementById('liveLogsContent').innerHTML = '<div style="color: #888;">Logs cleared</div>';
}

// ==================== Initialization ====================
/**
 * Initialize application
 */
document.addEventListener('DOMContentLoaded', () => {
  console.log('Initializing StationBoards Dashboard...');

  loadServerInfo();
  loadDevices();
  loadFirmware();
  setupEventListeners();
  setupWebSocket();

  console.log('Dashboard initialized');
});
