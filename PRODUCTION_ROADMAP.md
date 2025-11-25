# StationBoards - Production Roadmap

**Goal:** Launch order management system for production sales

---

## ✅ Phase 1: Foundation (COMPLETED)

**What's Built & Working:**

### Frontend
- ✅ Sales website deployed on Vercel
- ✅ Live OLED display demo (256×64, realistic Kings Cross departures)
- ✅ Customer order form with validation
- ✅ Responsive design for mobile/desktop

### Database (Supabase)
- ✅ Complete schema with 5 tables (orders, boards, shipments, order_history, email_log)
- ✅ Row Level Security (RLS) policies configured
- ✅ Auto-generated order numbers (SB-YYYYMMDD-XXXX)
- ✅ Board ID tracking system
- ✅ Indexes for performance
- ✅ Audit trail (order_history)

### Admin Dashboard
- ✅ Authentication with Supabase Auth
- ✅ Orders list with filters and search
- ✅ Order detail modal (customer info, shipping, status)
- ✅ Boards inventory tab
- ✅ Add new boards to inventory
- ✅ Board status tracking (in_stock, assigned, shipped, active)

### Backend APIs
- ✅ Order submission endpoint (`/api/submit-order`)
- ✅ Email service integration (Resend)
- ✅ Order confirmation emails
- ✅ Royal Mail API wrapper (ready for credentials)

**Current Limitations:**
- ⏳ Board assignment (stub function exists)
- ⏳ Shipping label generation (stub function exists)
- ⏳ Manual email sending (stub function exists)
- ⏳ Invoice generation (stub function exists)
- ⏳ Royal Mail requires API credentials

---

## 🚀 Phase 2: Core Production Features (NEXT - 3-5 days)

**Priority: HIGH - Essential for first sales**

### 2.1 Board Assignment (Day 1)
**Implement:** Assign boards from inventory to orders

**Implementation:**
```javascript
// Admin dashboard: showAssignBoardDialog()
1. Show modal with available boards (status = 'in_stock')
2. User selects board ID from dropdown
3. Call Supabase to update:
   - boards.order_id = order_id
   - boards.status = 'assigned'
   - boards.assigned_at = NOW()
   - log in order_history
4. Refresh order details
```

**Files to modify:**
- `/website/admin/admin-script.js:582` - Replace stub with full implementation
- `/website/admin/index.html` - Add assignment modal HTML

**Testing:**
- Add 5 test boards to inventory
- Create test order
- Assign board
- Verify board shows in order details
- Check board status changed in Boards tab

---

### 2.2 Manual Shipping & Email (Day 2)
**Implement:** Send shipping notifications without Royal Mail integration

**Implementation:**
```javascript
// For MVP: Manual shipping workflow
1. Admin enters tracking number manually
2. System creates shipment record
3. Sends email with tracking number
4. Updates order status to 'shipped'
```

**Why Manual First:**
- Royal Mail API requires business account approval (2-4 weeks)
- You can ship via Click & Drop web interface manually
- Copy tracking numbers into admin dashboard
- Still tracks everything in database

**Features:**
- Enter tracking number manually
- Send shipping notification email
- Update order status
- Log in order_history

**Files to modify:**
- `/website/admin/admin-script.js:586` - Manual shipment entry
- `/website/admin/admin-script.js:590` - Send email function
- `/website/lib/email.js` - Use existing shipping email template

---

### 2.3 Order Status Updates (Day 3)
**Implement:** Update payment and order status

**Already exists in UI, just needs wiring:**
```javascript
// Update order status (pending → paid → shipped → delivered)
// Update payment status (pending → paid → refunded)
```

**Files to modify:**
- `/website/admin/admin-script.js` - Add update functions
- Wire up dropdowns in order detail modal

---

### 2.4 Testing & Bug Fixes (Day 4-5)

**End-to-End Test:**
1. Customer submits order → Receives email ✅
2. Admin sees order → Opens details ✅
3. Admin marks paid → Updates status ✅
4. Admin assigns board → Board tracked ✅
5. Admin enters tracking → Email sent ✅
6. Customer receives board → Marks delivered ✅

**Verify:**
- All emails sending correctly
- Order history logging all actions
- Board lifecycle (in_stock → assigned → shipped)
- Search and filters working
- Mobile responsive

---

## 🔄 Phase 3: Automation (1-2 weeks)

**Priority: MEDIUM - Reduces manual work**

### 3.1 Royal Mail API Integration
**When:** After Royal Mail approves business account

**Implementation:**
- Request API credentials from Royal Mail
- Test in sandbox environment
- Implement label generation
- Download PDF labels directly
- Auto-create tracking numbers

**Files ready:**
- `/website/lib/royal-mail.js` - Already has API wrapper
- Just needs credentials added to `.env.local`

---

### 3.2 Automated Reminders
**Features:**
- Payment reminder emails (if unpaid after 3 days)
- Delivery confirmation requests
- Review requests (after delivery)

**Implementation:**
- Vercel Cron Jobs or Supabase Edge Functions
- Check order status daily
- Send automated emails

---

### 3.3 Board Setup Instructions
**Features:**
- Automatic setup email when board assigned
- Include WiFi setup guide
- Configuration portal link
- Support contact

**Files to modify:**
- `/website/lib/email.js` - Add setup email template

---

## 📊 Phase 4: Analytics & Optimization (Ongoing)

**Priority: LOW - Nice to have**

