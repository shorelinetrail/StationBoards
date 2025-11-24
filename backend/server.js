const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const WebSocket = require('ws');
const sqlite3 = require('sqlite3').verbose();
const multer = require('multer');
const path = require('path');
const fs = require('fs');
const session = require('express-session');
const bcrypt = require('bcryptjs');
const SQLiteStore = require('connect-sqlite3')(session);

const app = express();
const server = http.createServer(app);

// Trust Railway proxy for secure cookies
app.set('trust proxy', 1);

// Socket.IO for web dashboard
const io = socketIo(server);

// Plain WebSocket server for ESP32 devices
const wss = new WebSocket.Server({ 
  server,
  path: '/ws',  // ESP32s will connect to ws://IP:3000/ws
  clientTracking: true
});

// Database setup
const db = new sqlite3.Database('./stationboards.db');

// Initialize database tables
db.serialize(() => {
  db.run(`CREATE TABLE IF NOT EXISTS devices (
    id TEXT PRIMARY KEY,
    name TEXT,
    ip TEXT,
    firmware_version TEXT,
    station_code TEXT,
    station_name TEXT,
    service_type TEXT DEFAULT 'National Rail',
    rssi INTEGER,
    uptime INTEGER,
    free_heap INTEGER,
    services INTEGER,
    last_seen INTEGER,
    status TEXT DEFAULT 'offline',
    first_seen INTEGER,
    use_calling_at INTEGER DEFAULT 1,
    extra_services INTEGER DEFAULT 0,
    rotation_speed INTEGER DEFAULT 5000,
    refresh_interval INTEGER DEFAULT 60,
    scroll_speed INTEGER DEFAULT 50,
    show_station_name INTEGER DEFAULT 1
  )`);

  // Add new columns if they don't exist (for existing databases)
  db.run(`ALTER TABLE devices ADD COLUMN scroll_speed INTEGER DEFAULT 50`, () => {});
  db.run(`ALTER TABLE devices ADD COLUMN show_station_name INTEGER DEFAULT 1`, () => {});
  db.run(`ALTER TABLE devices ADD COLUMN service_type TEXT DEFAULT 'National Rail'`, () => {});
  db.run(`ALTER TABLE devices ADD COLUMN tfl_line_filter TEXT DEFAULT ''`, () => {});
  db.run(`ALTER TABLE devices ADD COLUMN tfl_platform_filter TEXT DEFAULT ''`, () => {});
  
  // Data migration: Fix rotation_speed values
  // 1. Fix values in seconds (< 1000) → convert to milliseconds
  db.run(`UPDATE devices SET rotation_speed = rotation_speed * 1000 WHERE rotation_speed > 0 AND rotation_speed < 1000`, (err) => {
    if (!err) {
      console.log('✓ Migration: Fixed rotation_speed values (seconds → milliseconds)');
    }
  });
  
  // 2. Fix NULL or 0 values → set to default (5000ms = 5 seconds)
  db.run(`UPDATE devices SET rotation_speed = 5000 WHERE rotation_speed IS NULL OR rotation_speed = 0`, (err) => {
    if (!err) {
      console.log('✓ Migration: Fixed NULL/0 rotation_speed values → 5000ms');
    }
  });
  db.run(`CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT,
    event_type TEXT,
    message TEXT,
    timestamp INTEGER,
    FOREIGN KEY(device_id) REFERENCES devices(id)
  )`);

  db.run(`CREATE TABLE IF NOT EXISTS firmware (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    version TEXT UNIQUE,
    filename TEXT,
    upload_date INTEGER,
    size INTEGER
  )`);
});

// ============================================================================
// Authentication Setup
// ============================================================================

// Get credentials from environment variables (set in Railway)
const ADMIN_USERNAME = process.env.ADMIN_USERNAME || 'admin';
const ADMIN_PASSWORD_HASH = process.env.ADMIN_PASSWORD_HASH || null;

// If no password hash is set, create one for default password "admin123"
// IMPORTANT: Change this in production!
let adminPasswordHash = ADMIN_PASSWORD_HASH;
if (!adminPasswordHash) {
  console.warn('⚠️  WARNING: Using default password! Set ADMIN_PASSWORD_HASH environment variable!');
  adminPasswordHash = bcrypt.hashSync('admin123', 10);
}

// ============================================================================
// Cloud Monitoring Integration
// ============================================================================

