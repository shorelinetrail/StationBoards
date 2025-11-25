-- StationBoards Order Management - Supabase Schema
-- Run this in your Supabase SQL Editor

-- Enable UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Orders table
CREATE TABLE public.orders (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  order_number TEXT UNIQUE NOT NULL,

  -- Customer information
  customer_name TEXT NOT NULL,
  customer_email TEXT NOT NULL,
  customer_phone TEXT,

  -- Shipping address
  shipping_address TEXT NOT NULL,
  shipping_city TEXT NOT NULL,
  shipping_postcode TEXT NOT NULL,
  shipping_country TEXT DEFAULT 'United Kingdom',

  -- Order details
  quantity INTEGER NOT NULL CHECK (quantity > 0),
  total_price DECIMAL(10, 2) NOT NULL CHECK (total_price > 0),
  notes TEXT,

  -- Status tracking
  status TEXT DEFAULT 'pending' CHECK (status IN ('pending', 'paid', 'processing', 'shipped', 'delivered', 'cancelled')),
  payment_status TEXT DEFAULT 'pending' CHECK (payment_status IN ('pending', 'paid', 'refunded')),

  -- Timestamps
  created_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW(),
  paid_at TIMESTAMPTZ,
  shipped_at TIMESTAMPTZ,
  delivered_at TIMESTAMPTZ
);

