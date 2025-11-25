// Email service using Resend
const { Resend } = require('resend');
const { logEmail } = require('./supabase');

const resend = new Resend(process.env.RESEND_API_KEY);
const FROM_EMAIL = process.env.FROM_EMAIL || 'orders@stationboards.co.uk';
const WEBSITE_URL = process.env.WEBSITE_URL || 'https://stationboards.co.uk';

/**
 * Send order confirmation email to customer
 */
async function sendOrderConfirmationEmail({
  orderId,
  orderNumber,
  customerName,
  customerEmail,
  quantity,
  totalPrice,
  shippingAddress
}) {
  const subject = `Order Confirmation - ${orderNumber}`;

  const html = `
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Order Confirmation</title>
</head>
<body style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; line-height: 1.6; color: #333; max-width: 600px; margin: 0 auto; padding: 20px;">
  <div style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 30px; border-radius: 10px 10px 0 0; text-align: center;">
    <h1 style="margin: 0; font-size: 28px;">🚉 StationBoards</h1>
    <p style="margin: 10px 0 0 0; font-size: 16px; opacity: 0.9;">Order Confirmation</p>
  </div>

  <div style="background: #f9f9f9; padding: 30px; border-radius: 0 0 10px 10px;">
    <p style="font-size: 18px; margin-top: 0;">Hi ${customerName},</p>

    <p>Thank you for your order! We've received your request and will send payment instructions to this email within 24 hours.</p>

    <div style="background: white; padding: 20px; border-radius: 8px; margin: 20px 0;">
      <h2 style="margin-top: 0; color: #667eea; font-size: 20px;">Order Details</h2>

      <table style="width: 100%; border-collapse: collapse;">
        <tr>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee;"><strong>Order Number:</strong></td>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee; text-align: right;">${orderNumber}</td>
        </tr>
        <tr>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee;"><strong>Quantity:</strong></td>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee; text-align: right;">${quantity} board${quantity > 1 ? 's' : ''}</td>
        </tr>
        <tr>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee;"><strong>Total:</strong></td>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee; text-align: right; font-size: 18px; color: #667eea;"><strong>£${totalPrice.toFixed(2)}</strong></td>
        </tr>
      </table>
    </div>

    <div style="background: white; padding: 20px; border-radius: 8px; margin: 20px 0;">
      <h3 style="margin-top: 0; color: #667eea; font-size: 18px;">Shipping Address</h3>
      <p style="margin: 0; white-space: pre-line;">${shippingAddress}</p>
    </div>

    <div style="background: #e8f4ff; padding: 20px; border-radius: 8px; border-left: 4px solid #667eea; margin: 20px 0;">
      <h3 style="margin-top: 0; font-size: 16px;">📋 What happens next?</h3>
      <ol style="margin: 10px 0; padding-left: 20px;">
        <li>We'll send payment instructions within 24 hours</li>
        <li>Once payment is received, we'll prepare your order</li>
        <li>You'll receive tracking information when your order ships</li>
        <li>Typical delivery: 3-5 business days</li>
      </ol>
    </div>

    <div style="text-align: center; margin: 30px 0;">
      <p style="margin-bottom: 15px; color: #666;">Track your order anytime:</p>
      <a href="${WEBSITE_URL}/track.html?order=${encodeURIComponent(orderNumber)}&email=${encodeURIComponent(customerEmail)}" style="display: inline-block; background: #667eea; color: white; padding: 14px 35px; text-decoration: none; border-radius: 6px; font-weight: 600; margin-bottom: 10px;">Track Your Order</a>
      <p style="margin-top: 20px; margin-bottom: 15px;">Questions about your order?</p>
      <a href="mailto:${FROM_EMAIL}" style="display: inline-block; background: white; color: #667eea; border: 2px solid #667eea; padding: 12px 30px; text-decoration: none; border-radius: 6px; font-weight: 600;">Contact Support</a>
    </div>

    <p style="color: #666; font-size: 14px; text-align: center; margin-top: 30px;">
      StationBoards - Live departure boards for your home<br>
      <a href="${WEBSITE_URL}" style="color: #667eea; text-decoration: none;">${WEBSITE_URL}</a>
    </p>
  </div>
</body>
</html>
  `;

  try {
    const { data, error } = await resend.emails.send({
      from: `StationBoards <${FROM_EMAIL}>`,
      to: customerEmail,
      subject,
      html
    });

    if (error) throw error;

    // Log successful email
    await logEmail(orderId, 'order_confirmation', customerEmail, subject, data?.id, 'sent');

    return data;
  } catch (error) {
    // Log failed email
    await logEmail(orderId, 'order_confirmation', customerEmail, subject, null, 'failed', error.message);
    throw error;
  }
}

