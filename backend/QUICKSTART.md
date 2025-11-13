# Quick Start Guide

Get your StationBoards monitoring system running in 5 minutes!

## 1. Install Node.js (2 minutes)

Download and install from: https://nodejs.org/

Choose the LTS version.

## 2. Setup Server (2 minutes)

Open Command Prompt or PowerShell:

```bash
cd monitoring-system
npm install
```

## 3. Find Your IP Address (30 seconds)

In Command Prompt:
```bash
ipconfig
```

Look for "IPv4 Address" (e.g., 192.168.1.100)

## 4. Start Server (10 seconds)

```bash
npm start
```

## 5. Open Dashboard (10 seconds)

Open browser to:
```
http://localhost:3000
```

## 6. Configure ESP32 Devices (1 minute per device)

Edit your ESP32 code:
```cpp
String monitorServerHost = "192.168.1.100";  // Your IP here
```

Upload to devices.

## Done! 🎉

Your devices should now appear on the dashboard within 30 seconds.

## Common Issues

**Firewall blocking?**
- Windows Defender → Allow port 3000

**Devices not showing?**
- Check ESP32 Serial Monitor
- Verify IP address is correct
- Make sure server is running

**Can't access dashboard?**
- Try http://localhost:3000 first
- Check if server is running

## Next Steps

- Upload firmware via the dashboard
- Configure device settings remotely
- Set up PM2 for auto-start (see README.md)