### 4.1 Dashboard Analytics
- Total sales revenue
- Orders by status (pending, paid, shipped)
- Boards in stock vs assigned
- Average time to ship
- Customer locations (heatmap)

### 4.2 Customer Portal
- Order tracking page (public, no login)
- Customer enters email + order number
- See order status and tracking

### 4.3 Inventory Management
- Low stock alerts
- Reorder notifications
- Bulk board import (CSV)
- Manufacturing batch tracking

### 4.4 Financial Tools
- Invoice generation (PDF)
- Payment integration (Stripe)
- Sales reports
- Tax calculations

---

## 🔒 Production Checklist

### Security
- [ ] Environment variables secured (not in git)
- [ ] RLS policies tested (authenticated users only)
- [ ] API rate limiting configured
- [ ] HTTPS enforced (Vercel handles this ✅)
- [ ] SQL injection prevention (Supabase handles this ✅)

### Performance
- [ ] Database indexes created ✅
- [ ] Image optimization
- [ ] CDN for static assets (Vercel handles this ✅)
- [ ] API response caching

### Monitoring
- [ ] Error tracking (Sentry or similar)
- [ ] Uptime monitoring (UptimeRobot)
- [ ] Email delivery monitoring (Resend dashboard)
- [ ] Database backup schedule (Supabase handles this ✅)

### Legal & Compliance
- [ ] Privacy policy on website
- [ ] Terms & conditions
- [ ] GDPR compliance (if selling to EU)
- [ ] Cookie consent (if using analytics)

### Documentation
- [ ] Admin user guide
- [ ] Customer FAQ
- [ ] Setup troubleshooting
- [ ] Developer handoff docs

---

## 💰 Cost Estimate (Monthly)

**Current Stack:**
- **Vercel:** £0 (Hobby plan, upgrade to Pro £20/mo if needed)
- **Supabase:** £0 (Free tier: 500MB, 2GB bandwidth)
  - Upgrade to Pro £25/mo at ~100 orders/month
- **Resend:** £0 (Free tier: 100 emails/day)
  - Upgrade to Pro £20/mo at 3000+ emails/month
- **Royal Mail API:** £0 (no monthly fee, pay per label)

**First Month Estimate:** £0-£20
**Scaling (100 orders/month):** £45-£65

---

## 🎯 Immediate Next Steps (Today/Tomorrow)

### Day 1: Board Assignment
1. **Implement `showAssignBoardDialog()` function**
   - Load available boards (status = 'in_stock')
   - Show selection modal
   - Update board.order_id
   - Update board.status = 'assigned'
   - Log action in order_history

2. **Add assignment modal HTML**
   - Dropdown of available boards
   - Confirmation button
   - Success/error messages

3. **Test workflow:**
   - Add 3 test boards
   - Create test order
   - Assign board
   - Verify it works

### Day 2: Manual Shipping
1. **Add manual tracking entry**
   - Input field for tracking number
   - Service dropdown (Royal Mail 48, 24, Special Delivery)
   - Save to shipments table

2. **Send shipping notification**
   - Use existing email template
   - Include tracking number and link
   - Update order status to 'shipped'

3. **Test:**
   - Assign board to order
   - Enter fake tracking number
   - Send email
   - Check email received

### Day 3: Polish & Test
1. **Wire up status updates**
   - Payment status dropdown
   - Order status dropdown
   - Save to database

2. **End-to-end test**
   - Complete workflow from order to shipping

3. **Fix any bugs**

---

## 📈 Launch Strategy

### Soft Launch (Week 1)
- Test with 3-5 friends/family orders
- Collect feedback
- Fix critical bugs
- Refine workflow

### Public Launch (Week 2)
- Announce on social media
- Share with railway enthusiast communities
- Monitor first 10 orders closely
- Provide exceptional customer service

### Scale (Week 3+)
- Implement Royal Mail API
- Add automation
- Optimize processes
- Collect reviews

---

## 🤝 Support Plan

### Customer Support Channels
1. **Email:** hello@stationboards.co.uk (via Resend)
2. **Order Issues:** Check admin dashboard
3. **Setup Help:** Email setup guide with each board

### Admin Support (For You)
1. **Database:** Supabase dashboard + SQL editor
2. **Emails:** Resend dashboard for delivery logs
3. **Debugging:** Browser console + Network tab
4. **This Repo:** All code and docs

---

## ⚡ Quick Win: Launch in 3 Days

**Minimum Viable Product (MVP):**

**Day 1 Morning:**
- Implement board assignment (3 hours)

**Day 1 Afternoon:**
- Add manual shipping entry (2 hours)

**Day 2 Morning:**
- Wire up status updates (2 hours)

**Day 2 Afternoon:**
- End-to-end testing (3 hours)

**Day 3:**
- Soft launch with 1-2 test customers
- Monitor and fix any issues

**You can start selling by Day 3** with:
✅ Order form
✅ Email confirmations
✅ Board assignment
✅ Manual shipping
✅ Customer tracking

Royal Mail API can be added later without disrupting service.

---

## 📞 Decision Points

**Do you want to:**

1. **Start with Phase 2 implementation?**
   - I can implement board assignment now (30 mins)
   - Add manual shipping entry (30 mins)
   - Wire up status updates (20 mins)
   - = Ready to test in 1.5 hours

2. **Do a thorough review first?**
   - Test every feature end-to-end
   - Document what works vs what needs work
   - Create detailed task list

3. **Focus on specific feature?**
   - Let me know what's most important
   - I'll prioritize that first

**What would you like to tackle first?**
