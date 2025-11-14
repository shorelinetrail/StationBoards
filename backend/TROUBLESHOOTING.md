# Troubleshooting Checklist

Quick checklist to diagnose common issues.

## Server Won't Start

- [ ] Node.js installed? Check: `node --version`
- [ ] Dependencies installed? Run: `npm install`
- [ ] Port 3000 already in use? Try: `netstat -ano | findstr :3000`
- [ ] Check error messages in console

## Can't Access Dashboard

- [ ] Server running? Look for "Server running on port 3000" message
- [ ] Tried http://localhost:3000 ?
- [ ] Firewall blocking? Add port 3000 exception
- [ ] Browser cache? Try Ctrl+F5 or incognito mode
- [ ] Try different browser

## Devices Not Appearing

### Check Server Side
- [ ] Server running and accessible
- [ ] Port 3000 open in firewall
- [ ] Check server console for connection attempts

### Check ESP32 Side
- [ ] WiFi connected? Check Serial Monitor
- [ ] Correct server IP in code?
- [ ] WebSocket library installed?
- [ ] Firmware uploaded successfully?
- [ ] Serial Monitor shows "Connected to monitoring server"?

### Network Issues
- [ ] PC and ESP32 on same network?
- [ ] Router not blocking traffic?
- [ ] PC IP address hasn't changed?
- [ ] Ping test: `ping ESP32_IP_ADDRESS`

## Devices Show Offline

- [ ] Device actually powered on?
- [ ] WiFi connection stable?
- [ ] Check "Last Seen" timestamp
- [ ] Heartbeat interval = 30 seconds (should update frequently)
- [ ] Network interruption?

## Configuration Changes Not Working

- [ ] Device online when sending config?
- [ ] Check device logs in dashboard
- [ ] Serial Monitor shows "Remote config update"?
- [ ] Settings saved on device?
- [ ] Try manual restart after config change

## OTA Update Fails

- [ ] Firmware file uploaded to server?
- [ ] Correct .bin file format?
- [ ] Device has enough free memory? (Check heap)
- [ ] Stable WiFi connection during update?
- [ ] Firmware file size < 5MB?
- [ ] Check Serial Monitor for OTA error messages

## Performance Issues

### Slow Dashboard
- [ ] How many devices? (200 is max recommended)
- [ ] Clear browser cache
- [ ] Check PC RAM usage
- [ ] Restart server

### High Memory on Server
- [ ] Normal for number of devices:
  - 50 devices = ~100 MB
  - 150 devices = ~200 MB
- [ ] Check database size: `dir stationboards.db`
- [ ] Old logs? Database grows over time

### Devices Disconnecting
- [ ] WiFi signal strength? (Check RSSI)
- [ ] Router overloaded?
- [ ] Power supply stable?
- [ ] Check ESP32 heap memory

## Database Issues

### Corrupted Database
```bash
# Backup current
copy stationboards.db stationboards_old.db

# Delete and restart
del stationboards.db
npm start
```

### Reset Everything
```bash
del stationboards.db
rd /s firmware
npm start
```

## Connection Test

### Test from ESP32 Side
Add to ESP32 code temporarily:
```cpp
Serial.println("Testing connection to: " + monitorServerHost);
if (WiFi.ping(monitorServerHost)) {
  Serial.println("✅ Ping successful");
} else {
  Serial.println("❌ Cannot reach server");
}
```

### Test from Server Side
In browser console (F12):
```javascript
// Should connect and show "connected"
const socket = io();
socket.on('connect', () => console.log('✅ Connected'));
```

## Get Help

Still stuck? Check:

1. **Server Console**: Look for error messages
2. **ESP32 Serial Monitor**: See connection attempts
3. **Browser Console** (F12): Check for JavaScript errors
4. **Event Logs**: Device logs tab in dashboard

## Diagnostic Command

In monitoring-system folder:
```bash
# Show what's using port 3000
netstat -ano | findstr :3000

# Show your IP addresses
ipconfig

# Test if server is responding
curl http://localhost:3000/api/devices
```

## Common Error Messages

### "EADDRINUSE: port 3000 already in use"
- Another instance of server running
- Kill process: `taskkill /F /PID <PID>`
- Or use different port in server.js

### "Cannot find module"
- Run: `npm install`

### "ECONNREFUSED"
- Server not running
- Firewall blocking
- Wrong IP/port

### ESP32: "Connection refused"
- Server not accessible
- Wrong IP address
- Firewall blocking
- Port 3000 not open

## Still Need Help?

Document these details:
- Windows version
- Node.js version (`node --version`)
- Number of devices
- Error messages (screenshots)
- What you've already tried
