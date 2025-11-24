/**
 * Create Stripe Checkout Session
 *
 * Creates a Stripe checkout session for order payment
 *
 * Endpoint: /api/create-checkout-session
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

import Stripe from 'stripe';

const stripe = new Stripe(process.env.STRIPE_SECRET_KEY);
const WEBSITE_URL = process.env.WEBSITE_URL || 'https://www.stationboards.co.uk';

export default async function handler(req, res) {
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  const {
    order_id,
    order_number,
    customer_email,
    customer_name,
    quantity,
    total_price
  } = req.body;

  // Validate required fields
  if (!order_id || !order_number || !customer_email || !total_price) {
    return res.status(400).json({
      success: false,
      error: 'Missing required fields'
    });
  }

  try {
    // Create Stripe checkout session
    const session = await stripe.checkout.sessions.create({
      payment_method_types: ['card'],
      line_items: [
        {
          price_data: {
            currency: 'gbp',
            product_data: {
              name: 'StationBoard - Live Departure Display',
              description: `Order ${order_number} - ${quantity} board${quantity > 1 ? 's' : ''}`,
              images: [`${WEBSITE_URL}/images/board-preview.jpg`]
            },
            unit_amount: Math.round(parseFloat(total_price) * 100 / quantity) // Convert to pence
          },
          quantity: quantity
        }
      ],
      mode: 'payment',
      success_url: `${WEBSITE_URL}/order-success.html?session_id={CHECKOUT_SESSION_ID}&order=${encodeURIComponent(order_number)}`,
      cancel_url: `${WEBSITE_URL}/index.html?order_cancelled=${encodeURIComponent(order_number)}#order`,
      customer_email: customer_email,
      client_reference_id: order_id,
      metadata: {
        order_id: order_id,
        order_number: order_number
      }
    });

    return res.status(200).json({
      success: true,
      session_id: session.id,
      checkout_url: session.url
    });

  } catch (error) {
    console.error('Stripe checkout error:', error);
    return res.status(500).json({
      success: false,
      error: 'Failed to create checkout session',
      message: error.message
    });
  }
}
