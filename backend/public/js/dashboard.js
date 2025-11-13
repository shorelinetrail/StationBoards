// Connect to server
const socket = io();

let devices = [];
let currentDeviceId = null;
let firmwareList = [];
let serverInfo = null; // Store server IP info

// Initialize
document.addEventListener('DOMContentLoaded', () => {
  loadServerInfo();
  loadDevices();
  loadFirmware();
  setupEventListeners();
  setupWebSocket();
});

// Load server info (IP address for firmware URLs)
async function loadServerInfo() {
  try {
    const response = await fetch('/api/server-info');
    serverInfo = await response.json();
    console.log('Server info loaded:', serverInfo);
  } catch (err) {
    console.error('Error loading server info:', err);
    // Fallback to window.location.origin
    serverInfo = {
      baseUrl: window.location.origin
    };
  }
}

// WebSocket event handlers
function setupWebSocket() {
  socket.on('connect', () => {
    console.log('Connected to server');
    showToast('Connected to monitoring server', 'success');
  });

  socket.on('disconnect', () => {
    console.log('Disconnected from server');
    showToast('Disconnected from server', 'warning');
  });

  socket.on('deviceUpdate', (data) => {
    console.log('Device update:', data);
    updateDeviceInList(data);
  });
}

// Load all devices
async function loadDevices() {
  try {
    const response = await fetch('/api/devices');
    devices = await response.json();
    renderDevices();
    updateCounts();
  } catch (err) {
    console.error('Error loading devices:', err);
    showToast('Error loading devices', 'danger');
  }
}

// Render devices grid
function renderDevices() {
  const grid = document.getElementById('devicesGrid');
  const searchTerm = document.getElementById('searchInput').value.toLowerCase();
  const statusFilter = document.getElementById('statusFilter').value;

  let filtered = devices.filter(device => {
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
          <i class="bi bi-inbox"></i>
          <h4>No devices found</h4>
          <p>No devices match your search criteria</p>
        </div>
      </div>
    `;
    return;
  }

  grid.innerHTML = filtered.map(device => createDeviceCard(device)).join('');
}

// Create device card HTML
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
      <div class="card device-card shadow-sm ${statusClass}" onclick="showDeviceDetails('${device.id}')">
        <div class="card-header bg-white">
          <div>
            <h6 class="mb-0">${device.name}</h6>
            <small class="text-muted">${device.station_code || 'N/A'}</small>
          </div>
          ${statusBadge}
        </div>
        <div class="card-body">
          <div class="device-icon text-center">
            <i class="bi bi-display"></i>
          </div>
          
          <div class="metric-item">
            <span class="metric-label">Station:</span>
            <span class="metric-value">${device.station_name || 'Unknown'}</span>
          </div>
          
          <div class="metric-item">
            <span class="metric-label">Signal:</span>
            <span class="metric-value">${rssiIcon} ${device.rssi || 0} dBm</span>
          </div>
          
          <div class="metric-item">
            <span class="metric-label">Services:</span>
            <span class="metric-value">${device.services_count || 0}</span>
          </div>
          
          <div class="metric-item">
            <span class="metric-label">Uptime:</span>
            <span class="metric-value">${uptime}</span>
          </div>
          
          <div class="metric-item">
            <span class="metric-label">Last Seen:</span>
            <span class="metric-value">${lastSeen}</span>
          </div>
          
          <div class="metric-item">
            <span class="metric-label">Firmware:</span>
            <span class="metric-value">${device.firmware_version || 'Unknown'}</span>
          </div>
        </div>
      </div>
    </div>
  `;
}

// Show device details modal
async function showDeviceDetails(deviceId) {
  currentDeviceId = deviceId;
  
  try {
    const response = await fetch(`/api/devices/${deviceId}`);
    const data = await response.json();
    
    populateDeviceModal(data);
    
    const modal = new bootstrap.Modal(document.getElementById('deviceModal'));
    modal.show();
  } catch (err) {
    console.error('Error loading device details:', err);
    showToast('Error loading device details', 'danger');
  }
}

// Populate device modal with data
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
  document.getElementById('infoLastSeen').textContent = formatLastSeen(device.last_seen);
  document.getElementById('infoRSSI').innerHTML = `${getRSSIIcon(device.rssi)} ${device.rssi || 0} dBm`;
  document.getElementById('infoUptime').textContent = formatUptime(device.uptime);
  document.getElementById('infoHeap').textContent = formatBytes(device.free_heap);
  document.getElementById('infoServices').textContent = device.services_count || 0;
  
  // Config tab
  if (config) {
    document.getElementById('configDeviceId').value = device.id;
    document.getElementById('configStationCode').value = config.station_code || '';
    document.getElementById('configRefreshInterval').value = config.refresh_interval || 60;
    document.getElementById('configUseCallingAt').value = config.use_calling_at ? '1' : '0';
    document.getElementById('configShowStationName').value = config.show_station_name ? '1' : '0';
    document.getElementById('configExtraServices').value = config.extra_services || 1;
    document.getElementById('configScrollSpeed').value = config.scroll_speed || 50;
    document.getElementById('configRotationSpeed').value = config.rotation_speed || 15;
  }
  
  // Logs tab
  const logsTable = document.getElementById('logsTableBody');
  if (logs && logs.length > 0) {
    logsTable.innerHTML = logs.map(log => `
      <tr class="log-event ${log.event_type}">
        <td>${formatTimestamp(log.timestamp)}</td>
        <td><span class="badge bg-secondary">${log.event_type}</span></td>
        <td>${log.message}</td>
      </tr>
    `).join('');
  } else {
    logsTable.innerHTML = '<tr><td colspan="3" class="text-center">No logs available</td></tr>';
  }
}

// Setup event listeners
function setupEventListeners() {
  // Search
  document.getElementById('searchInput').addEventListener('input', renderDevices);
  
  // Status filter
  document.getElementById('statusFilter').addEventListener('change', renderDevices);
  
  // Config form
  document.getElementById('configForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    await saveDeviceConfig();
  });
  
  // Firmware upload form
  document.getElementById('firmwareUploadForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    await uploadFirmware();
  });
}

