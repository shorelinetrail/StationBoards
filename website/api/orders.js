import { Resend } from 'resend';
import fs from 'fs/promises';
import path from 'path';

const resend = new Resend(process.env.RESEND_API_KEY);

// Simple file-based storage (migrate to database later)
const ORDERS_FILE = '/tmp/orders.json';

async function loadOrders() {
  try {
    const data = await fs.readFile(ORDERS_FILE, 'utf-8');
    return JSON.parse(data);
  } catch {
    return [];
  }
}

async function saveOrders(orders) {
  await fs.writeFile(ORDERS_FILE, JSON.stringify(orders, null, 2));
}

export default async function handler(req, res) {
  // Enable CORS
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') {
    return res.status(200).end();
  }

  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  try {
    const orderData = req.body;

    // Generate order ID
    const orderId = `SB${Date.now().toString(36).toUpperCase()}`;
    const timestamp = new Date().toISOString();

    // Create order object
    const order = {
      id: orderId,
      ...orderData,
      status: 'pending',
      tracking: null,
      createdAt: timestamp,
      updatedAt: timestamp
    };

    // Load existing orders
    const orders = await loadOrders();
    orders.push(order);
    await saveOrders(orders);

    // Send confirmation email to customer
    try {
      await resend.emails.send({
        from: 'StationBoards <orders@stationboards.co.uk>',
        to: orderData.email,
        subject: `Order Confirmation - ${orderId}`,
        html: `
          <div style="font-family: Arial, sans-serif; max-width: 600px; margin: 0 auto;">
            <h1 style="color: #667eea;">Thank You for Your Order!</h1>

            <p>Hi ${orderData.name},</p>

            <p>We've received your order and will send payment instructions within 24 hours.</p>

            <div style="background: #f7fafc; padding: 20px; border-radius: 8px; margin: 20px 0;">
              <h2 style="margin-top: 0;">Order Details</h2>
              <p><strong>Order ID:</strong> ${orderId}</p>
              <p><strong>Quantity:</strong> ${orderData.quantity} board(s)</p>
              <p><strong>Total:</strong> ${orderData.total}</p>
            </div>

            <div style="background: #f7fafc; padding: 20px; border-radius: 8px; margin: 20px 0;">
              <h2 style="margin-top: 0;">Shipping Address</h2>
              <p>${orderData.address}<br>
              ${orderData.city}, ${orderData.postcode}</p>
            </div>

            <p>We'll email you again with:</p>
            <ul>
              <li>Payment instructions</li>
              <li>Shipping confirmation with tracking number</li>
              <li>Delivery updates</li>
            </ul>

            <p>If you have any questions, reply to this email or contact us at support@stationboards.co.uk</p>

            <p>Thank you for your support!</p>
            <p><strong>The StationBoards Team</strong></p>

            <hr style="margin: 30px 0; border: none; border-top: 1px solid #e2e8f0;">
            <p style="color: #718096; font-size: 12px;">StationBoards.co.uk - Live train boards for your home</p>
          </div>
        `
      });
    } catch (emailError) {
      console.error('Email error:', emailError);
      // Don't fail the order if email fails
    }

    // Send notification email to admin
    try {
      await resend.emails.send({
        from: 'StationBoards <orders@stationboards.co.uk>',
        to: 'orders@stationboards.co.uk',
        subject: `New Order: ${orderId}`,
        html: `
          <div style="font-family: Arial, sans-serif;">
            <h1>New Order Received</h1>

            <p><strong>Order ID:</strong> ${orderId}</p>
            <p><strong>Customer:</strong> ${orderData.name}</p>
            <p><strong>Email:</strong> ${orderData.email}</p>
            <p><strong>Phone:</strong> ${orderData.phone || 'Not provided'}</p>
            <p><strong>Quantity:</strong> ${orderData.quantity}</p>
            <p><strong>Total:</strong> ${orderData.total}</p>

            <h3>Shipping Address:</h3>
            <p>${orderData.address}<br>
            ${orderData.city}, ${orderData.postcode}</p>

            ${orderData.notes ? `<h3>Notes:</h3><p>${orderData.notes}</p>` : ''}

            <p><a href="https://stationboards.co.uk/admin">View in Admin Dashboard</a></p>
          </div>
        `
      });
    } catch (emailError) {
      console.error('Admin email error:', emailError);
    }

    res.status(200).json({
      success: true,
      orderId: orderId,
      message: 'Order received successfully'
    });

  } catch (error) {
    console.error('Order error:', error);
    res.status(500).json({
      error: 'Failed to process order',
      message: error.message
    });
  }
}