-- Boards table - track individual boards
CREATE TABLE public.boards (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  board_id TEXT UNIQUE NOT NULL, -- Physical device ID (ESP32 chip ID)
  order_id UUID REFERENCES public.orders(id) ON DELETE SET NULL,

  -- Board status
  status TEXT DEFAULT 'in_stock' CHECK (status IN ('in_stock', 'assigned', 'shipped', 'active', 'faulty', 'returned')),

  -- Manufacturing info
  manufactured_date DATE,
  firmware_version TEXT,
  hardware_revision TEXT,

  -- Assignment tracking
  assigned_at TIMESTAMPTZ,
  shipped_at TIMESTAMPTZ,
  activated_at TIMESTAMPTZ, -- When board first connects to monitoring

  -- Notes
  notes TEXT,

  created_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Shipments table - Royal Mail tracking
CREATE TABLE public.shipments (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  order_id UUID REFERENCES public.orders(id) ON DELETE CASCADE UNIQUE,

  -- Royal Mail details
  tracking_number TEXT,
  service_code TEXT, -- e.g., 'CRL24', 'SD1', 'TPL'
  service_name TEXT, -- e.g., 'Royal Mail 24 Tracked'

  -- Shipping label
  label_url TEXT, -- Supabase Storage URL for label PDF
  label_created_at TIMESTAMPTZ,

  -- Package details
  weight_grams INTEGER,
  length_cm DECIMAL(5, 2),
  width_cm DECIMAL(5, 2),
  height_cm DECIMAL(5, 2),

  -- Royal Mail manifest
  manifest_id TEXT,
  manifest_url TEXT,

  -- Status
  status TEXT DEFAULT 'label_created' CHECK (status IN ('label_created', 'manifested', 'collected', 'in_transit', 'out_for_delivery', 'delivered', 'failed')),
  status_details TEXT,
  delivered_at TIMESTAMPTZ,

  -- Timestamps
  created_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Order history/activity log
CREATE TABLE public.order_history (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  order_id UUID REFERENCES public.orders(id) ON DELETE CASCADE,

  -- Activity details
  action TEXT NOT NULL,
  description TEXT,

  -- Metadata (JSONB for flexible data)
  metadata JSONB,

  -- User who performed action
  performed_by TEXT DEFAULT 'system',
  user_id UUID REFERENCES auth.users(id) ON DELETE SET NULL,

  created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Email log - track sent emails
CREATE TABLE public.email_log (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  order_id UUID REFERENCES public.orders(id) ON DELETE CASCADE,

  -- Email details
  email_type TEXT NOT NULL,
  recipient_email TEXT NOT NULL,
  subject TEXT,

  -- Provider details (Resend)
  email_id TEXT, -- Resend email ID

  -- Status
  status TEXT DEFAULT 'sent' CHECK (status IN ('sent', 'delivered', 'failed', 'bounced')),
  error_message TEXT,

  sent_at TIMESTAMPTZ DEFAULT NOW()
);

-- Create indexes for performance
CREATE INDEX idx_orders_order_number ON public.orders(order_number);
CREATE INDEX idx_orders_email ON public.orders(customer_email);
CREATE INDEX idx_orders_status ON public.orders(status);
CREATE INDEX idx_orders_created_at ON public.orders(created_at DESC);
CREATE INDEX idx_boards_board_id ON public.boards(board_id);
CREATE INDEX idx_boards_order_id ON public.boards(order_id);
CREATE INDEX idx_boards_status ON public.boards(status);
CREATE INDEX idx_shipments_order_id ON public.shipments(order_id);
CREATE INDEX idx_shipments_tracking_number ON public.shipments(tracking_number);
CREATE INDEX idx_order_history_order_id ON public.order_history(order_id, created_at DESC);
CREATE INDEX idx_email_log_order_id ON public.email_log(order_id);

-- Auto-update updated_at timestamp
CREATE OR REPLACE FUNCTION public.handle_updated_at()
RETURNS TRIGGER AS $$
BEGIN
  NEW.updated_at = NOW();
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER set_orders_updated_at
  BEFORE UPDATE ON public.orders
  FOR EACH ROW EXECUTE FUNCTION public.handle_updated_at();

CREATE TRIGGER set_boards_updated_at
  BEFORE UPDATE ON public.boards
  FOR EACH ROW EXECUTE FUNCTION public.handle_updated_at();

CREATE TRIGGER set_shipments_updated_at
  BEFORE UPDATE ON public.shipments
  FOR EACH ROW EXECUTE FUNCTION public.handle_updated_at();

-- Function to generate order number
CREATE OR REPLACE FUNCTION public.generate_order_number()
RETURNS TEXT AS $$
DECLARE
  new_number TEXT;
  done BOOLEAN := FALSE;
BEGIN
  WHILE NOT done LOOP
    -- Format: SB-YYYYMMDD-XXXX (e.g., SB-20250124-A7F2)
    new_number := 'SB-' ||
                  TO_CHAR(NOW(), 'YYYYMMDD') || '-' ||
                  UPPER(SUBSTRING(MD5(RANDOM()::TEXT) FROM 1 FOR 4));

    -- Check if number already exists
    IF NOT EXISTS (SELECT 1 FROM public.orders WHERE order_number = new_number) THEN
      done := TRUE;
    END IF;
  END LOOP;

  RETURN new_number;
END;
$$ LANGUAGE plpgsql;

-- Row Level Security (RLS) Policies
ALTER TABLE public.orders ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.boards ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.shipments ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.order_history ENABLE ROW LEVEL SECURITY;
ALTER TABLE public.email_log ENABLE ROW LEVEL SECURITY;

-- Public can insert orders (for order submission form)
CREATE POLICY "Anyone can create orders"
  ON public.orders FOR INSERT
  TO anon
  WITH CHECK (true);

-- Authenticated users (admins) can view and update everything
CREATE POLICY "Admins can view all orders"
  ON public.orders FOR SELECT
  TO authenticated
  USING (true);

CREATE POLICY "Admins can update orders"
  ON public.orders FOR UPDATE
  TO authenticated
  USING (true);

CREATE POLICY "Admins can delete orders"
  ON public.orders FOR DELETE
  TO authenticated
  USING (true);

-- Similar policies for other tables
CREATE POLICY "Admins can manage boards"
  ON public.boards FOR ALL
  TO authenticated
  USING (true);

CREATE POLICY "Admins can manage shipments"
  ON public.shipments FOR ALL
  TO authenticated
  USING (true);

CREATE POLICY "Admins can view order history"
  ON public.order_history FOR ALL
  TO authenticated
  USING (true);

CREATE POLICY "Admins can view email log"
  ON public.email_log FOR ALL
  TO authenticated
  USING (true);

-- Allow service role to do everything (for Edge Functions)
CREATE POLICY "Service role can do everything on orders"
  ON public.orders FOR ALL
  TO service_role
  USING (true);

CREATE POLICY "Service role can do everything on boards"
  ON public.boards FOR ALL
  TO service_role
  USING (true);

CREATE POLICY "Service role can do everything on shipments"
  ON public.shipments FOR ALL
  TO service_role
  USING (true);

CREATE POLICY "Service role can do everything on order_history"
  ON public.order_history FOR ALL
  TO service_role
  USING (true);

CREATE POLICY "Service role can do everything on email_log"
  ON public.email_log FOR ALL
  TO service_role
  USING (true);

-- Create storage bucket for shipping labels
INSERT INTO storage.buckets (id, name, public)
VALUES ('shipping-labels', 'shipping-labels', false)
ON CONFLICT (id) DO NOTHING;

-- Storage policy for shipping labels
CREATE POLICY "Admins can upload shipping labels"
  ON storage.objects FOR INSERT
  TO authenticated
  WITH CHECK (bucket_id = 'shipping-labels');

CREATE POLICY "Admins can view shipping labels"
  ON storage.objects FOR SELECT
  TO authenticated
  USING (bucket_id = 'shipping-labels');

CREATE POLICY "Service role can manage shipping labels"
  ON storage.objects FOR ALL
  TO service_role
  USING (bucket_id = 'shipping-labels');
