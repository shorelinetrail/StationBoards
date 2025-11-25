# Quick Start Guide - StationBoards Order System

## 🎉 What's Been Built

A complete order management system for your StationBoards business with:

✅ **Customer order form** on your website
✅ **Order confirmation emails** (Resend)
✅ **Admin dashboard** to manage orders
✅ **Board ID tracking** - know which board goes to which customer
✅ **Royal Mail integration** for shipping labels
✅ **Automatic email notifications** throughout order lifecycle

## ⚡ Quick Setup (15 minutes)

### 1. Create Supabase Project (5 min)

```bash
1. Go to https://supabase.com
2. Create new project
3. Copy your URL and keys
4. Go to SQL Editor
5. Paste contents of supabase/schema.sql
6. Click "Run"
7. Done! ✅
```

### 2. Setup Resend for Emails (5 min)

```bash
1. Go to https://resend.com
2. Sign up and verify your email domain
3. Get your API key
4. Add DNS records for SPF/DKIM
5. Done! ✅
```

### 3. Deploy to Vercel (5 min)

```bash
cd website
npm install

# Add environment variables to .env.local:
NEXT_PUBLIC_SUPABASE_URL=your-url
NEXT_PUBLIC_SUPABASE_ANON_KEY=your-key
SUPABASE_SERVICE_ROLE_KEY=your-service-key
RESEND_API_KEY=your-resend-key
FROM_EMAIL=orders@stationboards.co.uk

# Deploy
vercel --prod
```

### 4. Create Admin User

```bash
1. In Supabase: Auth → Users → Add user
2. Enter your email + password
3. Done! ✅
```

## 🎯 How It Works

### Customer Places Order

1. Customer fills form at `yoursite.com`
2. Order saved to Supabase
3. Customer receives confirmation email
4. You see order in admin dashboard

### You Process Order

1. Login to `yoursite.com/admin`
2. Click order to see details
3. Update payment status when paid
4. Click "Assign Board" → select board from inventory
5. Board now linked to customer
6. Click "Create Shipping Label"
7. Download label, attach to package
8. Customer gets automatic shipping email with tracking

### Customer Receives Board

1. Tracking shows delivered
2. Customer sets up board
3. Board connects to monitoring
4. You see board status change to "active"

## 📊 Admin Dashboard Features

### Orders Tab
- View all orders
- Filter by status (pending/paid/shipped/delivered)
- Search by customer name or email
- Click order for full details
- Update order status
- Update payment status
- Assign boards
- Create shipping labels
- Send emails (payment reminders, shipping notifications)

### Boards Tab
- See all boards in inventory
- Add new boards (enter ESP32 chip ID)
- Track board status
- See which customer has which board
- Stats: Total, In Stock, Assigned, Active

## 📦 Board ID Tracking

**Why track Board IDs?**
- Know exactly which board you shipped to which customer
- Track warranty and support issues
- See which boards are active vs faulty
- Maintain manufacturing records

**How to add boards:**
1. Go to Boards tab
2. Click "+ Add Board"
3. Enter Board ID (ESP32 chip ID like `ESP32-A1B2C3D4`)
4. Optional: firmware version, hardware revision
5. Board marked as "in_stock"

**Board lifecycle:**
```
in_stock → assigned → shipped → active
            ↓
         faulty / returned
```

## 📧 Email Templates

### Order Confirmation
- Sent immediately when order placed
- Includes order number
- Shows quantity and total
- Tells customer what happens next

### Shipping Notification
- Sent when you create shipping label
- Includes tracking number and link
- Shows board IDs being shipped
- Includes setup guide link

### Payment Reminder
- Send manually from admin dashboard
- Reminds customer to complete payment

## 🚚 Royal Mail Integration

**Services available:**
- Royal Mail 48 Tracked (£4.20) - Recommended
- Royal Mail 24 Tracked (£5.70)
- Special Delivery by 1pm (£10.50)

**How to use:**
1. Process order in admin dashboard
2. Click "Create Shipping Label"
3. Select service
4. Enter weight: 400g (typical for board)
5. Dimensions: 22cm x 14cm x 4cm
6. Download PDF label
7. Print and attach to package
8. Drop at Post Office

**Tracking:**
- Automatic tracking updates
- Customer can track via Royal Mail website
- You see status in admin dashboard

## 🔧 Configuration Files

### Environment Variables (.env.local)
```bash
# Supabase
NEXT_PUBLIC_SUPABASE_URL=https://xxx.supabase.co
NEXT_PUBLIC_SUPABASE_ANON_KEY=eyJxxx
SUPABASE_SERVICE_ROLE_KEY=eyJxxx

# Email
RESEND_API_KEY=re_xxx
FROM_EMAIL=orders@stationboards.co.uk

# Royal Mail (optional for now)
ROYAL_MAIL_CLIENT_ID=xxx
ROYAL_MAIL_CLIENT_SECRET=xxx

# URLs
WEBSITE_URL=https://stationboards.co.uk
```

### Admin Dashboard
Update `website/admin/admin-script.js` line 2-3:
```javascript
const SUPABASE_URL = 'https://xxx.supabase.co';
const SUPABASE_ANON_KEY = 'eyJxxx';
```

## 📁 File Structure

```
website/
├── index.html              # Main website (existing)
├── script.js               # Updated with new API call
├── package.json            # Dependencies
├── .env.example            # Environment template
│
├── admin/                  # Admin dashboard
│   ├── index.html         # Dashboard UI
│   ├── admin-styles.css   # Styling
│   └── admin-script.js    # Dashboard logic
│
├── api/                    # Vercel serverless functions
│   └── submit-order.js    # Order submission endpoint
│
└── lib/                    # Shared libraries
    ├── supabase.js        # Supabase client & helpers
    ├── email.js           # Email templates & sending
    └── royal-mail.js      # Royal Mail API integration

supabase/
└── schema.sql              # Database schema
```

## 🎓 Training Videos (TODO)

Would you like me to create:
- Video walkthrough of admin dashboard?
- Step-by-step order processing guide?
- Royal Mail integration demo?

## 🆘 Troubleshooting

**Orders not submitting?**
- Check Supabase connection in browser console
- Verify environment variables in Vercel
- Check Network tab for API errors

**Emails not sending?**
- Verify Resend API key
- Check domain DNS records
- Look at email_log table in Supabase

**Can't login to admin?**
- Make sure you created user in Supabase Auth
- Check Supabase credentials in admin-script.js
- Try password reset

**Royal Mail not working?**
- This is optional for now - you can manually create labels
- Need Royal Mail business account for API access
- Contact Royal Mail for API credentials

## 📞 Next Steps

1. **Test the system:**
   - Place a test order
   - Check if email arrives
   - Login to admin dashboard
   - Try assigning a board

2. **Add your boards:**
   - Go to Boards tab
   - Add 5-10 boards to start
   - Use real ESP32 chip IDs

3. **Customize emails:**
   - Edit `website/lib/email.js`
   - Update colors, logo, text
   - Test with your email

4. **Setup Royal Mail:**
   - Create business account
   - Request API access
   - Get credentials
   - Test in sandbox

## 💡 Tips

- Start with manual shipping labels while testing
- Add boards to inventory before processing orders
- Check email_log table to debug email issues
- Use order_history to see all changes to orders
- Export data regularly for backups

## 📚 Full Documentation

See `ORDERS_SETUP_GUIDE.md` for:
- Detailed setup instructions
- Database schema explanation
- API documentation
- Security best practices
- Advanced features

---

**Ready to sell some boards! 🚉📦**

Questions? Check the full guide or open an issue on GitHub.
