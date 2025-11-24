// Vercel Serverless Function - Submit Order
// POST /api/submit-order

const { supabaseAdmin, logOrderActivity } = require('../lib/supabase');
const { sendOrderConfirmationEmail } = require('../lib/email');

module.exports = async (req, res) => {
  // CORS headers
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
    const {
      name,
      email,
      phone,
      address,
      city,
      postcode,
      quantity,
      notes
    } = req.body;

    // Validation
    if (!name || !email || !address || !city || !postcode || !quantity) {
      return res.status(400).json({
        error: 'Missing required fields',
        required: ['name', 'email', 'address', 'city', 'postcode', 'quantity']
      });
    }

    // Email validation
    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    if (!emailRegex.test(email)) {
      return res.status(400).json({ error: 'Invalid email address' });
    }

    // Quantity validation
    const qty = parseInt(quantity);
    if (isNaN(qty) || qty < 1 || qty > 100) {
      return res.status(400).json({ error: 'Invalid quantity (must be 1-100)' });
    }

    // Calculate total price
    const pricing = {
      1: 89.00,
      2: 168.00,
      3: 240.00,
      5: 385.00
    };

    const totalPrice = pricing[qty] || (qty * 89.00);

    // Generate order number using database function
    const { data: orderNumberData, error: orderNumberError } = await supabaseAdmin
      .rpc('generate_order_number');

    if (orderNumberError) throw orderNumberError;
    const orderNumber = orderNumberData;

    // Insert order
    const { data: order, error: orderError } = await supabaseAdmin
      .from('orders')
      .insert({
        order_number: orderNumber,
        customer_name: name,
        customer_email: email,
        customer_phone: phone || null,
        shipping_address: address,
        shipping_city: city,
        shipping_postcode: postcode,
        quantity: qty,
        total_price: totalPrice,
        notes: notes || null,
        status: 'pending',
        payment_status: 'pending'
      })
      .select()
      .single();

    if (orderError) throw orderError;

    // Log order creation
    await logOrderActivity(
      order.id,
      'order_created',
      `Order created by ${name}`,
      {
        quantity: qty,
        total_price: totalPrice,
        email
      }
    );

    // Send confirmation email
    try {
      await sendOrderConfirmationEmail({
        orderId: order.id,
        orderNumber: order.order_number,
        customerName: name,
        customerEmail: email,
        quantity: qty,
        totalPrice,
        shippingAddress: `${address}, ${city}, ${postcode}`
      });
    } catch (emailError) {
      console.error('Failed to send confirmation email:', emailError);
      // Don't fail the order if email fails
      await logOrderActivity(
        order.id,
        'email_failed',
        'Failed to send confirmation email',
        { error: emailError.message }
      );
    }

    // Return success
    return res.status(201).json({
      success: true,
      order: {
        id: order.id,
        orderNumber: order.order_number,
        totalPrice,
        createdAt: order.created_at
      },
      message: 'Order submitted successfully! You will receive a confirmation email with payment instructions shortly.'
    });

  } catch (error) {
    console.error('Order submission error:', error);
    return res.status(500).json({
      error: 'Failed to submit order',
      message: error.message
    });
  }
};
