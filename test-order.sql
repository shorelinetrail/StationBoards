-- Quick test: Add a sample order to verify the system works
-- Run this in Supabase SQL Editor after running schema.sql

-- Insert a test order
INSERT INTO public.orders (
  order_number,
  customer_name,
  customer_email,
  customer_phone,
  shipping_address,
  shipping_city,
  shipping_postcode,
  quantity,
  total_price,
  status,
  payment_status
) VALUES (
  'SB-TEST-0001',
  'Test Customer',
  'test@example.com',
  '07700900000',
  '123 Test Street',
  'London',
  'SW1A 1AA',
  1,
  89.00,
  'pending',
  'pending'
);

-- Add a test board
INSERT INTO public.boards (
  board_id,
  status,
  firmware_version,
  hardware_revision
) VALUES (
  'ESP32-TEST123',
  'in_stock',
  '1.0.0',
  'v1.0'
);

-- Verify
SELECT 'Orders:' as table_name, COUNT(*) as count FROM public.orders
UNION ALL
SELECT 'Boards:' as table_name, COUNT(*) as count FROM public.boards;
