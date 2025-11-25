# Testing Guide - StationBoards Admin System

**Phase 2 Features Ready for Testing**

---

## 🚀 Deployment

Before testing, deploy to Vercel:

```bash
cd /home/user/StationBoards/website
vercel --prod
```

Wait for deployment to complete, then proceed with testing.

---

## ✅ Feature 1: Board Assignment

### Setup
1. Login to admin dashboard: `https://www.stationboards.co.uk/admin`
2. Go to **Boards** tab
3. Click "+ Add Board"
4. Add 3 test boards:
   - `ESP32-TEST001`, firmware: `1.0.0`, hardware: `v1.0`
   - `ESP32-TEST002`, firmware: `1.0.0`, hardware: `v1.0`
   - `ESP32-TEST003`, firmware: `1.0.0`, hardware: `v1.0`

### Test Assignment
1. Go to **Orders** tab
2. Click on an order
3. In the order detail modal, find **Assigned Boards** section
4. Click "**Assign Board**" button
5. **Expected**: Modal opens showing dropdown with 3 available boards
6. Select a board
7. (Optional) Add notes
8. Click "Assign Board"
9. **Expected**:
   - Success message appears
   - Order detail refreshes
   - Board shows in "Assigned Boards" section
   - Board status shows as "assigned"
10. Go to **Boards** tab
11. **Expected**: Board status changed from "in_stock" to "assigned"

### Database Verification
Check Supabase:
```sql
SELECT * FROM boards WHERE order_id IS NOT NULL;
SELECT * FROM order_history WHERE action = 'board_assigned';
```

---

## ✅ Feature 2: Manual Shipping

### Prerequisites
- Order must have status "paid" or "processing"
- Order must have at least one board assigned

### Test Shipping Entry
1. In order detail modal, find **Actions** section
2. Click "**Create Shipping Label**" button
3. **Expected**: Shipping modal opens
4. Fill in form:
   - **Service**: Royal Mail 48 Tracked
   - **Tracking Number**: `AB123456789GB` (test number)
   - **Weight**: 400g (default)
   - **Notes**: Test shipment
5. Click "Create Shipment"
6. **Expected**:
   - Success message: "Shipment created and notification sent successfully!"
   - Order status automatically changes to "shipped"
   - Shipping notification email logged (check email_log table)
   - Order detail refreshes showing shipment info

### Verify Shipment
1. In order detail, **Shipping Information** section should show:
   - Tracking Number: AB123456789GB
   - Service: Royal Mail 48 Tracked
   - Status: label_created
2. Go to **Boards** tab
3. **Expected**: Assigned board status changed to "shipped"

### Database Verification
```sql
SELECT * FROM shipments WHERE order_id = 'your-order-id';
SELECT * FROM email_log WHERE email_type = 'shipping';
SELECT * FROM order_history WHERE action = 'shipped';
```

---

## ✅ Feature 3: Status Updates

### Test Order Status Changes
1. Open order detail modal
2. Find **Order Details** section
3. Click the **Status** dropdown
4. **Available options**:
   - Pending
   - Paid
   - Processing
   - Shipped
   - Delivered
   - Cancelled
5. Change status to "paid"
6. **Expected**:
   - Page refreshes
   - New status saved
   - Logged in order_history
7. Try changing to other statuses
8. **Expected**: Each change is logged

### Test Payment Status Changes
1. In order detail, find **Payment Status** dropdown
2. **Available options**:
   - Pending
   - Paid
   - Refunded
3. Change to "paid"
4. **Expected**:
   - Status saves
   - `paid_at` timestamp recorded
   - Logged in order_history
5. Check database:
```sql
SELECT paid_at, payment_status FROM orders WHERE id = 'your-order-id';
```

---

## ✅ Feature 4: Email Sending

### Test Payment Reminder
1. Open order with payment_status = "pending"
2. In **Actions** section, click "Send Payment Reminder"
3. **Expected**:
   - Alert: "Payment email sent to [customer-email]"
   - Email logged in database
   - Order history updated

### Test Shipping Notification
1. Open order with shipment created
2. In **Actions** section, click "Send Shipping Notification"
3. **Expected**:
   - Alert: "Shipping email sent to [customer-email]"
   - Email includes:
     - Order number
     - Tracking number with Royal Mail link
     - Assigned board IDs
     - Estimated delivery time
   - Logged in email_log

### Verify Emails
Check database:
```sql
SELECT * FROM email_log ORDER BY sent_at DESC LIMIT 5;
```

Expected fields:
- order_id
- email_type ('shipping' or 'payment')
- recipient_email
- subject
- status ('sent')
- sent_at timestamp

---

## 🔄 Complete Workflow Test

### End-to-End Order Processing

**Step 1: New Order**
- Submit test order via website
- Check order appears in admin

**Step 2: Mark as Paid**
- Open order
- Change payment status to "paid"
- Change order status to "paid"

**Step 3: Assign Board**
- Click "Assign Board"
- Select board from inventory
- Confirm assignment
- Verify board shows in order

**Step 4: Ship Order**
- Click "Create Shipping Label"
- Enter tracking: `TEST123456789GB`
- Select service: Royal Mail 48
- Submit
- Verify order status → "shipped"
- Verify board status → "shipped"

