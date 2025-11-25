/**
 * Refresh Tracking Status
 *
 * Fetches latest tracking info from Royal Mail for a shipment
 *
 * Endpoint: /api/refresh-tracking
 * Method: POST
 * Body: { shipment_id: "uuid" }
 */

import { createClient } from '@supabase/supabase-js';

const ROYAL_MAIL_CLIENT_ID = process.env.ROYAL_MAIL_CLIENT_ID;
const ROYAL_MAIL_CLIENT_SECRET = process.env.ROYAL_MAIL_CLIENT_SECRET;
const ROYAL_MAIL_API_URL = process.env.ROYAL_MAIL_MODE === 'live'
  ? 'https://api.royalmail.net/shipping/v3'
  : 'https://api.sandbox.royalmail.net/shipping/v3';

const SUPABASE_URL = process.env.NEXT_PUBLIC_SUPABASE_URL;
const SUPABASE_SERVICE_KEY = process.env.SUPABASE_SERVICE_ROLE_KEY;

// Get Royal Mail access token
async function getAccessToken() {
  const response = await fetch(`${ROYAL_MAIL_API_URL}/token`, {
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

  if (!response.ok) {
    throw new Error('Failed to authenticate with Royal Mail');
  }

  const data = await response.json();
  return data.access_token;
}

export default async function handler(req, res) {
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  const { shipment_id } = req.body;

  if (!shipment_id) {
    return res.status(400).json({
      success: false,
      error: 'Missing shipment_id'
    });
  }

  // Check if Royal Mail is configured
  if (!ROYAL_MAIL_CLIENT_ID || !ROYAL_MAIL_CLIENT_SECRET) {
    return res.status(400).json({
      success: false,
      error: 'Royal Mail API not configured'
    });
  }

  try {
    const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

    // Get shipment details
    const { data: shipment, error: shipmentError } = await supabase
      .from('shipments')
      .select('*')
      .eq('id', shipment_id)
      .single();

    if (shipmentError || !shipment) {
      throw new Error('Shipment not found');
    }

    if (!shipment.tracking_number) {
      throw new Error('No tracking number for this shipment');
    }

    // Get access token
    const accessToken = await getAccessToken();

    // Fetch tracking info from Royal Mail
    const trackingResponse = await fetch(
      `${ROYAL_MAIL_API_URL}/tracking/${shipment.tracking_number}`,
      {
        headers: {
          'Authorization': `Bearer ${accessToken}`,
          'Accept': 'application/json'
        }
      }
    );

    if (!trackingResponse.ok) {
      throw new Error('Failed to fetch tracking information');
    }

    const trackingData = await trackingResponse.json();

    // Parse Royal Mail tracking status
    const status = trackingData.status || shipment.status;
    const statusDetails = trackingData.statusDescription || null;
    const deliveredAt = trackingData.deliveredAt || null;

    // Update shipment in database
    await supabase
      .from('shipments')
      .update({
        status: status,
        status_details: statusDetails,
        delivered_at: deliveredAt,
        updated_at: new Date().toISOString()
      })
      .eq('id', shipment_id);

    return res.status(200).json({
      success: true,
      status: status,
      status_details: statusDetails,
      delivered_at: deliveredAt
    });

  } catch (error) {
    console.error('Refresh tracking error:', error);
    return res.status(500).json({
      success: false,
      error: error.message
    });
  }
}