// Optional cloud monitoring endpoint (for stationboards.co.uk integration)
const CLOUD_MONITORING_URL = process.env.CLOUD_MONITORING_URL || '';
const CLOUD_MONITORING_ENABLED = !!CLOUD_MONITORING_URL;

if (CLOUD_MONITORING_ENABLED) {
  console.log('☁️  Cloud monitoring enabled:', CLOUD_MONITORING_URL);
} else {
  console.log('📍 Cloud monitoring disabled (local only)');
}

/**
 * Forward device heartbeat to cloud monitoring system
 * This integrates the local backend with the website's monitoring dashboard
 */
async function forwardHeartbeatToCloud(deviceId) {
  if (!CLOUD_MONITORING_ENABLED) return;

  try {
    const https = require('https');
    const http = require('http');

    const url = new URL(CLOUD_MONITORING_URL);
    const protocol = url.protocol === 'https:' ? https : http;

    const data = JSON.stringify({
      board_id: deviceId
    });

    const options = {
      hostname: url.hostname,
      port: url.port || (url.protocol === 'https:' ? 443 : 80),
      path: url.pathname,
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': data.length
      }
    };

    const req = protocol.request(options, (res) => {
      let responseData = '';
      res.on('data', (chunk) => {
        responseData += chunk;
      });
      res.on('end', () => {
        if (res.statusCode === 200) {
          try {
            const json = JSON.parse(responseData);
            if (json.success) {
              console.log(`☁️  Cloud heartbeat sent for ${deviceId}`);
            } else {
              console.warn(`⚠️  Cloud heartbeat failed for ${deviceId}:`, json.error);
            }
          } catch (e) {
            console.error(`❌ Cloud heartbeat parse error:`, responseData);
          }
        } else {
          console.error(`❌ Cloud heartbeat HTTP ${res.statusCode} for ${deviceId}`);
        }
      });
    });

    req.on('error', (error) => {
      console.error(`❌ Cloud heartbeat network error for ${deviceId}:`, error.message);
    });

    req.write(data);
    req.end();

  } catch (error) {
    console.error(`❌ Cloud heartbeat error for ${deviceId}:`, error.message);
  }
}

// Session configuration
const sessionMiddleware = session({
  store: new SQLiteStore({
    db: 'sessions.db',
    dir: './'
  }),
  secret: process.env.SESSION_SECRET || 'stationboards-secret-change-in-production',
  resave: false,
  saveUninitialized: false,
  cookie: {
    secure: process.env.NODE_ENV === 'production', // HTTPS only in production
    httpOnly: true,
    maxAge: 7 * 24 * 60 * 60 * 1000 // 7 days
  }
});

// Authentication middleware
function requireAuth(req, res, next) {
  if (req.session && req.session.authenticated) {
    return next();
  }

  // For API requests, return 401
  if (req.path.startsWith('/api/')) {
    return res.status(401).json({ error: 'Authentication required' });
  }

  // For page requests, redirect to login
  res.redirect('/login.html');
}

// Middleware
app.use(express.json());
app.use(sessionMiddleware);

// Serve static files EXCEPT index.html (which requires auth)
app.use((req, res, next) => {
  if (req.path === '/' || req.path === '/index.html') {
    return next(); // Don't serve index.html statically, let the protected route handle it
  }
  express.static('public')(req, res, next);
});
app.use('/firmware', express.static('firmware'));

// Multer for firmware uploads
const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    const dir = './firmware';
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir);
    }
    cb(null, dir);
  },
  filename: (req, file, cb) => {
    // Use temporary filename - we'll rename it after we have access to req.body
    cb(null, `temp-${Date.now()}.bin`);
  }
});

const upload = multer({ storage: storage });

// Store active WebSocket connections (ESP32 devices)
const wsClients = new Map(); // deviceId -> WebSocket

// ============================================================================
// WebSocket Handler for ESP32 Devices
// ============================================================================

