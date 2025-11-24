// Smooth scrolling for anchor links
document.querySelectorAll('a[href^="#"]').forEach(anchor => {
  anchor.addEventListener('click', function (e) {
    e.preventDefault();
    const target = document.querySelector(this.getAttribute('href'));
    if (target) {
      const offset = 80; // Account for fixed navbar
      const targetPosition = target.offsetTop - offset;
      window.scrollTo({
        top: targetPosition,
        behavior: 'smooth'
      });
    }
  });
});

// Quantity selector pricing update
const quantitySelect = document.getElementById('quantity');
const subtotalEl = document.getElementById('subtotal');
const totalEl = document.getElementById('total');

const pricing = {
  1: 89,
  2: 168,
  3: 240,
  5: 385
};

if (quantitySelect) {
  quantitySelect.addEventListener('change', (e) => {
    const quantity = parseInt(e.target.value);
    const price = pricing[quantity];
    subtotalEl.textContent = `£${price}.00`;
    totalEl.textContent = `£${price}.00`;
  });
}

// Order form submission
const orderForm = document.getElementById('orderForm');
const orderSuccess = document.getElementById('orderSuccess');

if (orderForm) {
  orderForm.addEventListener('submit', async (e) => {
    e.preventDefault();

    // Disable submit button to prevent double submission
    const submitBtn = orderForm.querySelector('button[type="submit"]');
    const originalBtnText = submitBtn.textContent;
    submitBtn.disabled = true;
    submitBtn.textContent = 'Submitting...';

    // Get form data
    const formData = new FormData(orderForm);
    const data = {
      name: formData.get('name'),
      email: formData.get('email'),
      phone: formData.get('phone'),
      address: formData.get('address'),
      city: formData.get('city'),
      postcode: formData.get('postcode'),
      quantity: parseInt(formData.get('quantity')),
      notes: formData.get('notes')
    };

    try {
      // Send order to API
      const response = await fetch('/api/submit-order', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      });

      const result = await response.json();

      if (response.ok) {
        // Show success message
        orderForm.style.display = 'none';
        orderSuccess.style.display = 'block';

        // Update success message with order number
        const successMessage = orderSuccess.querySelector('p');
        if (result.order && result.order.orderNumber) {
          successMessage.innerHTML = `Thank you for your order! Your order number is <strong>${result.order.orderNumber}</strong>. We've sent a confirmation email to ${data.email} with payment instructions.`;
        }

        console.log('Order submitted successfully:', result);
      } else {
        throw new Error(result.error || 'Order submission failed');
      }
    } catch (error) {
      console.error('Error submitting order:', error);

      // Show error message
      alert(`Failed to submit order: ${error.message}\n\nPlease try again or contact us at orders@stationboards.co.uk`);

      // Re-enable submit button
      submitBtn.disabled = false;
      submitBtn.textContent = originalBtnText;

      // Fallback: open email client with order details
      const shouldUseFallback = confirm('Would you like to send your order via email instead?');
      if (shouldUseFallback) {
        const subject = encodeURIComponent('StationBoard Order Request');
        const body = encodeURIComponent(`
Name: ${data.name}
Email: ${data.email}
Phone: ${data.phone}

Shipping Address:
${data.address}
${data.city}, ${data.postcode}

Quantity: ${data.quantity}
Total: ${totalEl.textContent}

Additional Notes:
${data.notes || 'None'}
        `);

        window.location.href = `mailto:orders@stationboards.co.uk?subject=${subject}&body=${body}`;
      }
    }
  });
}

// Reset form function
function resetForm() {
  orderForm.reset();
  orderForm.style.display = 'block';
  orderSuccess.style.display = 'none';
  subtotalEl.textContent = '£89.00';
  totalEl.textContent = '£89.00';

  // Scroll to top of form
  document.getElementById('order').scrollIntoView({ behavior: 'smooth' });
}

// Navbar background on scroll
let lastScroll = 0;
const navbar = document.querySelector('.navbar');

window.addEventListener('scroll', () => {
  const currentScroll = window.pageYOffset;

  if (currentScroll > 100) {
    navbar.style.background = 'rgba(255, 255, 255, 0.98)';
  } else {
    navbar.style.background = 'rgba(255, 255, 255, 0.95)';
  }

  lastScroll = currentScroll;
});

// Animate board preview on scroll
const boardPreview = document.querySelector('.board-screen');

const observer = new IntersectionObserver((entries) => {
  entries.forEach(entry => {
    if (entry.isIntersecting) {
      entry.target.style.animation = 'fadeInUp 0.8s ease-out';
    }
  });
}, {
  threshold: 0.1
});

