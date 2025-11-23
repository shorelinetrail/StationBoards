# StationBoards Remote Monitoring System v2.0

**NEW:** Now uses plain WebSockets for ESP32 devices - more reliable and simpler!

A complete remote monitoring and management system for your StationBoards.co.uk train departure displays.

## What's New in v2.0

✅ **Dual Protocol Support**
- Plain WebSockets (`/ws`) for ESP32 devices - reliable and efficient
- Socket.IO for web dashboard - rich real-time features
- No more connection/disconnection issues!

✅ **Improved Reliability**
- Automatic reconnection
- Better error handling
- More stable connections

## Features

✅ **Real-time Monitoring**
- Live device status (online/offline)
- WiFi signal strength (RSSI)
- Uptime tracking
- Memory usage
- Service counts
- Live display preview

✅ **Remote Management**
- Change station codes
- Adjust display settings
- Update refresh intervals
- Modify scroll/rotation speeds
- Toggle calling points display

✅ **Remote Control**
- Restart devices remotely
- Push OTA firmware updates
- Bulk configuration changes
- Live command execution

✅ **Mobile Responsive**
- Access from phone, tablet, or desktop
- Works on any modern web browser
- Responsive design

✅ **Event Logging**
- Connection history
- Configuration changes
- System events
- Command history

## Quick Start

### 1. Install Node.js

Download from: https://nodejs.org/ (LTS version 18.x or newer)

### 2. Extract and Install

```bash
cd monitoring-system
npm install
```

### 3. Configure Windows Firewall

Allow port 3000:

1. Open **Windows Defender Firewall**
2. **Advanced settings** → **Inbound Rules** → **New Rule**
3. **Port** → **TCP** → **3000** → **Allow**
4. Name: "StationBoards Monitor"

### 4. Find Your PC's IP Address

```bash
ipconfig
```

Look for **IPv4 Address** (e.g., 192.168.1.100)

### 5. Start the Server

```bash
npm start
```

You should see:
```
✅ StationBoards Monitor Server Running
📊 Web Dashboard: http://localhost:3000
🌐 WebSocket (Dashboard): ws://localhost:3000/socket.io/
📱 WebSocket (ESP32): ws://localhost:3000/ws
```

### 6. Access Dashboard

Web browser: `http://localhost:3000`

Or from another device: `http://YOUR-PC-IP:3000`

## ESP32 Firmware Configuration

### Update Your ESP32 Code

In your `main.cpp`, make sure you have these settings:

```cpp
// Monitoring server configuration
String monitorServerHost = "192.168.1.100";  // Your PC's IP
int monitorServerPort = 3000;
bool monitoringEnabled = true;
```

### Connection Setup (already in code)

The ESP32 connects to: `ws://YOUR-PC-IP:3000/ws`

This is a **plain WebSocket** endpoint (not Socket.IO), which is much more reliable for embedded devices!

### Upload Firmware

1. Update the `monitorServerHost` with your PC's IP
2. Compile and upload to ESP32
3. Watch Serial Monitor for connection status

### Expected Serial Output

```
🔌 Connecting to monitoring server...
   Host: 192.168.1.100:3000
✅ Connected to monitoring server
✅ Device registration confirmed
```

## Usage Guide

### View All Devices

Dashboard shows all connected devices with:
- Device name and ID
- Online/offline status
- Station code
- WiFi signal strength
- Last seen time

### Change Device Settings

1. Click device card
2. Go to **Configuration** tab
3. Update settings:
   - Station code
   - Refresh interval
   - Extra services
   - Rotation speed
   - Calling points display
4. Click **Save Configuration**
5. Settings apply immediately!

### Restart a Device

1. Click device card
2. **Actions** tab
3. **Restart Device**
4. Confirm

### Upload New Firmware

1. **Firmware** button (top nav)
2. Enter version (e.g., "2.1.0")
3. Select `.bin` file
4. **Upload**

### Push OTA Update

1. Click device card
2. **Actions** tab
3. **OTA Firmware Update**
4. Select version
5. **Start Update**
6. Device restarts automatically

## Architecture

### Server Components

**Express Server**: Web dashboard and REST API
**Socket.IO**: Real-time web dashboard updates
**WebSocket (`/ws`)**: ESP32 device connections
**SQLite**: Local database for all data

### Communication Flow

```
ESP32 Device ←→ WebSocket (/ws) ←→ Node.js Server ←→ Socket.IO ←→ Web Browser
```