wss.on('connection', (ws, req) => {
  console.log('📱 New WebSocket connection from:', req.socket.remoteAddress);

  let deviceId = null;
  let heartbeatInterval = null;

  ws.on('message', (data) => {
    try {
      const message = JSON.parse(data.toString());
      console.log('📨 Received from device:', message.type, message.deviceId || '');

      switch (message.type) {
        case 'register':
          handleDeviceRegister(ws, message);
          deviceId = message.deviceId;

          // Store WebSocket connection
          wsClients.set(deviceId, ws);
          console.log(`✓ Device registered in wsClients: ${deviceId} (total: ${wsClients.size})`);

          // Send acknowledgment
          ws.send(JSON.stringify({
            type: 'registered',
            success: true,
            message: 'Device registered successfully'
          }));

          // Notify web dashboard
          io.emit('deviceUpdate', {
            deviceId: deviceId,
            status: 'online'
          });
          break;

        case 'heartbeat':
          handleDeviceHeartbeat(message);

          // If deviceId not set yet, set it from heartbeat (handles reconnections)
          if (!deviceId && message.deviceId) {
            deviceId = message.deviceId;
            wsClients.set(deviceId, ws);
            console.log(`✓ Device tracked via heartbeat: ${deviceId} (total: ${wsClients.size})`);
          }

          // Echo heartbeat acknowledgment
          ws.send(JSON.stringify({
            type: 'heartbeat_ack',
            timestamp: Date.now()
          }));
          break;

        case 'metrics':
          handleDeviceMetrics(message);
          
          // Broadcast to dashboard
          io.emit('metricsUpdate', message);
          break;

        case 'status':
          handleDeviceStatus(message);
          
          // Broadcast to dashboard
          io.emit('statusUpdate', message);
          break;

        case 'log':
          handleDeviceLog(message);

          // Broadcast to dashboard
          io.emit('logUpdate', message);
          break;

        case 'configResponse':
          console.log('📖 Config response from device:', message.deviceId);

          // Broadcast config to dashboard
          io.emit('configResponse', {
            deviceId: message.deviceId,
            config: {
              station_code: message.stationCode,
              station_name: message.stationName,
              service_type: message.serviceType,
              use_calling_at: message.useCallingAt,
              show_station_name: message.showStationName,
              extra_services: message.extraServices,
              tfl_line_filter: message.tflLineFilter,
              tfl_platform_filter: message.tflPlatformFilter
            }
          });
          break;

        case 'configUpdate':
          console.log('🔄 Config update from device:', message.deviceId);
          handleDeviceConfigUpdate(message);

          // Broadcast config update to dashboard
          io.emit('configUpdate', {
            deviceId: message.deviceId,
            config: {
              station_code: message.stationCode,
              station_name: message.stationName,
              service_type: message.serviceType,
              use_calling_at: message.useCallingAt,
              show_station_name: message.showStationName,
              extra_services: message.extraServices,
              tfl_line_filter: message.tflLineFilter,
              tfl_platform_filter: message.tflPlatformFilter
            }
          });
          break;

        default:
          console.log('⚠️  Unknown message type:', message.type);
      }
    } catch (error) {
      console.error('❌ Error processing message:', error);
    }
  });

  ws.on('close', () => {
    console.log('📱 WebSocket disconnected:', deviceId);
    
    if (deviceId) {
      wsClients.delete(deviceId);
      
      // Mark device as offline
      updateDeviceStatus(deviceId, 'offline');
      
      // Notify dashboard
      io.emit('deviceUpdate', {
        deviceId: deviceId,
        status: 'offline'
      });
    }
  });

  ws.on('error', (error) => {
    console.error('❌ WebSocket error:', error);
  });

  // Send initial ping to verify connection
  ws.send(JSON.stringify({
    type: 'ping',
    timestamp: Date.now()
  }));
});

// ============================================================================
// Device Message Handlers
// ============================================================================

function handleDeviceRegister(ws, data) {
  const stmt = db.prepare(`
    INSERT OR REPLACE INTO devices
    (id, name, ip, firmware_version, station_code, station_name, service_type, rssi, uptime,
     free_heap, services, last_seen, status, first_seen, use_calling_at,
     extra_services, rotation_speed, refresh_interval, scroll_speed, show_station_name,
     tfl_line_filter, tfl_platform_filter)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 'online',
            COALESCE((SELECT first_seen FROM devices WHERE id = ?), ?),
            ?, ?, ?, ?, ?, ?, ?, ?)
  `);

  const now = Date.now();

  stmt.run(
    data.deviceId,
    data.name || 'Board-' + data.deviceId.substring(0, 8),
    data.ip,
    data.firmwareVersion,
    data.stationCode,
    data.stationName,
    data.serviceType || 'National Rail',
    data.rssi,
    data.uptime,
    data.freeHeap,
    data.services,
    now,
    data.deviceId, now,
    data.useCallingAt !== undefined ? (data.useCallingAt ? 1 : 0) : 1,
    data.extraServices || 0,
    data.rotationSpeed || 5000,
    data.refreshInterval || 60,
    data.scrollSpeed || 50,
    data.showStationName !== undefined ? (data.showStationName ? 1 : 0) : 1,
    data.tflLineFilter || '',
    data.tflPlatformFilter || ''
  );

  stmt.finalize();

  // Log event
  logEvent(data.deviceId, 'connection', 'Device connected');

  console.log('✅ Device registered:', data.deviceId, '-', data.name);
}

