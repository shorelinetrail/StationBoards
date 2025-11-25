/**
 * Board Heartbeat API
 *
 * Called by ESP32 boards to report they are online
 * Updates last_seen timestamp and activates board if first connection
 *
 * Endpoint: /api/board-heartbeat
 * Method: POST
 * Body: { board_id: "ESP32-XXXXXXXX" }
 *
 * Response:
 * {
 *   success: true,
 *   board_id: "ESP32-XXXXXXXX",
 *   status: "active",
 *   first_activation: false,
 *   timestamp: "2025-01-24T12:34:56Z"
 * }
 */

import { createClient } from '@supabase/supabase-js';

const SUPABASE_URL = process.env.NEXT_PUBLIC_SUPABASE_URL;
const SUPABASE_SERVICE_KEY = process.env.SUPABASE_SERVICE_ROLE_KEY;

export default async function handler(req, res) {
  // Only allow POST
  if (req.method !== 'POST') {
    return res.status(405).json({ error: 'Method not allowed' });
  }

  const { board_id } = req.body;

  // Validate board_id
  if (!board_id || typeof board_id !== 'string') {
    return res.status(400).json({
      success: false,
      error: 'Missing or invalid board_id'
    });
  }

  try {
    // Create Supabase client with service role key (bypasses RLS)
    const supabase = createClient(SUPABASE_URL, SUPABASE_SERVICE_KEY);

    // Get client IP address for diagnostics
    const ip = req.headers['x-forwarded-for'] ||
               req.headers['x-real-ip'] ||
               req.connection?.remoteAddress ||
               'unknown';

    // Call the database function
    const { data, error } = await supabase.rpc('board_heartbeat', {
      p_board_id: board_id,
      p_ip: ip
    });

    if (error) {
      console.error('Heartbeat database error:', error);
      return res.status(500).json({
        success: false,
        error: 'Database error',
        details: error.message
      });
    }

    // Return the response from the database function
    return res.status(200).json(data);

  } catch (error) {
    console.error('Heartbeat error:', error);
    return res.status(500).json({
      success: false,
      error: 'Internal server error',
      message: error.message
    });
  }
}
