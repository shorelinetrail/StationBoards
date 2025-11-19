const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const WebSocket = require('ws');
const sqlite3 = require('sqlite3').verbose();
const multer = require('multer');
const path = require('path');
const fs = require('fs');

const app = express();
const server = http.createServer(app);

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

// Middleware
app.use(express.json());
app.use(express.static('public'));
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
    cb(null, `firmware-${req.body.version}.bin`);
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
      console.log('📨 Received from device:', message.type);

      switch (message.type) {
        case 'register':
          handleDeviceRegister(ws, message);
          deviceId = message.deviceId;
          
          // Store WebSocket connection
          wsClients.set(deviceId, ws);
          
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
     extra_services, rotation_speed, refresh_interval, scroll_speed, show_station_name)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 'online',
            COALESCE((SELECT first_seen FROM devices WHERE id = ?), ?),
            COALESCE((SELECT use_calling_at FROM devices WHERE id = ?), 1),
            COALESCE((SELECT extra_services FROM devices WHERE id = ?), 0),
            COALESCE((SELECT rotation_speed FROM devices WHERE id = ?), 5000),
            COALESCE((SELECT refresh_interval FROM devices WHERE id = ?), 60),
            COALESCE((SELECT scroll_speed FROM devices WHERE id = ?), 50),
            COALESCE((SELECT show_station_name FROM devices WHERE id = ?), 1))
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
    data.deviceId,
    data.deviceId,
    data.deviceId,
    data.deviceId,
    data.deviceId,
    data.deviceId
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
    } else if (data.stationCode || data.stationName) {
      // Broadcast station update to web clients
      io.emit('deviceUpdate', {
        deviceId: data.deviceId,
        station_code: data.stationCode,
        station_name: data.stationName,
        service_type: data.serviceType
      });
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
// REST API Endpoints
// ============================================================================

// Get server info (for constructing firmware URLs)
app.get('/api/server-info', (req, res) => {
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
  
  res.json({
    ip: serverIp,
    port: port,
    baseUrl: `http://${serverIp}:${port}`,
    allAddresses: addresses
  });
});

// Get all devices
app.get('/api/devices', (req, res) => {
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
app.get('/api/devices/:id', (req, res) => {
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
              rssi: device.rssi,
              uptime: device.uptime,
              free_heap: device.free_heap,
              services_count: device.services,
              last_seen: Math.floor(device.last_seen / 1000), // Convert ms to seconds
              status: device.status
            },
            config: {
              station_code: device.station_code,
              refresh_interval: device.refresh_interval,
              use_calling_at: device.use_calling_at === 1,
              show_station_name: device.show_station_name === 1,
              extra_services: device.extra_services,
              scroll_speed: device.scroll_speed || 50,
              // Convert rotation_speed from milliseconds to seconds
              // Handle edge cases: NULL, 0, or values already in seconds
              rotation_speed: (() => {
                const rs = device.rotation_speed;
                // NULL or 0 → use default 5 seconds
                if (!rs || rs === 0) return 5;
                // Value already in seconds (< 100) → use as-is
                if (rs < 100) return rs;
                // Value in milliseconds (>= 1000) → convert to seconds
                if (rs >= 1000) return rs / 1000;
                // Ambiguous range (100-999) → assume seconds, use as-is
                return rs;
              })()
            },
            logs: formattedLogs
          });
        }
      );
    }
  });
});