function handleDeviceHeartbeat(data) {
  // Build dynamic SQL based on what fields are present
  let fields = ['rssi', 'uptime', 'free_heap', 'services', 'last_seen', 'status'];
  let values = [data.rssi, data.uptime, data.freeHeap, data.services, Date.now(), 'online'];

  // If station info is provided, update it too
  if (data.stationCode) {
    fields.push('station_code');
    values.push(data.stationCode);
  }
  if (data.stationName) {
    fields.push('station_name');
    values.push(data.stationName);
  }
  if (data.serviceType) {
    fields.push('service_type');
    values.push(data.serviceType);
  }

  // Add deviceId for WHERE clause
  values.push(data.deviceId);

  const updateSQL = `UPDATE devices SET ${fields.map(f => `${f} = ?`).join(', ')} WHERE id = ?`;

  db.run(updateSQL, values, (err) => {
    if (err) {
      console.error('Error updating device heartbeat:', err);
    } else {
      // Forward heartbeat to cloud monitoring (if enabled)
      forwardHeartbeatToCloud(data.deviceId);

      if (data.stationCode || data.stationName) {
        // Broadcast station update to web clients
        io.emit('deviceUpdate', {
          deviceId: data.deviceId,
          station_code: data.stationCode,
          station_name: data.stationName,
          service_type: data.serviceType
        });
      }
    }
  });
}

function handleDeviceMetrics(data) {
  // Update device metrics in database
  handleDeviceHeartbeat(data);
}

function handleDeviceStatus(data) {
  logEvent(data.deviceId, 'status', data.message);
}

function handleDeviceLog(data) {
  logEvent(data.deviceId, 'log', data.message);
}

function handleDeviceConfigUpdate(data) {
  db.run(
    `UPDATE devices
     SET station_code = ?, station_name = ?, service_type = ?,
         use_calling_at = ?, show_station_name = ?, extra_services = ?,
         refresh_interval = ?, scroll_speed = ?, rotation_speed = ?,
         tfl_line_filter = ?, tfl_platform_filter = ?
     WHERE id = ?`,
    [
      data.stationCode,
      data.stationName,
      data.serviceType,
      data.useCallingAt ? 1 : 0,
      data.showStationName ? 1 : 0,
      data.extraServices || 0,
      data.refreshInterval || 60,
      data.scrollSpeed || 50,
      data.rotationSpeed || 5000,
      data.tflLineFilter || '',
      data.tflPlatformFilter || '',
      data.deviceId
    ],
    (err) => {
      if (err) {
        console.error('Error updating device config:', err);
      } else {
        console.log('✅ Device config updated in database:', data.deviceId);
        logEvent(data.deviceId, 'config_change', `Config updated: ${data.stationCode} - ${data.serviceType}`);
      }
    }
  );
}

function updateDeviceStatus(deviceId, status) {
  db.run(
    'UPDATE devices SET status = ?, last_seen = ? WHERE id = ?',
    [status, Date.now(), deviceId]
  );

  if (status === 'offline') {
    logEvent(deviceId, 'disconnection', 'Device disconnected');
  }
}

function logEvent(deviceId, eventType, message) {
  db.run(
    'INSERT INTO events (device_id, event_type, message, timestamp) VALUES (?, ?, ?, ?)',
    [deviceId, eventType, message, Date.now()]
  );
}

// ============================================================================
// Socket.IO Handler for Web Dashboard
// ============================================================================

io.on('connection', (socket) => {
  console.log('🌐 Web client connected');

  // Send initial device list
  db.all('SELECT * FROM devices ORDER BY name', (err, devices) => {
    if (!err) {
      socket.emit('initialDevices', devices);
    }
  });

  // Handle commands from web dashboard to devices
  socket.on('sendCommand', async (data) => {
    const { deviceId, command, params } = data;
    console.log('📤 Sending command to device:', deviceId, command);

    const ws = wsClients.get(deviceId);
    
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({
        type: 'command',
        command: command,
        ...params
      }));

      socket.emit('commandSent', {
        success: true,
        message: 'Command sent to device'
      });
    } else {
      socket.emit('commandSent', {
        success: false,
        message: 'Device not connected'
      });
    }
  });

  socket.on('disconnect', () => {
    console.log('🌐 Web client disconnected');
  });
});

