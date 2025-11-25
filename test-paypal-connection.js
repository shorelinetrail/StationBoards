// Test PayPal Connection
// Run this in browser console or as a Node script

async function testPayPalConnection() {
  console.log('Testing PayPal Sandbox Connection...\n');

  const PAYPAL_CLIENT_ID = 'AZmOy44btg2DhXHw9nBi0cOT4GHmrk0EJPoazLyRaBJVLuVWWqi3zPFj4j7xIVtqtCaDYBmSG0Dlb35u';
  const PAYPAL_CLIENT_SECRET = 'EI-KYBEvcvCvvIB250IzWB64Ad5O9uiObgadun9W997n7VyB5O5OraoyoXhPgt6sDaF7pDogqDopomKr';
  const PAYPAL_API_BASE = 'https://api-m.sandbox.paypal.com';

  try {
    // Step 1: Test Authentication
    console.log('1. Testing authentication...');
    const auth = btoa(`${PAYPAL_CLIENT_ID}:${PAYPAL_CLIENT_SECRET}`);

    const authResponse = await fetch(`${PAYPAL_API_BASE}/v1/oauth2/token`, {
      method: 'POST',
      headers: {
        'Authorization': `Basic ${auth}`,
        'Content-Type': 'application/x-www-form-urlencoded'
      },
      body: 'grant_type=client_credentials'
    });

    console.log('Auth response status:', authResponse.status);

    if (!authResponse.ok) {
      const errorData = await authResponse.json();
      console.error('❌ Authentication failed:', errorData);
      return;
    }

    const authData = await authResponse.json();
    console.log('✅ Authentication successful');
    console.log('Access token received:', authData.access_token.substring(0, 20) + '...\n');

    // Step 2: Test Creating an Order
    console.log('2. Testing order creation...');
    const orderResponse = await fetch(`${PAYPAL_API_BASE}/v2/checkout/orders`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${authData.access_token}`,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        intent: 'CAPTURE',
        purchase_units: [{
          amount: {
            currency_code: 'GBP',
            value: '89.00'
          },
          description: 'Test StationBoard Order',
          shipping: {
            name: { full_name: 'Test Customer' },
            address: {
              address_line_1: '123 Test Street',
              admin_area_2: 'London',
              postal_code: 'SW1A 1AA',
              country_code: 'GB'
            }
          }
        }],
        application_context: {
          brand_name: 'StationBoards',
          shipping_preference: 'SET_PROVIDED_ADDRESS',
          user_action: 'PAY_NOW',
          return_url: 'https://www.stationboards.co.uk/success',
          cancel_url: 'https://www.stationboards.co.uk/cancel'
        }
      })
    });

    console.log('Order response status:', orderResponse.status);

    const orderData = await orderResponse.json();

    if (!orderResponse.ok) {
      console.error('❌ Order creation failed:', orderData);
      return;
    }

    console.log('✅ Order created successfully');
    console.log('Order ID:', orderData.id);
    console.log('Approval URL:', orderData.links.find(l => l.rel === 'approve')?.href);
    console.log('\n✅ All tests passed! PayPal integration is working.');

  } catch (error) {
    console.error('❌ Test failed with error:', error.message);
    console.error('Full error:', error);
  }
}

// Run the test
testPayPalConnection();