### Message Format (ESP32 → Server)

**Registration:**
```json
{
  "type": "register",
  "deviceId": "ABC123...",
  "name": "Board-ABC123",
  "ip": "192.168.1.50",
  "firmwareVersion": "2.0.0",
  "stationCode": "BHM",
  "stationName": "Birmingham New Street",
  "rssi": -65,
  "uptime": 3600,
  "freeHeap": 180000,
  "services": 5
}
```

**Heartbeat (every 30s):**
```json
{
  "type": "heartbeat",
  "deviceId": "ABC123...",
  "rssi": -65,
  "uptime": 3630,
  "freeHeap": 179500,
  "services": 5
}
```

### Message Format (Server → ESP32)

**Commands:**
```json
{
  "type": "command",
  "command": "updateConfig",
  "stationCode": "MAN",
  "refreshInterval": 90,
  "useCallingAt": true,
  "extraServices": 1,
  "rotationSpeed": 6000
}
```

```json
{
  "type": "command",
  "command": "restart"
}
```

## Running as Windows Service

### Option 1: PM2 (Recommended)

```bash
npm install -g pm2
pm2 start server.js --name stationboards
pm2 startup
pm2 save
```

**Management:**
```bash
pm2 status                  # Check status
pm2 logs stationboards     # View logs
pm2 restart stationboards  # Restart
pm2 stop stationboards     # Stop
```

### Option 2: Windows Task Scheduler

1. **Task Scheduler** → **Create Basic Task**
2. Name: "StationBoards Monitor"
3. Trigger: **At startup**
4. Action: **Start a program**
   - Program: `C:\Program Files\nodejs\node.exe`
   - Arguments: `server.js`
   - Start in: `C:\path\to\monitoring-system`

## Troubleshooting

### ESP32 Can't Connect

**Problem:** `❌ Monitoring server disconnected` immediately

**Solution:**
1. Verify server is running: `npm start`
2. Check firewall allows port 3000
3. Confirm PC IP is correct in ESP32 code
4. Make sure using `/ws` path (not `/socket.io/`)

### Devices Not Appearing

1. Check Serial Monitor for connection messages
2. Verify `monitoringEnabled = true` in ESP32 code
3. Confirm device successfully registered
4. Check server logs for errors

### Dashboard Not Loading

1. Try `http://localhost:3000` first
2. Check if server is running
3. Clear browser cache
4. Check firewall settings

### Connection Drops

1. Check WiFi signal strength (RSSI)
2. Verify router settings (no device isolation)
3. Check for IP conflicts
4. Monitor server logs for errors

## Database Management

### Location

`monitoring-system/stationboards.db`

### Backup

```bash
copy stationboards.db stationboards_backup_%date%.db
```

### Reset

```bash
del stationboards.db
```

The database will be recreated on next server start.

### View Data

Use a SQLite browser: https://sqlitebrowser.org/

## Network Setup

### Local Network Only

Default setup - devices and PC on same network.

### Access from Internet (Advanced)

1. Set static IP for PC
2. Configure router port forwarding:
   - External: 3000 → Internal: PC-IP:3000
3. Access via: `http://YOUR-PUBLIC-IP:3000`

**Security Warning:** Add authentication for public access!

## Performance

### Server Resources

- **CPU**: <5% idle, <20% under load
- **RAM**: 50-200 MB depending on device count
- **Disk**: ~100 MB (database grows with events)

### Device Limits

Tested with up to **200 devices** simultaneously.

### Network Usage

- Per device: ~1 KB/min (heartbeats only)
- Config changes: ~2-5 KB per operation
- OTA updates: Size of firmware file

## Security

### Current Implementation

- Local network only
- No authentication required
- Plain HTTP/WebSocket

### Recommended for Production

1. Add authentication (JWT tokens)
2. Use HTTPS/WSS
3. Implement rate limiting
4. Use environment variables for secrets

## System Requirements

### Server
- Windows 11 (or Windows 10)
- Node.js 18.x or newer
- 4GB RAM minimum
- 500MB disk space

### Clients
- Modern web browser (Chrome, Firefox, Safari, Edge)
- JavaScript enabled

### ESP32
- ESP32 with WiFi
- 4MB+ flash
- Network access to server

## Changelog