// Update device configuration
app.post('/api/devices/:id/config', (req, res) => {
  const { stationCode, useCallingAt, extraServices, rotationSpeed, refreshInterval, scrollSpeed, showStationName } = req.body;
  
  // Validate rotation speed is in expected range (5-60 seconds)
  const rotationSpeedSeconds = parseInt(rotationSpeed);
  if (isNaN(rotationSpeedSeconds) || rotationSpeedSeconds < 5 || rotationSpeedSeconds > 60) {
    return res.status(400).json({ error: 'Rotation speed must be between 5 and 60 seconds' });
  }
  
  // Convert to milliseconds for storage and device
  const rotationSpeedMs = rotationSpeedSeconds * 1000;
  
  console.log(`💾 Config update for ${req.params.id}: rotation_speed ${rotationSpeedSeconds}s (${rotationSpeedMs}ms)`);
  
  // First, get the current values to see what actually changed
  db.get('SELECT station_code, use_calling_at, extra_services, rotation_speed, refresh_interval, scroll_speed, show_station_name FROM devices WHERE id = ?', 
    [req.params.id], 
    (err, oldConfig) => {
      if (err) {
        res.status(500).json({ error: err.message });
        return;
      }
      
      // Now update the database
      db.run(
        `UPDATE devices 
         SET station_code = ?, use_calling_at = ?, extra_services = ?, 
             rotation_speed = ?, refresh_interval = ?, scroll_speed = ?, show_station_name = ?
         WHERE id = ?`,
        [stationCode, useCallingAt ? 1 : 0, extraServices, rotationSpeedMs, refreshInterval, scrollSpeed, showStationName ? 1 : 0, req.params.id],
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
                stationCode,
                useCallingAt,
                extraServices,
                rotationSpeed: rotationSpeedMs, // Send in milliseconds
                refreshInterval,
                scrollSpeed,
                showStationName
              }));
            }
            
            // Compare old vs new and log only what changed
            const changes = [];
            if (oldConfig) {
              if (oldConfig.station_code !== stationCode) {
                changes.push(`station: ${oldConfig.station_code || 'none'} → ${stationCode}`);
              }
              if ((oldConfig.use_calling_at === 1) !== useCallingAt) {
                changes.push(`calling_at: ${oldConfig.use_calling_at ? 'on' : 'off'} → ${useCallingAt ? 'on' : 'off'}`);
              }
              if (oldConfig.extra_services !== extraServices) {
                changes.push(`extra_services: ${oldConfig.extra_services} → ${extraServices}`);
              }
              if (oldConfig.rotation_speed !== rotationSpeedMs) {
                const oldSec = oldConfig.rotation_speed ? oldConfig.rotation_speed / 1000 : 0;
                changes.push(`rotation: ${oldSec}s → ${rotationSpeedSeconds}s`);
              }
              if (oldConfig.refresh_interval !== refreshInterval) {
                changes.push(`refresh: ${oldConfig.refresh_interval}s → ${refreshInterval}s`);
              }
              if (oldConfig.scroll_speed !== scrollSpeed) {
                changes.push(`scroll_speed: ${oldConfig.scroll_speed}ms → ${scrollSpeed}ms`);
              }
              if ((oldConfig.show_station_name === 1) !== showStationName) {
                changes.push(`show_name: ${oldConfig.show_station_name ? 'on' : 'off'} → ${showStationName ? 'on' : 'off'}`);
              }
            }
            
            const logMessage = changes.length > 0 
              ? `Configuration changed: ${changes.join(', ')}`
              : 'Configuration saved (no changes)';
            
            logEvent(req.params.id, 'config_change', logMessage);
            console.log(`✓ ${logMessage} for ${req.params.id}`);
            
            // Broadcast update to all web clients (keeping milliseconds for consistency with DB)
            io.emit('deviceUpdate', {
              deviceId: req.params.id,
              station_code: stationCode,
              use_calling_at: useCallingAt ? 1 : 0,
              extra_services: extraServices,
              rotation_speed: rotationSpeedMs, // Store in milliseconds
              refresh_interval: refreshInterval,
              scroll_speed: scrollSpeed,
              show_station_name: showStationName ? 1 : 0
            });
            
            res.json({ success: true });
          }
        }
      );
    }
  );
});

// Send command to device
app.post('/api/devices/:id/command', (req, res) => {
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
app.post('/api/devices/:id/restart', (req, res) => {
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

// OTA firmware update
app.post('/api/devices/:id/ota', (req, res) => {
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
app.get('/api/devices/:id/events', (req, res) => {
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
app.post('/api/firmware/upload', upload.single('firmware'), (req, res) => {
  const { version } = req.body;
  const { filename, size } = req.file;

  db.run(
    'INSERT INTO firmware (version, filename, upload_date, size) VALUES (?, ?, ?, ?)',
    [version, filename, Date.now(), size],
    (err) => {
      if (err) {
        res.status(500).json({ error: err.message });
      } else {
        res.json({ success: true, version, filename });
      }
    }
  );
});

// Get firmware list
app.get('/api/firmware', (req, res) => {
  db.all('SELECT * FROM firmware ORDER BY upload_date DESC', (err, firmwares) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(firmwares);
    }
  });
});

// Get statistics
app.get('/api/stats', (req, res) => {
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