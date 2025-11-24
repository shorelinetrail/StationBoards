# StationBoards Order Management System - Setup Guide

Complete order management system with board ID tracking, Royal Mail integration, and email notifications.

## 🏗️ Architecture

- **Database**: Supabase (PostgreSQL)
- **Backend**: Vercel Serverless Functions
- **Email**: Resend
- **Shipping**: Royal Mail Click & Drop API
- **Admin Dashboard**: Static HTML + Supabase Auth

## 📋 Features

✅ **Order Management**
- Customer order submission form
- Order confirmation emails
- Admin dashboard to view/manage all orders
- Status tracking (pending → paid → processing → shipped → delivered)
- Payment status tracking

✅ **Board Inventory**
- Track individual boards by ID (ESP32 chip ID)
- Assign boards to orders
- Track board status (in_stock, assigned, shipped, active, faulty)
- Monitor which board is shipped to which customer

✅ **Shipping Integration**
- Royal Mail API integration for postage
- Generate shipping labels
- Track packages
- Automatic customer notifications

✅ **Email Notifications**
- Order confirmation
- Payment reminders
- Shipping notifications with tracking

## 🚀 Setup Instructions

### 1. Supabase Setup

#### Create Supabase Project
1. Go to [supabase.com](https://supabase.com) and create a new project
2. Wait for the project to be ready
3. Note down your project URL and API keys

#### Run Database Schema
1. Go to SQL Editor in Supabase dashboard
2. Open `/supabase/schema.sql`
3. Copy and paste the entire SQL into the editor
4. Click "Run"

This creates:
- `orders` table
- `boards` table
- `shipments` table
- `order_history` table
- `email_log` table
- Storage bucket for shipping labels
- Row Level Security policies
- Indexes for performance

#### Create Admin User
1. Go to Authentication → Users
2. Click "Add user"
3. Enter email and password
4. This user can login to the admin dashboard

### 2. Resend Setup (Email Service)

1. Go to [resend.com](https://resend.com) and sign up
2. Add and verify your domain (e.g., stationboards.co.uk)
3. Get your API key from the dashboard
4. Add DNS records to verify domain:
   - SPF record
   - DKIM record
5. Test sending with the test API

### 3. Royal Mail Setup (Optional for now)

#### Register for Click & Drop
1. Sign up at [Royal Mail Click & Drop](https://www.royalmail.com/d2d/clickanddrop)
2. Get production API access (requires business account)
3. Note down Client ID and Client Secret

#### API Documentation
- [Royal Mail Shipping API Docs](https://developer.royalmail.net/api/shipping)
- Services: Royal Mail 24/48 Tracked, Special Delivery, etc.

### 4. Vercel Deployment

#### Install Dependencies
```bash
cd website
npm install
```

#### Environment Variables
Create `.env.local` for local development:

```bash
# Supabase
NEXT_PUBLIC_SUPABASE_URL=https://xxxxx.supabase.co
NEXT_PUBLIC_SUPABASE_ANON_KEY=your-anon-key
SUPABASE_SERVICE_ROLE_KEY=your-service-role-key

# Resend
RESEND_API_KEY=re_xxxxx
FROM_EMAIL=orders@stationboards.co.uk

# Royal Mail (optional)
ROYAL_MAIL_CLIENT_ID=your-client-id
ROYAL_MAIL_CLIENT_SECRET=your-client-secret
ROYAL_MAIL_API_URL=https://api.royalmail.net/shipping/v3

# URLs
WEBSITE_URL=https://stationboards.co.uk
ADMIN_DASHBOARD_URL=https://stationboards.co.uk/admin
```

#### Deploy to Vercel
```bash
# Install Vercel CLI
npm i -g vercel

# Login
vercel login

# Deploy
vercel --prod
```

#### Add Environment Variables to Vercel
1. Go to your Vercel project settings
2. Add all environment variables from `.env.local`
3. Redeploy

### 5. Admin Dashboard Configuration

Update `/website/admin/admin-script.js`:

```javascript
const SUPABASE_URL = 'https://xxxxx.supabase.co';
const SUPABASE_ANON_KEY = 'your-anon-key';
```

Or use environment variables if building with a framework.

## 📖 User Guide

### For Customers

1. **Place Order**
   - Fill out form on website
   - Receive confirmation email with order number
   - Receive payment instructions within 24 hours

2. **Make Payment**
   - Follow instructions in email
   - Receive payment confirmation

3. **Track Order**
   - Receive shipping notification with tracking number
   - Track via Royal Mail website

### For Admin

#### Access Dashboard
1. Go to `https://yoursite.com/admin`
2. Login with your admin credentials

#### Manage Orders

**View Orders**
- See all orders in one place
- Filter by status
- Search by customer name, email, or order number

**Process Order**
1. Click on order to see details
2. Update payment status when received
3. Assign board IDs to order
4. Update order status to "processing"
5. Create shipping label
6. Update status to "shipped"
7. System sends automatic shipping notification

**Assign Boards**
1. Open order details
2. Click "Assign Board"
3. Select available board from inventory
4. Board status updates to "assigned"

**Send Emails**
- Payment Reminder: Remind customer to pay
- Shipping Notification: Send when order ships

#### Manage Board Inventory

**Add New Board**
1. Go to "Boards" tab
2. Click "+ Add Board"
3. Enter Board ID (ESP32 chip ID)
4. Optional: firmware version, hardware revision
5. Board automatically marked as "in_stock"

**Track Boards**
- See all boards in inventory
- Filter by status
- See which order each board is assigned to
- Track when boards are activated (connect to monitoring)

**Board Statuses**
- `in_stock`: Available for assignment
- `assigned`: Assigned to order, not yet shipped
- `shipped`: Shipped to customer
- `active`: Connected to monitoring (customer using it)
- `faulty`: Hardware issue reported
- `returned`: Returned by customer

### Shipping Workflow

1. **Prepare Order**
   - Assign board(s) to order
   - Update order status to "processing"

2. **Create Label** (via Royal Mail API)
   - Click "Create Shipping Label"
   - Select service (24/48 Tracked, etc.)
   - Enter package weight/dimensions
   - Generate label PDF
   - Label stored in Supabase Storage

3. **Print & Ship**
   - Download and print label
   - Attach to package
   - Drop off at Post Office
   - System creates manifest

4. **Automatic Notifications**
   - Customer receives tracking email
   - Email includes tracking number
   - Includes board IDs shipped
   - Includes setup guide link

## 🔧 Royal Mail Integration

### Implementation Plan

The Royal Mail integration (in `/website/lib/royal-mail.js`) will handle:

1. **Authentication**
   ```javascript
   // OAuth2 authentication
   POST /token
   ```

2. **Create Shipment**
   ```javascript
   POST /shipments
   // Returns tracking number + label URL
   ```

3. **Generate Label**
   ```javascript
   GET /shipments/{shipmentId}/label
   // Returns PDF
   ```

4. **Create Manifest**
   ```javascript
   POST /manifests
   // Required before dropping off at Post Office
   ```

5. **Track Package**
   ```javascript
   GET /shipments/{shipmentId}/tracking
   ```

### Service Codes
- `CRL24`: Royal Mail 24 Tracked
- `CRL48`: Royal Mail 48 Tracked
- `SD1`: Special Delivery Guaranteed by 1pm
- `TPL`: Tracked 48 Large Letter

### Package Specifications
For StationBoard (7.5" display + ESP32):
- Weight: ~400g
- Dimensions: 22cm x 14cm x 4cm
- Service: Royal Mail 48 Tracked (£4.20)
- Insurance: Up to £50 included

## 📊 Database Schema

### Orders Table
- Customer info
- Shipping address
- Order details (quantity, price)
- Status tracking
- Timestamps

### Boards Table
- Board ID (unique identifier)
- Linked to order
- Manufacturing info
- Status tracking

### Shipments Table
- Linked to order (one-to-one)
- Royal Mail tracking number
- Service details
- Label URL
- Delivery tracking

### Order History
- Activity log for each order
- Who did what and when

### Email Log
- Track all sent emails
- Status (sent, delivered, failed)

## 🔐 Security

- **Supabase RLS**: Row Level Security ensures only admins can access sensitive data
- **Service Role Key**: Only used in API routes, never exposed to client
- **Authentication**: Admin dashboard requires Supabase Auth login
- **CORS**: API endpoints protected with proper CORS headers

## 🧪 Testing

### Test Order Submission
```bash
curl -X POST https://yoursite.com/api/submit-order \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Test Customer",
    "email": "test@example.com",
    "phone": "07700900000",
    "address": "123 Test St",
    "city": "London",
    "postcode": "SW1A 1AA",
    "quantity": 1,
    "notes": "Test order"
  }'
```

### Test Email
```javascript
// In Resend dashboard, use test API key
// Or use test mode to avoid sending real emails
```

## 📈 Next Steps

1. **Implement Royal Mail Integration**
   - Complete API wrapper
   - Add shipping label generation to admin dashboard
   - Test with Royal Mail sandbox

2. **Payment Integration**
   - Stripe or PayPal for online payments
   - Automatic payment confirmation

3. **Automated Workflows**
   - Auto-assign boards when payment received
   - Auto-send shipping notification when label created
   - Auto-update status when tracking shows delivered

4. **Customer Portal**
   - Allow customers to track their order
   - View board ID and setup instructions
   - Download invoices

5. **Analytics**
   - Order volume dashboard
   - Revenue tracking
   - Inventory forecasting

## 🆘 Support

### Common Issues

**Order submission fails**
- Check Supabase connection
- Verify environment variables
- Check browser console for errors

**Email not sending**
- Verify Resend API key
- Check domain verification
- Check email_log table for errors

**Admin dashboard not loading**
- Update Supabase credentials in `admin-script.js`
- Clear browser cache
- Check authentication

### Debug Mode
Enable console logging to see detailed errors:
```javascript
// In admin-script.js
console.log('Debug:', data);
```

## 📞 Contact

For technical support:
- Email: dev@stationboards.co.uk
- GitHub: Create an issue

---

Built with ❤️ for StationBoards