### v2.0.0
- ✅ Switched to plain WebSockets for ESP32
- ✅ Improved connection stability
- ✅ Better error handling
- ✅ Cleaner protocol (no Socket.IO overhead on ESP32)
- ✅ Dual protocol support (Socket.IO + WebSocket)

### v1.0.0
- Initial release
- Socket.IO for all connections

## Cloud Deployment (Railway)

### Why Railway?

Railway is perfect for hosting the backend when you want to monitor boards from anywhere:
- ✅ Full WebSocket support
- ✅ Persistent storage for SQLite
- ✅ Automatic HTTPS
- ✅ Simple deployment from GitHub
- ✅ Free tier available

### Deploy to Railway

1. **Push your code to GitHub** (if not already done)

2. **Sign up at Railway**: https://railway.app

3. **Create New Project**:
   - Click "New Project"
   - Choose "Deploy from GitHub repo"
   - Select your StationBoards repository
   - Railway will auto-detect the backend

4. **Configure Root Directory**:
   - Go to Settings → Service Settings
   - Set **Root Directory**: `backend`
   - Railway will use `backend/railway.json` for configuration

5. **Deploy**!
   - Railway automatically installs dependencies
   - Starts server with `node server.js`
   - Assigns a public URL like `https://stationboards-production.up.railway.app`

6. **Configure Your ESP32 Devices**:
   ```cpp
   String monitorServerHost = "stationboards-production.up.railway.app";
   int monitorServerPort = 443;  // HTTPS port
   bool monitoringUseSSL = true;  // Enable SSL
   ```

### Environment Variables on Railway

**Required for Production:**

1. **Set Admin Password** (IMPORTANT!):
   ```bash
   # Generate password hash locally
   node -e "console.log(require('bcryptjs').hashSync('your-secure-password', 10))"
   ```

   In Railway Dashboard → Variables:
   - `ADMIN_PASSWORD_HASH`: Paste the hash from above
   - `ADMIN_USERNAME`: Your username (default: "admin")
   - `SESSION_SECRET`: Random string (e.g., generate with `openssl rand -base64 32`)

2. **Default Credentials** (if not set):
   - Username: `admin`
   - Password: `admin123`
   - ⚠️ **Change immediately in production!**

Railway automatically sets `PORT` - no need to configure it.

### Database Persistence

Railway provides persistent volumes automatically. Your SQLite database will survive restarts and redeployments.

### Monitoring Your Deployment

- View logs in Railway dashboard
- Check deployment status
- Monitor resource usage
- View connected devices at your Railway URL

### Cost

- **Free tier**: Generous limits for hobby use
- **Pro plan**: ~$5-10/month for production use with multiple boards

## Authentication & Security

### Dashboard Login

The dashboard is protected with username/password authentication:
- **Login page**: Automatically shown when accessing the dashboard
- **Session duration**: 7 days (30 days if "Remember me" is checked)
- **Logout**: Click your username in dashboard (coming soon) or clear cookies

### Device Connections

ESP32 devices connect via WebSocket **without authentication**:
- Devices use the `/ws` endpoint (unprotected)
- Only the web dashboard requires login
- This keeps firmware simple while protecting the dashboard

### Changing Your Password

1. **Generate new password hash**:
   ```bash
   node -e "console.log(require('bcryptjs').hashSync('your-new-password', 10))"
   ```

2. **Update on Railway**:
   - Go to your service → Variables
   - Update `ADMIN_PASSWORD_HASH` with the new hash
   - Save (automatic redeploy)

### Security Best Practices

✅ **Always set** a strong password before deploying to production
✅ **Use HTTPS** (Railway provides this automatically)
✅ **Keep URL private** - don't share your Railway URL publicly
✅ **Rotate passwords** periodically
✅ **Monitor access** via Railway logs

## Support & Development

### View Logs

Server logs show in console or with PM2:
```bash
pm2 logs stationboards --lines 100
```

### Debug Mode

```bash
set NODE_ENV=development
npm start
```

### API Endpoints

- `GET /api/devices` - List all devices
- `GET /api/devices/:id` - Get device details
- `POST /api/devices/:id/config` - Update config
- `POST /api/devices/:id/command` - Send command
- `GET /api/devices/:id/events` - Get event history
- `GET /api/firmware` - List firmware versions
- `POST /api/firmware/upload` - Upload firmware
- `GET /api/stats` - System statistics

## License

MIT License - Free to use and modify

## Credits

Built for StationBoards.co.uk
