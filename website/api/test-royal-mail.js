/**
 * Test Royal Mail API Connection
 *
 * Tests if Royal Mail API credentials are configured and working
 *
 * Endpoint: /api/test-royal-mail
 * Method: POST
 */

const ROYAL_MAIL_CLIENT_ID = process.env.ROYAL_MAIL_CLIENT_ID;
const ROYAL_MAIL_CLIENT_SECRET = process.env.ROYAL_MAIL_CLIENT_SECRET;
const ROYAL_MAIL_API_URL = process.env.ROYAL_MAIL_MODE === 'live'
  ? 'https://api.royalmail.net/shipping/v3'
  : 'https://api.sandbox.royalmail.net/shipping/v3';

export default async function handler(req, res) {
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  // Check if credentials are configured
  if (!ROYAL_MAIL_CLIENT_ID || !ROYAL_MAIL_CLIENT_SECRET) {
    return res.status(200).json({
      success: false,
      error: 'Royal Mail API credentials not configured',
      configured: false
    });
  }

  try {
    // Try to authenticate
    const authResponse = await fetch(`${ROYAL_MAIL_API_URL}/token`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
      },
      body: new URLSearchParams({
        grant_type: 'client_credentials',
        client_id: ROYAL_MAIL_CLIENT_ID,
        client_secret: ROYAL_MAIL_CLIENT_SECRET
      })
    });

    if (!authResponse.ok) {
      const errorData = await authResponse.json();
      throw new Error(errorData.error_description || 'Authentication failed');
    }

    const data = await authResponse.json();

    return res.status(200).json({
      success: true,
      configured: true,
      mode: process.env.ROYAL_MAIL_MODE || 'sandbox',
      message: 'Royal Mail API connected successfully'
    });

  } catch (error) {
    console.error('Royal Mail test error:', error);
    return res.status(200).json({
      success: false,
      configured: true,
      error: error.message
    });
  }
}