/**
 * Send shipping notification email with tracking
 */
async function sendShippingNotificationEmail({
  orderId,
  orderNumber,
  customerName,
  customerEmail,
  trackingNumber,
  serviceName,
  boardIds = []
}) {
  const subject = `Your order ${orderNumber} has shipped! 📦`;

  const trackingUrl = trackingNumber
    ? `https://www.royalmail.com/track-your-item#/tracking-results/${trackingNumber}`
    : null;

  const html = `
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Order Shipped</title>
</head>
<body style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; line-height: 1.6; color: #333; max-width: 600px; margin: 0 auto; padding: 20px;">
  <div style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 30px; border-radius: 10px 10px 0 0; text-align: center;">
    <h1 style="margin: 0; font-size: 28px;">📦 Order Shipped!</h1>
  </div>

  <div style="background: #f9f9f9; padding: 30px; border-radius: 0 0 10px 10px;">
    <p style="font-size: 18px; margin-top: 0;">Hi ${customerName},</p>

    <p>Great news! Your StationBoard order has been shipped and is on its way to you.</p>

    <div style="background: white; padding: 20px; border-radius: 8px; margin: 20px 0;">
      <h2 style="margin-top: 0; color: #667eea; font-size: 20px;">Shipping Details</h2>

      <table style="width: 100%; border-collapse: collapse;">
        <tr>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee;"><strong>Order Number:</strong></td>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee; text-align: right;">${orderNumber}</td>
        </tr>
        ${trackingNumber ? `
        <tr>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee;"><strong>Tracking Number:</strong></td>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee; text-align: right; font-family: monospace;">${trackingNumber}</td>
        </tr>
        ` : ''}
        ${serviceName ? `
        <tr>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee;"><strong>Service:</strong></td>
          <td style="padding: 8px 0; border-bottom: 1px solid #eee; text-align: right;">${serviceName}</td>
        </tr>
        ` : ''}
      </table>

      ${trackingUrl ? `
      <div style="text-align: center; margin-top: 20px;">
        <a href="${trackingUrl}" style="display: inline-block; background: #667eea; color: white; padding: 12px 30px; text-decoration: none; border-radius: 6px; font-weight: 600;">Track Your Package</a>
      </div>
      ` : ''}
    </div>

    ${boardIds.length > 0 ? `
    <div style="background: white; padding: 20px; border-radius: 8px; margin: 20px 0;">
      <h3 style="margin-top: 0; color: #667eea; font-size: 18px;">Board IDs</h3>
      <p style="color: #666; font-size: 14px; margin-bottom: 10px;">Your order includes the following board${boardIds.length > 1 ? 's' : ''}:</p>
      <ul style="list-style: none; padding: 0; margin: 0;">
        ${boardIds.map(id => `<li style="padding: 8px; background: #f9f9f9; margin-bottom: 5px; border-radius: 4px; font-family: monospace;">${id}</li>`).join('')}
      </ul>
    </div>
    ` : ''}

    <div style="background: #e8f4ff; padding: 20px; border-radius: 8px; border-left: 4px solid #667eea; margin: 20px 0;">
      <h3 style="margin-top: 0; font-size: 16px;">🏠 Setup Guide</h3>
      <p>Once your board arrives, setup is easy:</p>
      <ol style="margin: 10px 0; padding-left: 20px;">
        <li>Plug in your board using the included USB-C cable</li>
        <li>Connect to the "TrainBoard_AP" WiFi network</li>
        <li>Follow the web interface to configure your WiFi</li>
        <li>Choose your station and customize settings</li>
      </ol>
      <p style="margin: 15px 0 0 0;">
        <a href="${WEBSITE_URL}/setup" style="color: #667eea; text-decoration: none; font-weight: 600;">View Full Setup Guide →</a>
      </p>
    </div>

    <div style="text-align: center; margin: 30px 0;">
      <p style="margin-bottom: 15px; color: #666;">Check your order status anytime:</p>
      <a href="${WEBSITE_URL}/track.html?order=${encodeURIComponent(orderNumber)}&email=${encodeURIComponent(customerEmail)}" style="display: inline-block; background: #667eea; color: white; padding: 14px 35px; text-decoration: none; border-radius: 6px; font-weight: 600; margin-bottom: 10px;">View Order Status</a>
      <p style="margin-top: 20px; margin-bottom: 15px;">Need help?</p>
      <a href="mailto:support@stationboards.co.uk" style="display: inline-block; background: white; color: #667eea; border: 2px solid #667eea; padding: 12px 30px; text-decoration: none; border-radius: 6px; font-weight: 600;">Contact Support</a>
    </div>

    <p style="color: #666; font-size: 14px; text-align: center; margin-top: 30px;">
      StationBoards - Live departure boards for your home<br>
      <a href="${WEBSITE_URL}" style="color: #667eea; text-decoration: none;">${WEBSITE_URL}</a>
    </p>
  </div>
</body>
</html>
  `;

  try {
    const { data, error } = await resend.emails.send({
      from: `StationBoards <${FROM_EMAIL}>`,
      to: customerEmail,
      subject,
      html
    });

    if (error) throw error;

    await logEmail(orderId, 'shipping_notification', customerEmail, subject, data?.id, 'sent');
    return data;
  } catch (error) {
    await logEmail(orderId, 'shipping_notification', customerEmail, subject, null, 'failed', error.message);
    throw error;
  }
}

