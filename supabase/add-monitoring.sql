-- Add monitoring columns to boards table
-- Run this in Supabase SQL Editor after running schema.sql

-- Add last_seen column for tracking board health
ALTER TABLE public.boards ADD COLUMN IF NOT EXISTS last_seen TIMESTAMPTZ;

-- Add IP address tracking for diagnostics
ALTER TABLE public.boards ADD COLUMN IF NOT EXISTS last_ip TEXT;

-- Create index for monitoring queries
CREATE INDEX IF NOT EXISTS idx_boards_last_seen ON public.boards(last_seen DESC);

-- Add RLS policy for anon users to update heartbeat (boards calling in)
CREATE POLICY "Boards can update their own heartbeat"
  ON public.boards FOR UPDATE
  TO anon
  USING (true)
  WITH CHECK (true);

-- Create a function to handle board heartbeat
CREATE OR REPLACE FUNCTION public.board_heartbeat(
  p_board_id TEXT,
  p_ip TEXT DEFAULT NULL
)
RETURNS JSON AS $$
DECLARE
  v_board public.boards;
  v_order_id UUID;
  v_is_first_activation BOOLEAN := FALSE;
BEGIN
  -- Get current board state
  SELECT * INTO v_board FROM public.boards WHERE board_id = p_board_id;

  IF v_board IS NULL THEN
    RETURN json_build_object(
      'success', false,
      'error', 'Board not found',
      'board_id', p_board_id
    );
  END IF;

  -- Check if this is first activation
  IF v_board.activated_at IS NULL THEN
    v_is_first_activation := TRUE;
  END IF;

  -- Update board status
  UPDATE public.boards
  SET
    last_seen = NOW(),
    last_ip = COALESCE(p_ip, last_ip),
    activated_at = COALESCE(activated_at, NOW()),
    status = CASE
      WHEN status = 'shipped' THEN 'active'
      WHEN status = 'in_stock' THEN 'active'
      WHEN status = 'assigned' THEN 'active'
      ELSE status
    END
  WHERE board_id = p_board_id
  RETURNING order_id INTO v_order_id;

  -- Log first activation in order history
  IF v_is_first_activation AND v_order_id IS NOT NULL THEN
    INSERT INTO public.order_history (
      order_id,
      action,
      description,
      performed_by
    ) VALUES (
      v_order_id,
      'board_activated',
      'Board ' || p_board_id || ' came online for the first time',
      'system'
    );
  END IF;

  RETURN json_build_object(
    'success', true,
    'board_id', p_board_id,
    'status', v_board.status,
    'first_activation', v_is_first_activation,
    'timestamp', NOW()
  );
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- Grant execute permission to anon role
GRANT EXECUTE ON FUNCTION public.board_heartbeat(TEXT, TEXT) TO anon;

COMMENT ON FUNCTION public.board_heartbeat IS 'Called by ESP32 boards to report their online status';
