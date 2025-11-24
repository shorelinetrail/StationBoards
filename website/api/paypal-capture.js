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

  console.log('PayPal capture initiated:', { token, order_id });

  if (!token || !order_id) {
    console.error('Missing parameters:', { token, order_id });
    return res.redirect(`${WEBSITE_URL}/index.html?error=missing_parameters`);
  }

  let captureData = null;
  let order = null;

  try {
    // Get PayPal access token
    console.log('Getting PayPal access token...');
    const accessToken = await getPayPalAccessToken();

    if (!accessToken) {
      throw new Error('Failed to get PayPal access token');
    }

    // Capture the payment
    console.log('Capturing payment for order:', token);
    const response = await fetch(`${PAYPAL_API_BASE}/v2/checkout/orders/${token}/capture`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${accessToken}`,
        'Content-Type': 'application/json'
      }
    });

    // Parse response
    try {
      captureData = await response.json();
      console.log('PayPal capture response:', JSON.stringify(captureData, null, 2));
    } catch (parseError) {
      console.error('Failed to parse PayPal response:', parseError);
      throw new Error('PayPal returned invalid response');
    }

    if (!response.ok) {
      console.error('PayPal capture failed:', { status: response.status, data: captureData });
      const errorMsg = captureData?.message || captureData?.details?.[0]?.description || 'Payment capture failed';
      throw new Error(errorMsg);
    }

    // Check if payment was completed
    const captureStatus = captureData.status;
    console.log('Payment capture status:', captureStatus);

    if (captureStatus !== 'COMPLETED' && captureStatus !== 'APPROVED') {
      console.error('Unexpected capture status:', captureStatus);
      throw new Error(`Payment status: ${captureStatus}`);
    }

    // Update order in database
    console.log('Updating order in database:', order_id);
    const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

    // Get order details
    const { data: orderData, error: orderError } = await supabase
      .from('orders')
      .select('*')
      .eq('id', order_id)
      .single();

    if (orderError || !orderData) {
      console.error('Order not found:', orderError);
      throw new Error('Order not found in database');
    }

    order = orderData;
    console.log('Found order:', order.order_number);

    // Update order status
    const { error: updateError } = await supabase
      .from('orders')
      .update({
        payment_status: 'paid',
        paid_at: new Date().toISOString(),
        status: 'paid'
      })
      .eq('id', order_id);

    if (updateError) {
      console.error('Failed to update order status:', updateError);
      throw new Error('Failed to update order status');
    }

    console.log('Order status updated to paid');

    // Log in order history
    try {
      const captureAmount = captureData.purchase_units?.[0]?.payments?.captures?.[0]?.amount?.value;

      const { error: historyError } = await supabase
        .from('order_history')
        .insert({
          order_id: order_id,
          action: 'payment_received',
          description: `Payment received via PayPal (${captureData.id})`,
          performed_by: 'system',
          metadata: {
            paypal_order_id: token,
            paypal_capture_id: captureData.id,
            amount: captureAmount
          }
        });

      if (historyError) {
        console.error('Failed to log order history:', historyError);
        // Don't throw - order is already marked as paid
      } else {
        console.log('Order history logged successfully');
      }
    } catch (historyError) {
      console.error('Error logging order history:', historyError);
      // Don't throw - order is already marked as paid
    }

    // Redirect to success page
    console.log('Redirecting to success page for order:', order.order_number);
    return res.redirect(`${WEBSITE_URL}/order-success.html?order=${encodeURIComponent(order.order_number)}&payment=paypal`);

  } catch (error) {
    console.error('PayPal capture error:', error);
    console.error('Error stack:', error.stack);

    // If we have order info, try to redirect with order number
    if (order && order.order_number) {
      return res.redirect(`${WEBSITE_URL}/index.html?error=payment_processing&order=${encodeURIComponent(order.order_number)}&message=${encodeURIComponent(error.message)}`);
    }

    // Otherwise generic error redirect
    return res.redirect(`${WEBSITE_URL}/index.html?error=payment_failed&message=${encodeURIComponent(error.message)}`);
  }
}