// ============================================================================
// Authentication Routes (unprotected)
// ============================================================================

// Login endpoint
app.post('/api/auth/login', (req, res) => {
  const { username, password, remember } = req.body;

  if (username === ADMIN_USERNAME && bcrypt.compareSync(password, adminPasswordHash)) {
    req.session.authenticated = true;
    req.session.username = username;

    // Extend session if "remember me" is checked
    if (remember) {
      req.session.cookie.maxAge = 30 * 24 * 60 * 60 * 1000; // 30 days
    }

    res.json({ success: true });
  } else {
    res.status(401).json({ error: 'Invalid username or password' });
  }
});

// Logout endpoint
app.post('/api/auth/logout', (req, res) => {
  req.session.destroy();
  res.json({ success: true });
});

// Check authentication status
app.get('/api/auth/status', (req, res) => {
  res.json({
    authenticated: !!(req.session && req.session.authenticated),
    username: req.session?.username || null
  });
});

// ============================================================================
// Protected Routes - Dashboard and API
// ============================================================================

// Protect the main dashboard
app.get('/', requireAuth, (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// ============================================================================
// REST API Endpoints (all protected)
// ============================================================================

// Get server info (for constructing firmware URLs)
app.get('/api/server-info', requireAuth, (req, res) => {
  const os = require('os');
  const interfaces = os.networkInterfaces();
  const addresses = [];

  // Get all non-internal IPv4 addresses
  for (const name of Object.keys(interfaces)) {
    for (const iface of interfaces[name]) {
      if (iface.family === 'IPv4' && !iface.internal) {
        addresses.push(iface.address);
      }
    }
  }

  // Prefer the first non-localhost address, fallback to localhost
  const serverIp = addresses.length > 0 ? addresses[0] : 'localhost';
  const port = PORT;

  // For Railway/production: use the actual request host (e.g., stationboards.up.railway.app)
  // For local development: use the local IP
  let baseUrl;
  if (process.env.RAILWAY_PUBLIC_DOMAIN || req.get('host').includes('railway.app')) {
    // Running on Railway - use public HTTPS URL
    const protocol = req.protocol; // 'https' on Railway
    const host = req.get('host'); // e.g., 'stationboards.up.railway.app'
    baseUrl = `${protocol}://${host}`;
  } else {
    // Local development - use local IP
    baseUrl = `http://${serverIp}:${port}`;
  }

  res.json({
    ip: serverIp,
    port: port,
    baseUrl: baseUrl,
    allAddresses: addresses
  });
});

// Get all devices
app.get('/api/devices', requireAuth, (req, res) => {
  db.all('SELECT * FROM devices ORDER BY name', (err, devices) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      // Convert last_seen from milliseconds to seconds for consistency
      const formattedDevices = devices.map(device => ({
        ...device,
        last_seen: device.last_seen ? Math.floor(device.last_seen / 1000) : null
      }));
      res.json(formattedDevices);
    }
  });
});

// Get single device
app.get('/api/devices/:id', requireAuth, (req, res) => {
  db.get('SELECT * FROM devices WHERE id = ?', [req.params.id], (err, device) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else if (!device) {
      res.status(404).json({ error: 'Device not found' });
    } else {
      // Get recent logs for this device
      db.all(
        'SELECT * FROM events WHERE device_id = ? ORDER BY timestamp DESC LIMIT 50',
        [req.params.id],
        (err, logs) => {
          // Convert logs to expected format
          const formattedLogs = (logs || []).map(log => ({
            event_type: log.event_type,
            message: log.message,
            timestamp: Math.floor(log.timestamp / 1000) // Convert ms to seconds
          }));
          
          // Return structured response with device info, config, and logs
          res.json({
            device: {
              id: device.id,
              name: device.name,
              ip_address: device.ip,
              firmware_version: device.firmware_version,
              station_code: device.station_code,
              station_name: device.station_name,
              service_type: device.service_type,
              rssi: device.rssi,
              uptime: device.uptime,
              free_heap: device.free_heap,
              services_count: device.services,
              last_seen: Math.floor(device.last_seen / 1000), // Convert ms to seconds
              status: device.status
            },
            config: {
              station_code: device.station_code,
              station_name: device.station_name,
              service_type: device.service_type,
              use_calling_at: device.use_calling_at === 1,
              show_station_name: device.show_station_name === 1,
              extra_services: device.extra_services,
              tfl_line_filter: device.tfl_line_filter || '',
              tfl_platform_filter: device.tfl_platform_filter || ''
            },
            logs: formattedLogs
          });
        }
      );
    }
  });
});

