# StationBoards Sales Website

A professional landing page for selling StationBoards - live departure boards for your home.

## Features

- **Modern Design**: Beautiful gradient hero section with animated board preview
- **Responsive**: Mobile-first design that works on all devices
- **Complete Sales Flow**: Features, specs, pricing, and order form
- **FAQ Section**: Answers common customer questions
- **Email Fallback**: Order form falls back to email if API not available

## Deployment Options

### Option 1: Static Hosting (Recommended)
Deploy to any static hosting service:

- **Netlify**: Drag and drop the `website` folder
- **Vercel**: Import from GitHub and set root to `website`
- **GitHub Pages**: Push to repo and enable Pages
- **AWS S3**: Upload files and enable static website hosting
- **Cloudflare Pages**: Connect repo and deploy

### Option 2: Railway (With Backend)
Deploy alongside the monitoring backend:

1. Add to Railway project
2. Set root directory to `website`
3. Configure build command: `echo "No build needed"`
4. Configure start command: `npx serve -s . -p $PORT`

### Option 3: Custom Domain
Point your domain `stationboards.co.uk` to the hosting service

## Order Processing

The website currently uses two methods for order processing:

1. **API Endpoint** (Preferred): POST to `/api/orders`
2. **Email Fallback**: Opens mailto link with order details

### Implementing the Orders API

Add this to your `backend/server.js`:

```javascript
app.post('/api/orders', (req, res) => {
  const order = req.body;

  // Save to database
  db.run(
    'INSERT INTO orders (name, email, phone, address, city, postcode, quantity, notes, total, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)',
    [order.name, order.email, order.phone, order.address, order.city, order.postcode, order.quantity, order.notes, order.total, order.timestamp],
    (err) => {
      if (err) {
        res.status(500).json({ error: 'Failed to save order' });
      } else {
        // Send confirmation email
        sendOrderConfirmation(order);
        res.json({ success: true });
      }
    }
  );
});
```

## Customization

### Change Pricing
Edit `script.js` line 13:
```javascript
const pricing = {
  1: 89,    // 1 board
  2: 168,   // 2 boards
  3: 240,   // 3 boards
  5: 385    // 5 boards
};
```

### Update Contact Email
Replace `orders@stationboards.co.uk` throughout the files

### Add Product Images
Replace the animated board preview with real product photos

### Update Colors
Edit CSS variables in `styles.css` line 11:
```css
:root {
  --primary: #667eea;
  --secondary: #764ba2;
  /* ... */
}
```

## Analytics

Add your analytics tracking code to `index.html` before `</head>`:

```html
<!-- Google Analytics -->
<script async src="https://www.googletagmanager.com/gtag/js?id=GA_MEASUREMENT_ID"></script>
<script>
  window.dataLayer = window.dataLayer || [];
  function gtag(){dataLayer.push(arguments);}
  gtag('js', new Date());
  gtag('config', 'GA_MEASUREMENT_ID');
</script>
```

## SEO

The site includes:
- Meta descriptions
- Semantic HTML
- Schema markup ready (add product schema)
- Fast loading (no external dependencies)
- Mobile responsive

### Add Schema.org Product Markup

Add this to `<head>`:
```html
<script type="application/ld+json">
{
  "@context": "https://schema.org/",
  "@type": "Product",
  "name": "StationBoard",
  "description": "Live departure board with real-time National Rail and London Underground arrivals",
  "brand": {
    "@type": "Brand",
    "name": "StationBoards"
  },
  "offers": {
    "@type": "Offer",
    "price": "89.00",
    "priceCurrency": "GBP"
  }
}
</script>
```

## Support

- Orders: orders@stationboards.co.uk
- Support: support@stationboards.co.uk
- Website issues: Report on GitHub

## License

Copyright © 2024 StationBoards.co.uk