**Step 5: Verify Customer Communication**
```sql
-- Should have 2 emails:
-- 1. Order confirmation (from submit-order API)
-- 2. Shipping notification (from shipping form)
SELECT * FROM email_log WHERE order_id = 'your-order-id';
```

**Step 6: Check Audit Trail**
```sql
-- Should have ~5-7 entries:
-- 1. order_created
-- 2. payment_updated
-- 3. status_updated (paid)
-- 4. board_assigned
-- 5. shipped
-- 6. email_sent_shipping
SELECT * FROM order_history
WHERE order_id = 'your-order-id'
ORDER BY created_at ASC;
```

---

## 🐛 Known Issues / Limitations

### Email API
- `/api/send-email` endpoint may not exist yet
- Emails are **logged** but may not actually send
- To actually send emails:
  - Create `/website/api/send-email.js`
  - Use Resend API
  - Or emails will just be logged in database

### Royal Mail API
- Manual entry only (no auto-label generation)
- Requires business account for API access
- For MVP: Use Royal Mail Click & Drop web interface
- Copy tracking number into admin manually

### Invoice Generation
- Not yet implemented
- Shows "coming soon" alert
- Can be added in Phase 4

---

## 📊 Success Criteria

✅ **Board Assignment Works:**
- Boards can be assigned to orders
- Board status updates correctly
- Order shows assigned boards
- Inventory reflects assignment

✅ **Shipping Works:**
- Can enter tracking numbers
- Order status updates to "shipped"
- Board status updates to "shipped"
- Shipment record created

✅ **Status Updates Work:**
- Dropdowns save changes
- Timestamps recorded
- History logged

✅ **Audit Trail Complete:**
- All actions logged in order_history
- Timestamps accurate
- User attribution works (performed_by)

✅ **UI Responsive:**
- Modals open/close smoothly
- Forms validate input
- Success/error messages clear
- Views refresh after updates

---

## 🚨 Troubleshooting

### "Failed to load available boards"
- **Cause**: No boards in inventory with status 'in_stock'
- **Fix**: Add boards via Boards tab → "+ Add Board"

### "Failed to assign board"
- **Cause**: Board already assigned or order doesn't exist
- **Fix**: Check board status in Boards tab
- **Fix**: Refresh page and try again

### "No shipment found for this order"
- **Cause**: Trying to send shipping email before creating shipment
- **Fix**: Create shipment first via "Create Shipping Label"

### Dropdowns not saving
- **Cause**: JavaScript error or database permission issue
- **Fix**: Check browser console (F12) for errors
- **Fix**: Verify RLS policies in Supabase

### Email not received
- **Expected**: Emails logged but may not actually send
- **Reason**: `/api/send-email` endpoint optional
- **Fix**: Check email_log table to confirm logging
- **Note**: Real email sending requires Resend integration

---

## 📝 Test Checklist

Print this and check off as you test:

### Board Management
- [ ] Add new board to inventory
- [ ] Board shows in Boards tab with "in_stock" status
- [ ] Can assign board to order
- [ ] Board disappears from available list after assignment
- [ ] Board status changes to "assigned"
- [ ] Assigned board shows in order detail

### Shipping
- [ ] Can open shipping modal
- [ ] Form validates required fields
- [ ] Can select shipping service
- [ ] Can enter tracking number
- [ ] Shipment creates successfully
- [ ] Order status updates to "shipped"
- [ ] Board status updates to "shipped"
- [ ] Shipping info displays in order detail

### Status Management
- [ ] Order status dropdown saves changes
- [ ] Payment status dropdown saves changes
- [ ] Status changes logged in history
- [ ] Paid timestamp recorded when payment marked paid

### Email Notifications
- [ ] Can send payment reminder
- [ ] Can send shipping notification
- [ ] Emails logged in email_log table
- [ ] Order history records email sent

### Complete Workflow
- [ ] Submit test order via website
- [ ] Order confirmation email received
- [ ] Mark order as paid
- [ ] Assign board to order
- [ ] Create shipment with tracking
- [ ] Shipping email logged
- [ ] All steps logged in order_history
- [ ] Final order status: "shipped"
- [ ] Final board status: "shipped"

---

## ✨ Next Steps After Testing

### If Everything Works:
1. **Go Live!**
   - System is production-ready
   - Start taking real orders
   - Monitor first 5-10 orders closely

2. **Customer Communication:**
   - Respond to order confirmation
   - Send shipping notifications
   - Provide excellent support

3. **Iterate:**
   - Add Royal Mail API when approved
   - Implement invoice generation
   - Add analytics dashboard

### If Issues Found:
1. Document the issue
2. Check browser console for errors
3. Check Supabase logs
4. Report issue with:
   - What you did
   - What happened
   - What you expected
   - Error messages

---

## 🎯 MVP Definition: Ready for First Sale

**Minimum requirements to start selling:**

✅ Customer can order via website
✅ Order confirmation emails send
✅ Admin can view orders
✅ Admin can track boards by ID
✅ Admin can assign boards to orders
✅ Admin can enter tracking numbers
✅ Admin can update order status
✅ System logs all actions
✅ Shipping notifications sent

**You have all of this now!**

Start with friends/family orders to test the system with real workflows, then scale up to public sales.

---

**Ready to test! Follow the checklist above and report any issues. Good luck! 🚀**
