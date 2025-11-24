/**
 * Create PayPal Order
 *
 * Creates a PayPal order for payment
 *
 * Endpoint: /api/create-paypal-order
 * Method: POST
 * Body: {
 *   order_id: "uuid",
 *   order_number: "SB-20250124-A7F2",
 *   customer_email: "customer@example.com",
 *   customer_name: "John Doe",
 *   quantity: 1,
 *   total_price: 89.00
 * }
 */

const PAYPAL_API_BASE = process.env.PAYPAL_MODE === 'live'
  ? 'https://api-m.paypal.com'
  : 'https://api-m.sandbox.paypal.com';

const PAYPAL_CLIENT_ID = process.env.PAYPAL_CLIENT_ID;
const PAYPAL_CLIENT_SECRET = process.env.PAYPAL_CLIENT_SECRET;
const WEBSITE_URL = process.env.WEBSITE_URL || 'https://www.stationboards.co.uk';

// Get PayPal access token
async function getPayPalAccessToken() {
  const auth = Buffer.from(`${PAYPAL_CLIENT_ID}:${PAYPAL_CLIENT_SECRET}`).toString('base64');

  const response = await fetch(`${PAYPAL_API_BASE}/v1/oauth2/token`, {
    method: 'POST',
    headers: {
      'Authorization': `Basic ${auth}`,
      'Content-Type': 'application/x-www-form-urlencoded'
    },
    body: 'grant_type=client_credentials'
  });

  const data = await response.json();
  return data.access_token;
}

export default async function handler(req, res) {
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  // Check if PayPal credentials are configured
  if (!PAYPAL_CLIENT_ID || !PAYPAL_CLIENT_SECRET) {
    console.error('PayPal credentials not configured');
    return res.status(500).json({
      success: false,
      error: 'PayPal payment is not configured. Please contact support or use card payment.',
      details: 'Missing PayPal API credentials'
    });
  }

  const {
    order_id,
    order_number,
    customer_email,
    customer_name,
    quantity,
    total_price,
    shipping_address,
    shipping_city,
    shipping_postcode,
    shipping_country
  } = req.body;

  // Validate required fields
  if (!order_id || !order_number || !customer_email || !total_price || !shipping_address || !shipping_city || !shipping_postcode) {
    return res.status(400).json({
      success: false,
      error: 'Missing required fields',
      required: ['order_id', 'order_number', 'customer_email', 'total_price', 'shipping_address', 'shipping_city', 'shipping_postcode']
    });
  }

  try {
    const accessToken = await getPayPalAccessToken();

    // Create PayPal order
    const response = await fetch(`${PAYPAL_API_BASE}/v2/checkout/orders`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${accessToken}`,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        intent: 'CAPTURE',
        purchase_units: [
          {
            reference_id: order_id,
            description: `StationBoard Order ${order_number}`,
            custom_id: order_id,
            soft_descriptor: 'STATIONBOARDS',
            amount: {
              currency_code: 'GBP',
              value: parseFloat(total_price).toFixed(2),
              breakdown: {
                item_total: {
                  currency_code: 'GBP',
                  value: parseFloat(total_price).toFixed(2)
                }
              }
            },
            items: [
              {
                name: 'StationBoard - Live Departure Display',
                description: `${quantity} board${quantity > 1 ? 's' : ''}`,
                unit_amount: {
                  currency_code: 'GBP',
                  value: (parseFloat(total_price) / quantity).toFixed(2)
                },
                quantity: quantity.toString(),
                category: 'PHYSICAL_GOODS'
              }
            ],
            shipping: {
              name: {
                full_name: customer_name || 'Customer'
              },
              address: {
                address_line_1: shipping_address.split('\n')[0] || shipping_address,
                address_line_2: shipping_address.split('\n')[1] || undefined,
                admin_area_2: shipping_city,
                postal_code: shipping_postcode,
                country_code: shipping_country === 'United Kingdom' ? 'GB' : 'IE'
              }
            }
          }
        ],
        application_context: {
          brand_name: 'StationBoards',
          landing_page: 'BILLING',
          shipping_preference: 'SET_PROVIDED_ADDRESS',
          user_action: 'PAY_NOW',
          return_url: `${WEBSITE_URL}/api/paypal-capture?order_id=${encodeURIComponent(order_id)}`,
          cancel_url: `${WEBSITE_URL}/index.html?payment_cancelled=${encodeURIComponent(order_number)}#order`
        }
      })
    });

    const orderData = await response.json();

    if (!response.ok) {
      console.error('PayPal API error:', orderData);
      const errorMessage = orderData.details?.[0]?.description || orderData.message || 'PayPal order creation failed';
      throw new Error(errorMessage);
    }

    // Find approval URL
    const approvalUrl = orderData.links?.find(link => link.rel === 'approve')?.href;

    if (!approvalUrl) {
      console.error('No approval URL in PayPal response:', orderData);
      throw new Error('PayPal did not return an approval URL');
    }

    return res.status(200).json({
      success: true,
      paypal_order_id: orderData.id,
      approval_url: approvalUrl
    });

  } catch (error) {
    console.error('PayPal order creation error:', error);
    console.error('Error stack:', error.stack);
    return res.status(500).json({
      success: false,
      error: error.message || 'Failed to create PayPal order',
      details: process.env.NODE_ENV === 'development' ? error.stack : undefined
    });
  }
}
