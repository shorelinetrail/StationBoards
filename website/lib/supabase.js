// Supabase client configuration
import { createClient } from '@supabase/supabase-js';

// Client-side Supabase client (uses anon key, respects RLS)
export const supabase = createClient(
  process.env.NEXT_PUBLIC_SUPABASE_URL,
  process.env.NEXT_PUBLIC_SUPABASE_ANON_KEY
);

// Server-side Supabase client (uses service role key, bypasses RLS)
// Only use this in API routes, never expose to client
export const supabaseAdmin = createClient(
  process.env.NEXT_PUBLIC_SUPABASE_URL,
  process.env.SUPABASE_SERVICE_ROLE_KEY,
  {
    auth: {
      autoRefreshToken: false,
      persistSession: false
    }
  }
);

// Helper: Log order activity
export async function logOrderActivity(orderId, action, description, metadata = null, performedBy = 'system') {
  try {
    const { error } = await supabaseAdmin
      .from('order_history')
      .insert({
        order_id: orderId,
        action,
        description,
        metadata,
        performed_by: performedBy
      });

    if (error) throw error;
  } catch (error) {
    console.error('Failed to log order activity:', error);
  }
}

// Helper: Log sent email
export async function logEmail(orderId, emailType, recipientEmail, subject, emailId = null, status = 'sent', errorMessage = null) {
  try {
    const { error } = await supabaseAdmin
      .from('email_log')
      .insert({
        order_id: orderId,
        email_type: emailType,
        recipient_email: recipientEmail,
        subject,
        email_id: emailId,
        status,
        error_message: errorMessage
      });

    if (error) throw error;
  } catch (error) {
    console.error('Failed to log email:', error);
  }
}

// Helper: Get order with related data
export async function getOrderWithDetails(orderId) {
  const { data: order, error: orderError } = await supabaseAdmin
    .from('orders')
    .select(`
      *,
      boards (*),
      shipments (*),
      order_history (
        *,
        user:user_id (email)
      ),
      email_log (*)
    `)
    .eq('id', orderId)
    .single();

  if (orderError) throw orderError;
  return order;
}

// Helper: Get available boards (in stock)
export async function getAvailableBoards() {
  const { data, error } = await supabaseAdmin
    .from('boards')
    .select('*')
    .eq('status', 'in_stock')
    .order('created_at', { ascending: true });

  if (error) throw error;
  return data;
}

// Helper: Assign board to order
export async function assignBoardToOrder(boardId, orderId, performedBy = 'system') {
  const { data, error } = await supabaseAdmin
    .from('boards')
    .update({
      order_id: orderId,
      status: 'assigned',
      assigned_at: new Date().toISOString()
    })
    .eq('board_id', boardId)
    .select()
    .single();

  if (error) throw error;

  // Log activity
  await logOrderActivity(
    orderId,
    'board_assigned',
    `Board ${boardId} assigned to order`,
    { board_id: boardId },
    performedBy
  );

  return data;
}