if (boardPreview) {
  observer.observe(boardPreview);
}

// Add CSS animation
const style = document.createElement('style');
style.textContent = `
  @keyframes fadeInUp {
    from {
      opacity: 0;
      transform: translateY(30px) rotateY(-5deg) rotateX(2deg);
    }
    to {
      opacity: 1;
      transform: translateY(0) rotateY(-5deg) rotateX(2deg);
    }
  }
`;
document.head.appendChild(style);

// Log page view (add analytics tracking here)
console.log('StationBoards website loaded');
console.log('Ready to take orders! 🚉');

// ============= Live OLED Display =============

// Update clock every second
function updateClock() {
  const clockEl = document.getElementById('liveClock');
  if (!clockEl) return;

  const now = new Date();
  const hours = String(now.getHours()).padStart(2, '0');
  const minutes = String(now.getMinutes()).padStart(2, '0');
  const seconds = String(now.getSeconds()).padStart(2, '0');

  clockEl.textContent = `${hours}:${minutes}:${seconds}`;
}

// Start clock
setInterval(updateClock, 1000);
updateClock(); // Initial call

// Fetch live departures from Kings Cross
async function fetchLiveDepartures() {
  const servicesEl = document.getElementById('liveServices');
  if (!servicesEl) return;

  try {
    // Use CORS proxy for demo purposes
    const response = await fetch('https://api.allorigins.win/raw?url=' + encodeURIComponent('http://lite.realtime.nationalrail.co.uk/OpenLDBWS/ldb9.asmx'), {
      method: 'POST',
      headers: {
        'Content-Type': 'text/xml'
      },
      body: `<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope">
  <soap:Header>
    <AccessToken xmlns="http://thalesgroup.com/RTTI/2013-11-28/Token/types">
      <TokenValue>73ee3834-af35-4f22-9b8b-480b70571c39</TokenValue>
    </AccessToken>
  </soap:Header>
  <soap:Body>
    <GetDepBoardWithDetailsRequest xmlns="http://thalesgroup.com/RTTI/2016-02-16/ldb/">
      <numRows>2</numRows>
      <crs>KGX</crs>
    </GetDepBoardWithDetailsRequest>
  </soap:Body>
</soap:Envelope>`
    });

    const xmlText = await response.text();
    const parser = new DOMParser();
    const xmlDoc = parser.parseFromString(xmlText, 'text/xml');

    // Extract services
    const services = xmlDoc.getElementsByTagName('lt5:service');

    if (services.length === 0) {
      servicesEl.innerHTML = '<div class="oled-loading">No departures available</div>';
      return;
    }

    let html = '';

    for (let i = 0; i < Math.min(2, services.length); i++) {
      const service = services[i];

      const std = service.getElementsByTagName('lt4:std')[0]?.textContent ||
                  service.getElementsByTagName('lt5:std')[0]?.textContent || '';
      const etd = service.getElementsByTagName('lt4:etd')[0]?.textContent ||
                  service.getElementsByTagName('lt5:etd')[0]?.textContent || '';
      const destName = service.getElementsByTagName('lt4:locationName')[0]?.textContent ||
                       service.getElementsByTagName('lt5:locationName')[0]?.textContent || '';

      // Get calling points
      const cpList = service.getElementsByTagName('lt5:callingPoint');
      let callingAt = [];
      for (let j = 0; j < cpList.length; j++) {
        const cpName = cpList[j].getElementsByTagName('lt4:locationName')[0]?.textContent ||
                       cpList[j].getElementsByTagName('lt5:locationName')[0]?.textContent;
        if (cpName) callingAt.push(cpName);
      }

      const label = i === 0 ? '1st' : '2nd';

      html += `
        <div class="oled-service">
          <div class="oled-service-header">
            <div>
              <div class="oled-label">${label}</div>
              <div class="oled-time">${std}</div>
            </div>
            <div class="oled-etd">${etd}</div>
          </div>
          <div class="oled-destination">${destName}</div>
          ${callingAt.length > 0 ? `
            <div class="oled-calling">
              <span class="oled-calling-text">Calling at: ${callingAt.join(', ')}</span>
            </div>
          ` : ''}
        </div>
      `;
    }

    servicesEl.innerHTML = html;

  } catch (error) {
    console.error('Failed to fetch live departures:', error);
    servicesEl.innerHTML = '<div class="oled-loading">Unable to load live data</div>';
  }
}

// Fetch departures on load and refresh every 60 seconds
fetchLiveDepartures();
setInterval(fetchLiveDepartures, 60000);
