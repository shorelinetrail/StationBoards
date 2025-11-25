/**
 * Stripe Webhook Handler
 *
 * Handles Stripe webhook events for automatic order status updates
 *
 * Endpoint: /api/stripe-webhook
 * Method: POST
 *
 * Set webhook URL in Stripe Dashboard:
 * https://www.stationboards.co.uk/api/stripe-webhook
 *
 * Events to listen for:
 * - checkout.session.completed
 * - payment_intent.succeeded
 * - payment_intent.payment_failed
 */

import Stripe from 'stripe';
import { createClient } from '@supabase/supabase-js';

const stripe = new Stripe(process.env.STRIPE_SECRET_KEY);
const WEBHOOK_SECRET = process.env.STRIPE_WEBHOOK_SECRET;

const SUPABASE_URL = process.env.NEXT_PUBLIC_SUPABASE_URL;
const SUPABASE_SERVICE_KEY = process.env.SUPABASE_SERVICE_ROLE_KEY;

export const config = {
  api: {
    bodyParser: false, // Stripe needs raw body
  },
};

// Helper to read raw body
async function getRawBody(req) {
  return new Promise((resolve, reject) => {
    let data = '';
    req.on('data', chunk => {
      data += chunk;
    });
    req.on('end', () => {
      resolve(Buffer.from(data));
    });
    req.on('error', reject);
  });
}

export default async function handler(req, res) {
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  const rawBody = await getRawBody(req);
  const sig = req.headers['stripe-signature'];

  let event;

  try {
    // Verify webhook signature
    event = stripe.webhooks.constructEvent(rawBody, sig, WEBHOOK_SECRET);
  } catch (err) {
    console.error('Webhook signature verification failed:', err.message);
    return res.status(400).json({ error: `Webhook Error: ${err.message}` });
  }

  // Handle the event
  try {
    switch (event.type) {
      case 'checkout.session.completed':
        await handleCheckoutCompleted(event.data.object);
        break;

      case 'payment_intent.succeeded':
        await handlePaymentSucceeded(event.data.object);
        break;

      case 'payment_intent.payment_failed':
        await handlePaymentFailed(event.data.object);
        break;

      default:
        console.log(`Unhandled event type: ${event.type}`);
    }

    res.status(200).json({ received: true });

  } catch (error) {
    console.error('Webhook handler error:', error);
    res.status(500).json({ error: 'Webhook handler failed' });
  }
}

// Handle checkout session completed
async function handleCheckoutCompleted(session) {
  const orderId = session.client_reference_id;

  if (!orderId) {
    console.error('No order ID in checkout session');
    return;
  }

  const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

  // Update order status
  await supabase
    .from('orders')
    .update({
      payment_status: 'paid',
      paid_at: new Date().toISOString(),
      status: 'paid'
    })
    .eq('id', orderId);

  // Log payment in order history
  await supabase
    .from('order_history')
    .insert({
      order_id: orderId,
      action: 'payment_received',
      description: `Payment received via Stripe (${session.payment_intent})`,
      performed_by: 'system',
      metadata: {
        stripe_session_id: session.id,
        stripe_payment_intent: session.payment_intent,
        amount: session.amount_total / 100 // Convert from cents
      }
    });

  console.log(`Order ${orderId} marked as paid (Stripe)`);
}

// Handle payment intent succeeded
async function handlePaymentSucceeded(paymentIntent) {
  // This is a backup in case checkout.session.completed doesn't fire
  console.log(`Payment succeeded: ${paymentIntent.id}`);
}

// Handle payment intent failed
async function handlePaymentFailed(paymentIntent) {
  console.error(`Payment failed: ${paymentIntent.id}`, paymentIntent.last_payment_error);

  const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

  // Try to find order by payment intent
  const orderId = paymentIntent.metadata?.order_id;

  if (orderId) {
    await supabase
      .from('order_history')
      .insert({
        order_id: orderId,
        action: 'payment_failed',
        description: `Payment failed: ${paymentIntent.last_payment_error?.message || 'Unknown error'}`,
        performed_by: 'system',
        metadata: {
          stripe_payment_intent: paymentIntent.id,
          error: paymentIntent.last_payment_error
        }
      });
  }
}
