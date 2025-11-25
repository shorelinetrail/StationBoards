# PayPal Integration Setup Guide

## Overview

Your StationBoards checkout integrates with PayPal to accept payments. You need to configure PayPal API credentials to enable this functionality.

## Step 1: Create a PayPal Developer Account

1. Go to [PayPal Developer Dashboard](https://developer.paypal.com/)
2. Log in with your PayPal account (or create one)
3. Accept the developer terms if prompted

## Step 2: Create a Sandbox App (For Testing)

**Sandbox** allows you to test payments without real money.

1. In the [PayPal Developer Dashboard](https://developer.paypal.com/dashboard/), click **Apps & Credentials**
2. Make sure you're on the **Sandbox** tab
3. Click **Create App**
4. Enter app details:
   - **App Name**: `StationBoards Sandbox`
   - **App Type**: Select **Merchant**
5. Click **Create App**

## Step 3: Get Your Sandbox Credentials

After creating the app, you'll see:

```
Client ID: AxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
Secret: ExxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
```

Copy these values!

## Step 4: Add to Environment Variables

Add these to your `.env.local` file in the `/website` directory:

```bash
# PayPal Configuration (Sandbox - for testing)
PAYPAL_CLIENT_ID=AxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
PAYPAL_CLIENT_SECRET=ExxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
PAYPAL_MODE=sandbox
```

## Step 5: Test the Integration

1. Start your development server:
   ```bash
   cd website
   npm run dev
   ```

2. Go to `http://localhost:3000/checkout.html`
3. Fill in the checkout form
4. Click the **PayPal** button
5. You'll be redirected to PayPal sandbox
6. Log in with a **sandbox buyer account** (see below)

### Creating Sandbox Test Accounts

1. In PayPal Developer Dashboard, go to **Testing Tools** → **Sandbox Accounts**
2. You'll see default accounts or can create new ones:
   - **Business Account**: Receives payments (your merchant account)
   - **Personal Account**: Makes payments (test buyer account)
3. Click **View/Edit Account** to see login credentials
4. Use these credentials to test payments

### Sandbox Buyer Account Example
```
Email: sb-buyer47802@business.example.com
Password: (shown in sandbox accounts section)
```

## Step 6: Production Setup (When Ready to Go Live)

### Create a Live App

1. In PayPal Developer Dashboard, click **Apps & Credentials**
2. Switch to the **Live** tab
3. Click **Create App**
4. Enter app details:
   - **App Name**: `StationBoards`
   - **App Type**: Select **Merchant**
5. Click **Create App**

### Get Live Credentials

Copy the **Live** Client ID and Secret:

```
Client ID: AfxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
Secret: ExxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
```

### Update Environment Variables for Production

In your Vercel dashboard (or production environment):

```bash
# PayPal Configuration (LIVE - for production)
PAYPAL_CLIENT_ID=AfxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
PAYPAL_CLIENT_SECRET=ExxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxQ
PAYPAL_MODE=live
```

**⚠️ IMPORTANT**: Never commit real credentials to Git! Always use environment variables.

## Vercel Deployment

### Add Environment Variables in Vercel

1. Go to your Vercel project dashboard
2. Click **Settings** → **Environment Variables**
3. Add the following:

| Name | Value | Environment |
|------|-------|-------------|
| `PAYPAL_CLIENT_ID` | Your Client ID | Production, Preview, Development |
| `PAYPAL_CLIENT_SECRET` | Your Secret | Production, Preview, Development |
| `PAYPAL_MODE` | `live` or `sandbox` | Production = `live`, Others = `sandbox` |

4. Click **Save**
5. Redeploy your application

## Webhook Setup (For Payment Confirmations)

PayPal webhooks notify your server when payments complete.

### Configure Webhook URL

1. In PayPal Developer Dashboard, go to your app
2. Scroll to **Webhooks**
3. Click **Add Webhook**
4. Enter webhook URL:
   ```
   https://stationboards.co.uk/api/paypal-capture
   ```
5. Select event types:
   - ✅ **Payment capture completed**
   - ✅ **Payment capture denied**
   - ✅ **Payment capture pending**
   - ✅ **Payment capture refunded**
   - ✅ **Checkout order completed**
6. Click **Save**

## Testing Checklist

- [ ] PayPal credentials added to `.env.local`
- [ ] Development server running
- [ ] Checkout form loads correctly
- [ ] PayPal button is visible and clickable
- [ ] Clicking PayPal redirects to sandbox login
- [ ] Can log in with sandbox buyer account
- [ ] Can complete test payment
- [ ] Redirected back to success page
- [ ] Order status updates to "paid" in admin dashboard
- [ ] Confirmation email sent to customer

## Troubleshooting

### Error: "PayPal payment is not configured"

**Cause**: Missing `PAYPAL_CLIENT_ID` or `PAYPAL_CLIENT_SECRET`

**Solution**:
1. Check `.env.local` file exists in `/website` directory
2. Verify credentials are correctly copied
3. Restart development server after adding env variables

### Error: "Failed to create PayPal order"

**Cause**: Invalid credentials or API error

**Solutions**:
1. Verify Client ID and Secret are correct
2. Check you're using **Sandbox** credentials for testing
3. Ensure `PAYPAL_MODE=sandbox` is set
4. Check PayPal Developer Dashboard for app status

### Payment succeeds but order not updated

**Cause**: Webhook not configured or failing

**Solutions**:
1. Check webhook URL is correct
2. Verify webhook is receiving events in PayPal dashboard
3. Check server logs for webhook errors
4. Ensure `/api/paypal-capture` endpoint is working

### "Merchant account is restricted"

**Cause**: PayPal app needs approval or business verification

**Solutions**:
1. Use **Sandbox** mode for testing (no approval needed)
2. For **Live** mode, complete PayPal business verification
3. Check PayPal Developer Dashboard for app status

## Support

For PayPal-specific issues:
- [PayPal Developer Documentation](https://developer.paypal.com/docs/)
- [PayPal Developer Forums](https://www.paypal-community.com/t5/Developer-Central/ct-p/developer-central)
- [PayPal Developer Support](https://developer.paypal.com/support/)

For StationBoards integration issues, check the server logs:
```bash
# Vercel logs
vercel logs

# Local development
npm run dev
# Check terminal output
```

## Security Best Practices

1. ✅ Never commit credentials to Git
2. ✅ Use environment variables for all secrets
3. ✅ Use Sandbox for development/testing
4. ✅ Use Live credentials only in production
5. ✅ Rotate credentials if exposed
6. ✅ Enable webhook signature validation
7. ✅ Monitor transactions in PayPal dashboard
8. ✅ Set up fraud filters in PayPal account

## What Happens During Checkout

1. **Customer fills checkout form** → Order created in database (status: `pending`)
2. **Customer clicks PayPal button** → API creates PayPal order
3. **Redirect to PayPal** → Customer logs in and approves payment
4. **PayPal processes payment** → Funds captured
5. **Return to success page** → Customer sees confirmation
6. **Webhook received** → Order status updated to `paid`
7. **Email sent** → Customer receives confirmation email
8. **Admin dashboard updated** → Order visible with "paid" status

---

**Last Updated**: 2025-11-24
**Integration Status**: ✅ PayPal REST API v2
**Supported**: Sandbox & Live modes
**Currency**: GBP (British Pounds)
