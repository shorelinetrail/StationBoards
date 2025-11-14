# StationBoards Monitoring System - Complete Package

## 📦 What's Included

This complete monitoring system allows you to manage up to 200 StationBoards devices from a central dashboard on your Windows 11 PC.

### Server Components

**Backend (Node.js)**
- `server.js` - Main server application with WebSocket support
- `package.json` - Dependencies and configuration
- SQLite database for data storage

**Frontend (Web Dashboard)**
- `public/index.html` - Responsive dashboard interface
- `public/css/style.css` - Mobile-friendly styling
- `public/js/dashboard.js` - Real-time updates and controls

### Documentation

- `README.md` - Complete installation and usage guide
- `QUICKSTART.md` - Get started in 5 minutes
- `TROUBLESHOOTING.md` - Comprehensive problem-solving guide
- `ESP32_MODIFICATIONS.md` - Firmware integration instructions

## 🚀 Key Features

### Real-Time Monitoring
- Device status (online/offline)
- WiFi signal strength
- Memory usage
- Uptime tracking
- Service counts
- Live updates via WebSocket

### Remote Management
- Change station codes
- Adjust display settings
- Control refresh intervals
- Modify animation speeds
- All changes apply instantly

### Remote Control
- Restart devices remotely
- Push OTA firmware updates
- Bulk operations
- Event logging

### Mobile Responsive
- Works on desktop, tablet, phone
- Touch-friendly interface
- Real-time synchronization

## 📋 Installation Steps

### 1. Server Setup (Windows 11)

```bash
# Install Node.js from nodejs.org

# Navigate to folder
cd monitoring-system

# Install dependencies
npm install

# Start server
npm start
```

### 2. Configure Firewall

- Open port 3000 in Windows Defender Firewall
- Allow inbound connections for Node.js

### 3. Find Your IP Address

```bash
ipconfig
```

Note your IPv4 address (e.g., 192.168.1.100)

### 4. Update ESP32 Firmware

Follow instructions in `ESP32_MODIFICATIONS.md` to:
- Add WebSocket client library
- Add monitoring code
- Set your server IP address
- Upload to devices

### 5. Access Dashboard

Open browser to:
```
http://localhost:3000
```

Or from another device:
```
http://YOUR-IP:3000
```

## 🔧 How It Works

### Architecture

```
ESP32 Devices  ←→  WebSocket  ←→  Node.js Server  ←→  Web Dashboard
                                         ↓
                                   SQLite Database
```

### Communication Flow

1. **Device Registration**
   - ESP32 connects to server via WebSocket
   - Sends device info (ID, IP, firmware, station)
   - Server stores in database

2. **Heartbeat**
   - ESP32 sends status every 30 seconds
   - Includes current metrics (WiFi, memory, services)
   - Server marks offline if no heartbeat for 3 minutes

3. **Remote Commands**
   - Dashboard sends command to server
   - Server forwards to device via WebSocket
   - Device executes and responds

4. **Real-Time Updates**
   - Server pushes changes to dashboard
   - No page refresh needed
   - Instant status updates

## 🎯 Common Use Cases

### Scenario 1: Change Station on Multiple Devices
1. Search for devices in dashboard
2. Click each device
3. Go to Configuration tab
4. Change station code
5. Save - applies immediately

### Scenario 2: Update All Devices to New Firmware
1. Click "Firmware" button
2. Upload new .bin file
3. For each device:
   - Click device → Actions tab
   - OTA Firmware Update
   - Select new version
   - Confirm
4. Devices update and restart automatically

### Scenario 3: Monitor Device Health
1. View device grid
2. Check signal strength (colored icons)
3. View uptime and memory
4. Check logs for issues
5. Restart problematic devices remotely

### Scenario 4: Troubleshoot Offline Device
1. Check "Last Seen" timestamp
2. View logs for disconnect reason
3. If WiFi issue: check signal strength
4. If crashed: check memory usage
5. Remote restart to recover

## 📊 Dashboard Sections

### Main Grid
- Card for each device
- Status indicator (online/offline)
- Key metrics visible
- Click to view details

### Device Details Modal