// Save device configuration
async function saveDeviceConfig() {
  const deviceId = document.getElementById('configDeviceId').value;
  
  const config = {
    stationCode: document.getElementById('configStationCode').value,
    refreshInterval: parseInt(document.getElementById('configRefreshInterval').value),
    useCallingAt: document.getElementById('configUseCallingAt').value === '1',
    showStationName: document.getElementById('configShowStationName').value === '1',
    extraServices: parseInt(document.getElementById('configExtraServices').value),
    scrollSpeed: parseInt(document.getElementById('configScrollSpeed').value),
    rotationSpeed: parseInt(document.getElementById('configRotationSpeed').value)
  };
  
  try {
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
      if (currentDeviceId === deviceId) {
        const deviceResponse = await fetch(`/api/devices/${deviceId}`);
        const data = await deviceResponse.json();
        populateDeviceModal(data);
      }
    } else {
      throw new Error('Failed to save configuration');
    }
  } catch (err) {
    console.error('Error saving config:', err);
    showToast('Error saving configuration', 'danger');
  }
}

// Restart device
async function restartDevice() {
  if (!confirm('Are you sure you want to restart this device?')) {
    return;
  }
  
  try {
    const response = await fetch(`/api/devices/${currentDeviceId}/restart`, {
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

// Show OTA dialog
function showOTADialog() {
  document.getElementById('otaDialog').style.display = 'block';
  
  const select = document.getElementById('firmwareSelect');
  select.innerHTML = firmwareList.map(fw => 
    `<option value="${fw.filename}">${fw.version} (${formatBytes(fw.size)})</option>`
  ).join('');
}

// Start OTA update
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
  const baseUrl = serverInfo ? serverInfo.baseUrl : window.location.origin;
  const firmwareUrl = `${baseUrl}/firmware/${filename}`;
  
  console.log('Starting OTA update:');
  console.log('  Device:', currentDeviceId);
  console.log('  Firmware:', filename);
  console.log('  URL:', firmwareUrl);
  
  try {
    const response = await fetch(`/api/devices/${currentDeviceId}/ota`, {
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

// Load firmware list
async function loadFirmware() {
  try {
    const response = await fetch('/api/firmware');
    firmwareList = await response.json();
    renderFirmwareList();
  } catch (err) {
    console.error('Error loading firmware:', err);
  }
}

// Render firmware list
function renderFirmwareList() {
  const list = document.getElementById('firmwareList');
  
  if (firmwareList.length === 0) {
    list.innerHTML = '<div class="text-center py-3 text-muted">No firmware uploaded yet</div>';
    return;
  }
  
  list.innerHTML = firmwareList.map(fw => `
    <div class="list-group-item">
      <div class="d-flex justify-content-between align-items-center">
        <div class="firmware-info">
          <div class="firmware-version">Version ${fw.version}</div>
          <div class="firmware-date">${formatTimestamp(fw.uploaded_at)} • ${formatBytes(fw.size)}</div>
        </div>
        <span class="badge bg-primary">${fw.filename}</span>
      </div>
    </div>
  `).join('');
}

// Upload firmware
async function uploadFirmware() {
  const version = document.getElementById('firmwareVersion').value;
  const fileInput = document.getElementById('firmwareFile');
  const file = fileInput.files[0];
  
  if (!file) {
    showToast('Please select a firmware file', 'warning');
    return;
  }
  
  const formData = new FormData();
  formData.append('firmware', file);
  formData.append('version', version);
  
  try {
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
  }
}

// Update device in list
function updateDeviceInList(data) {
  const index = devices.findIndex(d => d.id === data.deviceId);
  
  if (index !== -1) {
    // Update existing device
    devices[index] = { ...devices[index], ...data, id: data.deviceId };
  } else {
    // Add new device
    devices.push({ id: data.deviceId, ...data });
  }
  
  renderDevices();
  updateCounts();
}

// Update device counts
function updateCounts() {
  document.getElementById('deviceCount').textContent = devices.length;
  const onlineCount = devices.filter(d => d.status === 'online').length;
  document.getElementById('onlineCount').textContent = `${onlineCount} Online`;
}

// Refresh devices
function refreshDevices() {
  loadDevices();
  showToast('Refreshing devices...', 'info');
}

// Helper functions
function formatLastSeen(timestamp) {
  if (!timestamp) return 'Never';
  const date = new Date(timestamp * 1000);
  const now = Date.now();
  const diff = now - date.getTime();
  
  if (diff < 60000) return 'Just now';
  if (diff < 3600000) return `${Math.floor(diff / 60000)}m ago`;
  if (diff < 86400000) return `${Math.floor(diff / 3600000)}h ago`;
  return `${Math.floor(diff / 86400000)}d ago`;
}

function formatTimestamp(timestamp) {
  if (!timestamp) return 'N/A';
  const date = new Date(timestamp * 1000);
  return date.toLocaleString();
}

function formatUptime(seconds) {
  if (!seconds) return 'N/A';
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  
  if (days > 0) return `${days}d ${hours}h`;
  if (hours > 0) return `${hours}h ${minutes}m`;
  return `${minutes}m`;
}

function formatBytes(bytes) {
  if (!bytes) return 'N/A';
  if (bytes < 1024) return bytes + ' B';
  if (bytes < 1048576) return (bytes / 1024).toFixed(1) + ' KB';
  return (bytes / 1048576).toFixed(1) + ' MB';
}

function getRSSIIcon(rssi) {
  if (!rssi) return '<i class="bi bi-wifi text-muted"></i>';
  if (rssi > -50) return '<i class="bi bi-wifi rssi-excellent"></i>';
  if (rssi > -60) return '<i class="bi bi-wifi rssi-good"></i>';
  if (rssi > -70) return '<i class="bi bi-wifi rssi-fair"></i>';
  return '<i class="bi bi-wifi rssi-poor"></i>';
}

function showToast(message, type = 'info') {
  // Simple toast notification
  const toast = document.createElement('div');
  toast.className = `alert alert-${type} alert-dismissible fade show`;
  toast.style.cssText = 'position: fixed; top: 70px; right: 20px; z-index: 9999; min-width: 300px;';
  toast.innerHTML = `
    ${message}
    <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
  `;
  document.body.appendChild(toast);
  
  setTimeout(() => toast.remove(), 5000);
}
