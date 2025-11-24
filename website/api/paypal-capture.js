/**
 * PayPal Payment Capture
 *
 * Captures PayPal payment after customer approval
 *
 * Endpoint: /api/paypal-capture?token=PAYPAL_ORDER_ID&order_id=UUID
 * Method: GET (redirect from PayPal)
 */

import { createClient } from '@supabase/supabase-js';

const PAYPAL_API_BASE = process.env.PAYPAL_MODE === 'live'
  ? 'https://api-m.paypal.com'
  : 'https://api-m.sandbox.paypal.com';

const PAYPAL_CLIENT_ID = process.env.PAYPAL_CLIENT_ID;
const PAYPAL_CLIENT_SECRET = process.env.PAYPAL_CLIENT_SECRET;
const WEBSITE_URL = process.env.WEBSITE_URL || 'https://www.stationboards.co.uk';

const SUPABASE_URL = process.env.NEXT_PUBLIC_SUPABASE_URL;
const SUPABASE_SERVICE_KEY = process.env.SUPABASE_SERVICE_ROLE_KEY;

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
  const { token, order_id } = req.query;

  if (!token || !order_id) {
    return res.redirect(`${WEBSITE_URL}/index.html?error=missing_parameters`);
  }

  try {
    const accessToken = await getPayPalAccessToken();

    // Capture the payment
    const response = await fetch(`${PAYPAL_API_BASE}/v2/checkout/orders/${token}/capture`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${accessToken}`,
        'Content-Type': 'application/json'
      }
    });

    const captureData = await response.json();

    if (!response.ok || captureData.status !== 'COMPLETED') {
      throw new Error('Payment capture failed');
    }

    // Update order in database
    const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

    // Get order details
    const { data: order, error: orderError } = await supabase
      .from('orders')
      .select('*')
      .eq('id', order_id)
      .single();

    if (orderError) {
      throw new Error('Order not found');
    }

    // Update order status
    await supabase
      .from('orders')
      .update({
        payment_status: 'paid',
        paid_at: new Date().toISOString(),
        status: 'paid'
      })
      .eq('id', order_id);

    // Log in order history
    await supabase
      .from('order_history')
      .insert({
        order_id: order_id,
        action: 'payment_received',
        description: `Payment received via PayPal (${captureData.id})`,
        performed_by: 'system',
        metadata: {
          paypal_order_id: token,
          paypal_capture_id: captureData.id,
          amount: captureData.purchase_units[0].payments.captures[0].amount.value
        }
      });

    // Redirect to success page
    return res.redirect(`${WEBSITE_URL}/order-success.html?order=${encodeURIComponent(order.order_number)}&payment=paypal`);

  } catch (error) {
    console.error('PayPal capture error:', error);
    return res.redirect(`${WEBSITE_URL}/index.html?error=payment_failed&message=${encodeURIComponent(error.message)}`);
  }
}