**Info Tab**
- Device ID and IP
- Firmware version
- Station information
- System metrics

**Configuration Tab**
- Station settings
- Display options
- Timing controls
- Save to apply remotely

**Logs Tab**
- Connection events
- Configuration changes
- System events
- Timestamps

**Actions Tab**
- Restart device
- OTA firmware update
- Emergency controls

### Top Navigation
- Total device count
- Online device count
- Firmware upload
- Search and filters

## 🔐 Security Notes

### Local Network Only (Default)
- Server binds to all interfaces (0.0.0.0)
- Accessible on local network only
- No authentication required (trust local network)

### Internet Access (Optional)
If you want remote access:
1. Set up port forwarding on router
2. Consider adding authentication
3. Use HTTPS (SSL certificate)
4. Implement rate limiting

**Not implemented in this version:**
- User authentication
- HTTPS/SSL
- Rate limiting
- API keys

Add these if exposing to internet!

## 💾 Data Storage

### Database (stationboards.db)
- Device information
- Configuration history
- Event logs
- Firmware metadata

### Firmware Files
Stored in `firmware/` directory

### Backup Strategy
```bash
# Backup database
copy stationboards.db backup\stationboards_YYYYMMDD.db

# Backup firmware
xcopy /E /I firmware backup\firmware
```

## 🔄 Updates and Maintenance

### Update Server
1. Stop server (Ctrl+C)
2. Replace files with new version
3. Run `npm install`
4. Start server `npm start`

### Clean Database
Periodically clean old logs:
```sql
DELETE FROM device_logs WHERE timestamp < strftime('%s', 'now', '-30 days');
```

### Monitor Performance
- Check `stationboards.db` size
- Monitor RAM usage
- Review server console logs

## 📱 Mobile Access

### Same Network
Just open `http://YOUR-IP:3000` on mobile browser

### Remote Access
1. Set up port forwarding
2. Use dynamic DNS (if IP changes)
3. Access via `http://YOUR-PUBLIC-IP:3000`

## 🎓 Learning Resources

### Technologies Used
- **Node.js**: JavaScript runtime
- **Express**: Web server framework
- **Socket.IO**: WebSocket library
- **SQLite**: Database
- **Bootstrap**: UI framework

### Customization Ideas
- Add user authentication
- Implement email alerts
- Create mobile app
- Add data visualization
- Export reports
- Backup automation

## 📞 Support

### Self-Help
1. Check TROUBLESHOOTING.md
2. Review server console logs
3. Check ESP32 Serial Monitor
4. View device logs in dashboard

### Files to Check When Debugging
- Server console output
- `stationboards.db` (SQLite Browser)
- Browser console (F12)
- ESP32 Serial Monitor

## ✅ Pre-Flight Checklist

Before deploying:
- [ ] Node.js installed
- [ ] Dependencies installed (`npm install`)
- [ ] Firewall configured (port 3000)
- [ ] IP address noted
- [ ] ESP32 code updated with IP
- [ ] Server starts successfully
- [ ] Dashboard accessible
- [ ] Test with one device first

## 🎉 Success Indicators

You'll know it's working when:
- Server shows "running on port 3000"
- Dashboard loads at http://localhost:3000
- ESP32 Serial shows "Connected to monitoring server"
- Device appears in dashboard within 30 seconds
- Status updates in real-time
- Configuration changes apply instantly
- Device responds to restart command

## 📦 Package Contents

```
monitoring-system/
├── server.js                  # Main server application
├── package.json              # Dependencies
├── README.md                 # Full documentation
├── QUICKSTART.md            # 5-minute setup guide
├── TROUBLESHOOTING.md       # Problem solving
├── ESP32_MODIFICATIONS.md   # Firmware instructions
├── public/                   # Web dashboard
│   ├── index.html           # Dashboard UI
│   ├── css/
│   │   └── style.css        # Styling
│   └── js/
│       └── dashboard.js     # Frontend logic
└── firmware/                # OTA firmware storage (created automatically)
```

---

**Version:** 1.0.0  
**Compatible with:** StationBoards Firmware 2.0.0+  
**Platform:** Windows 11, Node.js 18+  
**License:** MIT