// Update device configuration (full config)
app.post('/api/devices/:id/config', requireAuth, (req, res) => {
  const { serviceType, stationCode, useCallingAt, extraServices, showStationName, tflLineFilter, tflPlatformFilter } = req.body;

  console.log(`💾 Config update for ${req.params.id}`);

  // First, get the current values to see what actually changed
  db.get('SELECT station_code, service_type, use_calling_at, extra_services, show_station_name, tfl_line_filter, tfl_platform_filter FROM devices WHERE id = ?',
    [req.params.id],
    (err, oldConfig) => {
      if (err) {
        res.status(500).json({ error: err.message });
        return;
      }

      // Update all configuration settings
      db.run(
        `UPDATE devices
         SET station_code = ?, service_type = ?, use_calling_at = ?, extra_services = ?, show_station_name = ?,
             tfl_line_filter = ?, tfl_platform_filter = ?
         WHERE id = ?`,
        [
          stationCode || '',
          serviceType || 'National Rail',
          useCallingAt ? 1 : 0,
          extraServices || 0,
          showStationName ? 1 : 0,
          tflLineFilter || '',
          tflPlatformFilter || '',
          req.params.id
        ],
        (err) => {
          if (err) {
            res.status(500).json({ error: err.message });
          } else {
            // Send command to device
            const ws = wsClients.get(req.params.id);
            if (ws && ws.readyState === WebSocket.OPEN) {
              ws.send(JSON.stringify({
                type: 'command',
                command: 'updateConfig',
                serviceType,
                stationCode,
                useCallingAt,
                extraServices,
                showStationName,
                tflLineFilter,
                tflPlatformFilter
              }));
            }

            // Compare old vs new and log only what changed
            const changes = [];
            if (oldConfig) {
              if (oldConfig.station_code !== stationCode) {
                changes.push(`station: ${oldConfig.station_code || 'none'} → ${stationCode}`);
              }
              if (oldConfig.service_type !== serviceType) {
                changes.push(`service: ${oldConfig.service_type || 'National Rail'} → ${serviceType}`);
              }
              if ((oldConfig.use_calling_at === 1) !== useCallingAt) {
                changes.push(`calling_at: ${oldConfig.use_calling_at ? 'on' : 'off'} → ${useCallingAt ? 'on' : 'off'}`);
              }
              if (oldConfig.extra_services !== extraServices) {
                changes.push(`extra_services: ${oldConfig.extra_services} → ${extraServices}`);
              }
              if ((oldConfig.show_station_name === 1) !== showStationName) {
                changes.push(`show_name: ${oldConfig.show_station_name ? 'on' : 'off'} → ${showStationName ? 'on' : 'off'}`);
              }
              if (oldConfig.tfl_line_filter !== tflLineFilter) {
                changes.push(`tfl_line: ${oldConfig.tfl_line_filter || 'none'} → ${tflLineFilter || 'none'}`);
              }
              if (oldConfig.tfl_platform_filter !== tflPlatformFilter) {
                changes.push(`tfl_platform: ${oldConfig.tfl_platform_filter || 'none'} → ${tflPlatformFilter || 'none'}`);
              }
            }

            const logMessage = changes.length > 0
              ? `Configuration changed: ${changes.join(', ')}`
              : 'Configuration saved (no changes)';

            logEvent(req.params.id, 'config_change', logMessage);
            console.log(`✓ ${logMessage} for ${req.params.id}`);

            // Broadcast update to all web clients
            io.emit('deviceUpdate', {
              deviceId: req.params.id,
              station_code: stationCode,
              service_type: serviceType,
              use_calling_at: useCallingAt ? 1 : 0,
              extra_services: extraServices,
              show_station_name: showStationName ? 1 : 0,
              tfl_line_filter: tflLineFilter || '',
              tfl_platform_filter: tflPlatformFilter || ''
            });

            res.json({ success: true });
          }
        }
      );
    }
  );
});

