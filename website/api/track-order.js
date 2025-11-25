/**
 * Track Order API
 *
 * Public endpoint for customers to track their orders
 * No authentication required - validates using email + order number
 *
 * Endpoint: /api/track-order
 * Method: POST
 * Body: { email: "customer@example.com", order_number: "SB-20250124-A7F2" }
 *
 * Response:
 * {
 *   success: true,
 *   order: {
 *     order_number, status, payment_status, created_at, shipped_at,
 *     quantity, total_price, shipping_address, shipping_city, shipping_postcode,
 *     boards: [...], shipment: {...}, order_history: [...]
 *   }
 * }
 */

import { createClient } from '@supabase/supabase-js';

const SUPABASE_URL = process.env.NEXT_PUBLIC_SUPABASE_URL;
const SUPABASE_SERVICE_KEY = process.env.SUPABASE_SERVICE_ROLE_KEY;

// Simple in-memory rate limiting (for basic protection)
const rateLimitMap = new Map();
const RATE_LIMIT_WINDOW = 60 * 1000; // 1 minute
const MAX_REQUESTS = 10; // 10 requests per minute per IP

export default async function handler(req, res) {
  // Only allow POST
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  // Get client IP for rate limiting
  const clientIp = req.headers['x-forwarded-for'] ||
                   req.headers['x-real-ip'] ||
                   req.connection?.remoteAddress ||
                   'unknown';

  // Rate limiting
  const now = Date.now();
  const clientData = rateLimitMap.get(clientIp) || { count: 0, resetAt: now + RATE_LIMIT_WINDOW };

  if (now < clientData.resetAt) {
    if (clientData.count >= MAX_REQUESTS) {
      return res.status(429).json({
        success: false,
        error: 'Too many requests. Please try again in a minute.'
      });
    }
    clientData.count++;
  } else {
    clientData.count = 1;
    clientData.resetAt = now + RATE_LIMIT_WINDOW;
  }

  rateLimitMap.set(clientIp, clientData);

  // Clean up old entries every 100 requests
  if (Math.random() < 0.01) {
    for (const [ip, data] of rateLimitMap.entries()) {
      if (now > data.resetAt) {
        rateLimitMap.delete(ip);
      }
    }
  }

  const { email, order_number } = req.body;

  // Validate input
  if (!email || !order_number) {
    return res.status(400).json({
      success: false,
      error: 'Email and order number are required'
    });
  }

  // Validate email format
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  if (!emailRegex.test(email)) {
    return res.status(400).json({
      success: false,
      error: 'Invalid email format'
    });
  }

  // Validate order number format
  const orderRegex = /^SB-\d{8}-[A-Z0-9]{4}$/;
  if (!orderRegex.test(order_number)) {
    return res.status(400).json({
      success: false,
      error: 'Invalid order number format'
    });
  }

  try {
    // Create Supabase client with service role key (to bypass RLS for read-only tracking)
    const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

    // Query order with related data
    const { data: order, error } = await supabase
      .from('orders')
      .select(`
        id,
        order_number,
        customer_name,
        customer_email,
        customer_phone,
        shipping_address,
        shipping_city,
        shipping_postcode,
        shipping_country,
        quantity,
        total_price,
        status,
        payment_status,
        created_at,
        paid_at,
        shipped_at,
        delivered_at,
        boards (
          board_id,
          status,
          assigned_at,
          activated_at,
          last_seen
        ),
        shipments (
          tracking_number,
          service_code,
          service_name,
          status,
          delivered_at
        ),
        order_history (
          action,
          description,
          created_at
        )
      `)
      .eq('order_number', order_number)
      .eq('customer_email', email.toLowerCase())
      .single();

    if (error) {
      if (error.code === 'PGRST116') {
        // No rows returned
        return res.status(404).json({
          success: false,
          error: 'Order not found. Please check your email and order number.'
        });
      }

      console.error('Database error:', error);
      return res.status(500).json({
        success: false,
        error: 'Failed to retrieve order'
      });
    }

    // Remove sensitive data before sending to client
    const safeOrder = {
      order_number: order.order_number,
      customer_name: order.customer_name,
      shipping_address: order.shipping_address,
      shipping_city: order.shipping_city,
      shipping_postcode: order.shipping_postcode,
      shipping_country: order.shipping_country,
      quantity: order.quantity,
      total_price: order.total_price,
      status: order.status,
      payment_status: order.payment_status,
      created_at: order.created_at,
      paid_at: order.paid_at,
      shipped_at: order.shipped_at,
      delivered_at: order.delivered_at,
      boards: order.boards || [],
      shipment: order.shipments || null,
      order_history: order.order_history || []
    };

    return res.status(200).json({
      success: true,
      order: safeOrder
    });

  } catch (error) {
    console.error('Track order error:', error);
    return res.status(500).json({
      success: false,
      error: 'Internal server error',
      message: error.message
    });
  }
}
