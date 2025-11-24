/**
 * Create Royal Mail Manifest
 *
 * Creates a manifest for all unmanifested shipments created today
 *
 * Endpoint: /api/create-manifest
 * Method: POST
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

  // Check if Royal Mail is configured
  if (!ROYAL_MAIL_CLIENT_ID || !ROYAL_MAIL_CLIENT_SECRET) {
    return res.status(400).json({
      success: false,
      error: 'Royal Mail API not configured'
    });
  }

  try {
    const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

    // Get today's date
    const today = new Date();
    today.setHours(0, 0, 0, 0);

    // Find all shipments created today without a manifest
    const { data: shipments, error: shipmentsError } = await supabase
      .from('shipments')
      .select('*')
      .eq('status', 'label_created')
      .is('manifest_id', null)
      .gte('created_at', today.toISOString());

    if (shipmentsError) {
      throw new Error('Failed to fetch shipments');
    }

    if (!shipments || shipments.length === 0) {
      return res.status(200).json({
        success: false,
        error: 'No unmanifested shipments found for today'
      });
    }

    // Get access token
    const accessToken = await getAccessToken();

    // Create manifest
    const manifestResponse = await fetch(`${ROYAL_MAIL_API_URL}/manifests`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${accessToken}`,
        'Content-Type': 'application/json',
        'Accept': 'application/json'
      },
      body: JSON.stringify({
        trackingNumbers: shipments.map(s => s.tracking_number).filter(Boolean),
        manifestDate: new Date().toISOString().split('T')[0]
      })
    });

    if (!manifestResponse.ok) {
      const errorData = await manifestResponse.json();
      throw new Error(errorData.message || 'Failed to create manifest');
    }

    const manifestData = await manifestResponse.json();

    // Update shipments with manifest ID
    const manifestId = manifestData.manifestId;
    const manifestUrl = manifestData.manifestUrl;

    await supabase
      .from('shipments')
      .update({
        manifest_id: manifestId,
        manifest_url: manifestUrl,
        status: 'manifested'
      })
      .in('id', shipments.map(s => s.id));

    return res.status(200).json({
      success: true,
      manifest_id: manifestId,
      manifest_url: manifestUrl,
      shipment_count: shipments.length
    });

  } catch (error) {
    console.error('Create manifest error:', error);
    return res.status(500).json({
      success: false,
      error: error.message
    });
  }
}