// Send command to device
app.post('/api/devices/:id/command', requireAuth, (req, res) => {
  const { command, params } = req.body;
  const ws = wsClients.get(req.params.id);
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'command',
      command: command,
      ...params
    }));
    
    logEvent(req.params.id, 'command', `Command sent: ${command}`);
    res.json({ success: true, message: 'Command sent' });
  } else {
    res.status(503).json({ success: false, message: 'Device not connected' });
  }
});

// Restart device
app.post('/api/devices/:id/restart', requireAuth, (req, res) => {
  const ws = wsClients.get(req.params.id);

  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'command',
      command: 'restart'
    }));

    logEvent(req.params.id, 'command', 'Restart command sent');
    res.json({ success: true, message: 'Restart command sent' });
  } else {
    res.status(503).json({ success: false, message: 'Device not connected' });
  }
});

// Request config from device
app.post('/api/devices/:id/getConfig', requireAuth, (req, res) => {
  const deviceId = req.params.id;
  const ws = wsClients.get(deviceId);

  console.log(`📖 Config request for device: ${deviceId}`);
  console.log(`   Tracked devices: [${Array.from(wsClients.keys()).join(', ')}]`);
  console.log(`   WebSocket exists: ${!!ws}`);
  console.log(`   WebSocket state: ${ws ? ws.readyState : 'N/A'} (1 = OPEN)`);

  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'command',
      command: 'getConfig'
    }));

    console.log(`   ✓ Config request sent to device`);
    res.json({ success: true, message: 'Config request sent' });
  } else {
    console.log(`   ❌ Device not connected or WebSocket not OPEN`);
    res.status(503).json({ success: false, message: 'Device not connected' });
  }
});

// Enable log streaming
app.post('/api/devices/:id/enableLogs', requireAuth, (req, res) => {
  const ws = wsClients.get(req.params.id);

  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'command',
      command: 'enableLogs'
    }));

    console.log(`📡 Log streaming enabled for device: ${req.params.id}`);
    res.json({ success: true, message: 'Log streaming enabled' });
  } else {
    res.status(503).json({ success: false, message: 'Device not connected' });
  }
});

// Disable log streaming
app.post('/api/devices/:id/disableLogs', requireAuth, (req, res) => {
  const ws = wsClients.get(req.params.id);

  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({
      type: 'command',
      command: 'disableLogs'
    }));

    console.log(`📡 Log streaming disabled for device: ${req.params.id}`);
    res.json({ success: true, message: 'Log streaming disabled' });
  } else {
    res.status(503).json({ success: false, message: 'Device not connected' });
  }
});

// OTA firmware update
app.post('/api/devices/:id/ota', requireAuth, (req, res) => {
  const { firmwareUrl } = req.body;
  const ws = wsClients.get(req.params.id);
  
  console.log(`\n🔄 OTA Update Request:`);
  console.log(`   Device: ${req.params.id}`);
  console.log(`   Firmware URL: ${firmwareUrl}`);
  console.log(`   WebSocket Connected: ${ws && ws.readyState === WebSocket.OPEN ? 'Yes' : 'No'}`);
  
  if (ws && ws.readyState === WebSocket.OPEN) {
    const otaCommand = {
      type: 'command',
      command: 'ota',
      firmwareUrl: firmwareUrl
    };
    
    console.log(`   Sending command:`, JSON.stringify(otaCommand, null, 2));
    ws.send(JSON.stringify(otaCommand));
    
    logEvent(req.params.id, 'command', `OTA update initiated: ${firmwareUrl}`);
    console.log(`   ✓ OTA command sent to device\n`);
    res.json({ success: true, message: 'OTA update initiated' });
  } else {
    console.log(`   ❌ Device not connected\n`);
    res.status(503).json({ success: false, message: 'Device not connected' });
  }
});

// Get device events
app.get('/api/devices/:id/events', requireAuth, (req, res) => {
  const limit = req.query.limit || 100;
  
  db.all(
    'SELECT * FROM events WHERE device_id = ? ORDER BY timestamp DESC LIMIT ?',
    [req.params.id, limit],
    (err, events) => {
      if (err) {
        res.status(500).json({ error: err.message });
      } else {
        res.json(events);
      }
    }
  );
});