/**
 * Send payment reminder email
 */
async function sendPaymentReminderEmail({
  orderId,
  orderNumber,
  customerName,
  customerEmail,
  totalPrice
}) {
  const subject = `Payment Pending - Order ${orderNumber}`;

  const html = `
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Payment Reminder</title>
</head>
<body style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; line-height: 1.6; color: #333; max-width: 600px; margin: 0 auto; padding: 20px;">
  <div style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 30px; border-radius: 10px 10px 0 0; text-align: center;">
    <h1 style="margin: 0; font-size: 28px;">💳 Payment Reminder</h1>
  </div>

  <div style="background: #f9f9f9; padding: 30px; border-radius: 0 0 10px 10px;">
    <p style="font-size: 18px; margin-top: 0;">Hi ${customerName},</p>

    <p>This is a friendly reminder that payment is pending for your StationBoard order.</p>

    <div style="background: white; padding: 20px; border-radius: 8px; margin: 20px 0; text-align: center;">
      <p style="margin: 0; color: #666;">Order Number</p>
      <p style="font-size: 24px; font-weight: bold; color: #667eea; margin: 10px 0;">${orderNumber}</p>
      <p style="font-size: 32px; font-weight: bold; margin: 20px 0;">£${totalPrice.toFixed(2)}</p>
    </div>

    <div style="background: #fff3cd; padding: 20px; border-radius: 8px; border-left: 4px solid #ffc107; margin: 20px 0;">
      <p style="margin: 0;"><strong>Please contact us to arrange payment:</strong></p>
      <p style="margin: 10px 0 0 0;">
        Email: <a href="mailto:${FROM_EMAIL}" style="color: #667eea;">${FROM_EMAIL}</a>
      </p>
    </div>

    <p style="color: #666; font-size: 14px; text-align: center; margin-top: 30px;">
      StationBoards - Live departure boards for your home<br>
      <a href="${WEBSITE_URL}" style="color: #667eea; text-decoration: none;">${WEBSITE_URL}</a>
    </p>
  </div>
</body>
</html>
  `;

  try {
    const { data, error } = await resend.emails.send({
      from: `StationBoards <${FROM_EMAIL}>`,
      to: customerEmail,
      subject,
      html
    });

    if (error) throw error;

    await logEmail(orderId, 'payment_reminder', customerEmail, subject, data?.id, 'sent');
    return data;
  } catch (error) {
    await logEmail(orderId, 'payment_reminder', customerEmail, subject, null, 'failed', error.message);
    throw error;
  }
}

module.exports = {
  sendOrderConfirmationEmail,
  sendShippingNotificationEmail,
  sendPaymentReminderEmail
};
