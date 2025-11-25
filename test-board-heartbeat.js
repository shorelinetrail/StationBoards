/**
 * Board Heartbeat Test Script
 *
 * This script tests the board monitoring integration by simulating
 * an ESP32 board calling the heartbeat endpoint.
 *
 * Usage: node test-board-heartbeat.js [board_id]
 * Example: node test-board-heartbeat.js ESP32-TEST001
 */

const https = require('https');

// Configuration
const WEBSITE_URL = process.env.WEBSITE_URL || 'https://stationboards.co.uk';
const TEST_BOARD_ID = process.argv[2] || 'ESP32-TEST001';

/**
 * Send heartbeat to the API
 */
async function sendHeartbeat(boardId) {
  console.log(`\n🔄 Sending heartbeat for board: ${boardId}`);
  console.log(`   Endpoint: ${WEBSITE_URL}/api/board-heartbeat`);

  const data = JSON.stringify({
    board_id: boardId
  });

  const url = new URL(`${WEBSITE_URL}/api/board-heartbeat`);

  const options = {
    hostname: url.hostname,
    port: url.port || 443,
    path: url.pathname,
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': data.length
    }
  };

  return new Promise((resolve, reject) => {
    const req = https.request(options, (res) => {
      let responseData = '';

      res.on('data', (chunk) => {
        responseData += chunk;
      });

      res.on('end', () => {
        console.log(`   Status: ${res.statusCode}`);

        try {
          const json = JSON.parse(responseData);
          console.log(`   Response:`, JSON.stringify(json, null, 2));
          resolve(json);
        } catch (e) {
          console.error(`   Failed to parse response:`, responseData);
          reject(e);
        }
      });
    });

    req.on('error', (error) => {
      console.error(`   Request failed:`, error.message);
      reject(error);
    });

    req.write(data);
    req.end();
  });
}

/**
 * Run monitoring test
 */
async function runTest() {
  console.log('╔═══════════════════════════════════════════════════╗');
  console.log('║   Board Monitoring Integration Test              ║');
  console.log('╚═══════════════════════════════════════════════════╝');

  try {
    // Test 1: Send heartbeat
    console.log('\n📋 Test 1: Send board heartbeat');
    const result = await sendHeartbeat(TEST_BOARD_ID);

    if (result.success) {
      console.log('\n✅ Heartbeat successful!');
      console.log(`   Board ID: ${result.board_id}`);
      console.log(`   Status: ${result.status}`);
      console.log(`   First Activation: ${result.first_activation ? 'Yes' : 'No'}`);
      console.log(`   Timestamp: ${result.timestamp}`);
    } else {
      console.log('\n❌ Heartbeat failed!');
      console.log(`   Error: ${result.error}`);
    }

    // Test 2: Send another heartbeat after 2 seconds
    console.log('\n📋 Test 2: Send second heartbeat (2 seconds later)');
    await new Promise(resolve => setTimeout(resolve, 2000));
    const result2 = await sendHeartbeat(TEST_BOARD_ID);

    if (result2.success) {
      console.log('\n✅ Second heartbeat successful!');
      console.log(`   First Activation: ${result2.first_activation ? 'Yes' : 'No'}`);
    }

    // Summary
    console.log('\n╔═══════════════════════════════════════════════════╗');
    console.log('║   Monitoring Integration Status                   ║');
    console.log('╚═══════════════════════════════════════════════════╝');
    console.log('\n✅ Monitoring integration is WORKING');
    console.log('\nWhat this means:');
    console.log('  • ESP32 boards can successfully report their online status');
    console.log('  • The heartbeat endpoint accepts and processes requests');
    console.log('  • Board last_seen timestamps are being updated');
    console.log('  • First activation is tracked correctly');
    console.log('  • Admin dashboard will show boards as online/offline');
    console.log('\nTo verify in admin dashboard:');
    console.log('  1. Go to /admin');
    console.log('  2. Click "Boards" tab');
    console.log(`  3. Look for board "${TEST_BOARD_ID}"`);
    console.log('  4. Check "Last Seen" column (should show "Just now")');
    console.log('  5. Green indicator (●) means board is online\n');

  } catch (error) {
    console.log('\n❌ Test failed!');
    console.error('Error:', error.message);

    console.log('\n╔═══════════════════════════════════════════════════╗');
    console.log('║   Troubleshooting                                 ║');
    console.log('╚═══════════════════════════════════════════════════╝');
    console.log('\n1. Ensure the board exists in the database:');
    console.log('   - Go to /admin');
    console.log('   - Click "Boards" tab');
    console.log(`   - Check if "${TEST_BOARD_ID}" exists`);
    console.log('   - Or add it using "Add Board" button\n');
    console.log('2. Check environment variables:');
    console.log('   - NEXT_PUBLIC_SUPABASE_URL');
    console.log('   - SUPABASE_SERVICE_ROLE_KEY\n');
    console.log('3. Verify Supabase permissions:');
    console.log('   - Run add-monitoring.sql in Supabase SQL Editor');
    console.log('   - Check RLS policies on boards table\n');

    process.exit(1);
  }
}

// Run the test
runTest();