// Upload firmware
app.post('/api/firmware/upload', requireAuth, upload.single('firmware'), (req, res) => {
  const { version } = req.body;
  const { path: tempPath, size } = req.file;

  if (!version) {
    // Delete temp file if version is missing
    fs.unlinkSync(tempPath);
    return res.status(400).json({ error: 'Version is required' });
  }

  const finalFilename = `firmware-${version}.bin`;
  const finalPath = path.join('./firmware', finalFilename);

  // Rename temp file to final filename
  fs.rename(tempPath, finalPath, (err) => {
    if (err) {
      // Clean up temp file on error
      fs.unlinkSync(tempPath);
      return res.status(500).json({ error: 'Failed to save firmware file' });
    }

    // Save to database
    db.run(
      'INSERT INTO firmware (version, filename, upload_date, size) VALUES (?, ?, ?, ?)',
      [version, finalFilename, Date.now(), size],
      (err) => {
        if (err) {
          // Clean up file if database insert fails
          fs.unlinkSync(finalPath);
          res.status(500).json({ error: err.message });
        } else {
          res.json({ success: true, version, filename: finalFilename });
        }
      }
    );
  });
});

// Get firmware list
app.get('/api/firmware', requireAuth, (req, res) => {
  db.all('SELECT * FROM firmware ORDER BY upload_date DESC', (err, firmwares) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(firmwares);
    }
  });
});

// Delete firmware
app.delete('/api/firmware/:id', requireAuth, (req, res) => {
  const firmwareId = req.params.id;

  // First, get the firmware details to know which file to delete
  db.get('SELECT * FROM firmware WHERE id = ?', [firmwareId], (err, firmware) => {
    if (err) {
      return res.status(500).json({ error: err.message });
    }

    if (!firmware) {
      return res.status(404).json({ error: 'Firmware not found' });
    }

    const filePath = path.join('./firmware', firmware.filename);

    // Delete the file from disk
    fs.unlink(filePath, (unlinkErr) => {
      // Continue even if file doesn't exist (may have been manually deleted)
      if (unlinkErr && unlinkErr.code !== 'ENOENT') {
        console.warn(`⚠️  Warning: Could not delete file ${firmware.filename}:`, unlinkErr.message);
      }

      // Delete from database
      db.run('DELETE FROM firmware WHERE id = ?', [firmwareId], (dbErr) => {
        if (dbErr) {
          return res.status(500).json({ error: dbErr.message });
        }

        console.log(`🗑️  Deleted firmware: ${firmware.version} (${firmware.filename})`);
        res.json({ success: true, message: 'Firmware deleted successfully' });
      });
    });
  });
});

// Get statistics
app.get('/api/stats', requireAuth, (req, res) => {
  const stats = {
    totalDevices: 0,
    onlineDevices: 0,
    offlineDevices: 0,
    totalEvents: 0
  };

  db.get('SELECT COUNT(*) as total FROM devices', (err, row) => {
    if (!err) stats.totalDevices = row.total;
    
    db.get('SELECT COUNT(*) as online FROM devices WHERE status = "online"', (err, row) => {
      if (!err) stats.onlineDevices = row.online;
      stats.offlineDevices = stats.totalDevices - stats.onlineDevices;
      
      db.get('SELECT COUNT(*) as total FROM events', (err, row) => {
        if (!err) stats.totalEvents = row.total;
        res.json(stats);
      });
    });
  });
});

// ============================================================================
// Background Tasks
// ============================================================================

// Check for offline devices every 60 seconds
setInterval(() => {
  const threeMinutesAgo = Date.now() - (3 * 60 * 1000);
  
  db.all(
    'SELECT id FROM devices WHERE status = "online" AND last_seen < ?',
    [threeMinutesAgo],
    (err, devices) => {
      if (!err && devices) {
        devices.forEach(device => {
          updateDeviceStatus(device.id, 'offline');
          io.emit('deviceUpdate', {
            deviceId: device.id,
            status: 'offline'
          });
        });
      }
    }
  );
}, 60000);

// ============================================================================
// Server Startup
// ============================================================================

const PORT = process.env.PORT || 3000;

server.listen(PORT, () => {
  console.log('\n===========================================');
  console.log('✅ StationBoards Monitor Server Running');
  console.log('===========================================');
  console.log(`📊 Web Dashboard: http://localhost:${PORT}`);
  console.log(`🌐 WebSocket (Dashboard): ws://localhost:${PORT}/socket.io/`);
  console.log(`📱 WebSocket (ESP32): ws://localhost:${PORT}/ws`);
  console.log('===========================================\n');
});

// Graceful shutdown
process.on('SIGTERM', () => {
  console.log('SIGTERM received, closing server...');
  server.close(() => {
    db.close();
    process.exit(0);
  });
});