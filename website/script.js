// Mobile Menu Toggle
function toggleMobileMenu() {
  const menu = document.getElementById('mobileMenu');
  const icon = document.getElementById('menuIcon');
  menu.classList.toggle('mobile-open');
  icon.textContent = menu.classList.contains('mobile-open') ? '✕' : '☰';
}

function closeMobileMenu() {
  const menu = document.getElementById('mobileMenu');
  const icon = document.getElementById('menuIcon');
  menu.classList.remove('mobile-open');
  icon.textContent = '☰';
}

// Close mobile menu when clicking outside
document.addEventListener('click', function(event) {
  const menu = document.getElementById('mobileMenu');
  const toggle = document.querySelector('.mobile-menu-toggle');

  if (menu && toggle && !menu.contains(event.target) && !toggle.contains(event.target)) {
    closeMobileMenu();
  }
});

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

// Live board preview with National Rail API
async function fetchLiveServices() {
  const stationCode = 'KGX'; // Kings Cross
  const liveServices = document.getElementById('liveServices');

  try {
    // Fetch from National Rail API
    const response = await fetch(`https://huxley2.azurewebsites.net/departures/${stationCode}/4`);

    if (!response.ok) {
      throw new Error('Failed to fetch train services');
    }

    const data = await response.json();
    const services = data.trainServices || [];

    if (services.length === 0) {
      liveServices.innerHTML = '<div class="loading-spinner"><p>No services available</p></div>';
      return;
    }

    // Display services
    liveServices.innerHTML = services.slice(0, 4).map(service => {
      const time = service.std || '--:--';
      const destination = service.destination?.[0]?.locationName || 'Unknown';

      // Determine status
      let statusClass = 'status-ontime';
      let statusText = 'On time';

      if (service.isCancelled) {
        statusClass = 'status-cancelled';
        statusText = 'Cancelled';
      } else if (service.etd && service.etd !== 'On time') {
        statusClass = 'status-delay';
        statusText = service.etd;
      }

      return `
        <div class="service-row service-row-no-platform">
          <span class="time">${time}</span>
          <span class="destination">${destination}</span>
          <span class="status ${statusClass}">${statusText}</span>
        </div>
      `;
    }).join('');

    console.log(`✓ Loaded ${services.length} live services from ${stationCode}`);

  } catch (error) {
    console.error('Error fetching live services:', error);
    liveServices.innerHTML = `
      <div class="loading-spinner">
        <p>Unable to load live data</p>
        <p style="font-size: 0.8rem; opacity: 0.7;">Showing example services</p>
      </div>
    `;

    // Fallback to static example
    setTimeout(() => {
      liveServices.innerHTML = `
        <div class="service-row">
          <span class="time">14:35</span>
          <span class="destination">Edinburgh</span>
          <span class="platform">Platform 0</span>
          <span class="status status-ontime">On time</span>
        </div>
        <div class="service-row">
          <span class="time">14:42</span>
          <span class="destination">Cambridge</span>
          <span class="platform">Platform 2</span>
          <span class="status status-ontime">On time</span>
        </div>
        <div class="service-row">
          <span class="time">14:50</span>
          <span class="destination">Leeds</span>
          <span class="platform">Platform 7</span>
          <span class="status status-delay">Delayed 5 mins</span>
        </div>
        <div class="service-row">
          <span class="time">15:00</span>
          <span class="destination">Peterborough</span>
          <span class="platform">Platform 4</span>
          <span class="status status-ontime">On time</span>
        </div>
      `;
    }, 2000);
  }
}

// Add cancelled status style dynamically
const cancelledStyle = document.createElement('style');
cancelledStyle.textContent = `
  .status-cancelled {
    color: #ff4444;
    border-color: #ff4444;
  }
`;
document.head.appendChild(cancelledStyle);

// Fetch live data on page load
document.addEventListener('DOMContentLoaded', () => {
  fetchLiveServices();

  // Refresh every 60 seconds
  setInterval(fetchLiveServices, 60000);
});

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

// Display realistic demo departures from Kings Cross
function showDemoDepartures() {
  const servicesEl = document.getElementById('liveServices');
  if (!servicesEl) return;

  // Calculate realistic departure times based on current time
  const now = new Date();
  const currentMinutes = now.getHours() * 60 + now.getMinutes();

  // First train: next departure in 5-15 minutes
  const mins1 = Math.floor((currentMinutes % 30) / 2) + 5;
  const time1 = new Date(now.getTime() + mins1 * 60000);
  const std1 = `${String(time1.getHours()).padStart(2, '0')}:${String(time1.getMinutes()).padStart(2, '0')}`;

  // Second train: 15-25 minutes after first
  const mins2 = mins1 + 15 + Math.floor((currentMinutes % 20) / 2);
  const time2 = new Date(now.getTime() + mins2 * 60000);
  const std2 = `${String(time2.getHours()).padStart(2, '0')}:${String(time2.getMinutes()).padStart(2, '0')}`;

  // Rotate through different destinations with realistic journey times (minutes from Kings Cross)
  const destinations = [
    {
      name: 'Edinburgh',
      calling: ['Peterborough', 'York', 'Darlington', 'Newcastle'],
      times: [50, 100, 145, 175] // minutes from departure
    },
    {
      name: 'Leeds',
      calling: ['Stevenage', 'Peterborough', 'Doncaster', 'Wakefield'],
      times: [25, 50, 95, 115]
    },
    {
      name: 'Cambridge',
      calling: ['Finsbury Park', 'Stevenage', 'Hitchin', 'Royston'],
      times: [7, 25, 35, 45]
    },
    {
      name: 'Newcastle',
      calling: ['Peterborough', 'York', 'Darlington', 'Durham'],
      times: [50, 100, 145, 165]
    }
  ];

  const hour = now.getHours();
  const dest1 = destinations[hour % destinations.length];
  const dest2 = destinations[(hour + 1) % destinations.length];

  // ETD varies slightly
  const etd1 = mins1 <= 1 ? 'On time' : mins1 <= 5 ? `${mins1} min` : 'On time';
  const etd2 = 'Delayed'; // Always show Delayed for second train

  // Generate calling point times using realistic journey times
  function generateCallingTimes(departureTime, destination) {
    return destination.times.map(mins => {
      const arrivalTime = new Date(departureTime.getTime() + mins * 60000);
      const hh = String(arrivalTime.getHours()).padStart(2, '0');
      const mm = String(arrivalTime.getMinutes()).padStart(2, '0');
      return `${hh}:${mm}`;
    });
  }

  const callingTimes1 = generateCallingTimes(time1, dest1);
  const callingText1 = dest1.calling.map((station, i) => `${station} (${callingTimes1[i]})`).join(', ');

  const html = `
    <div class="oled-service">
      <div class="oled-service-line">
        <span>1st  ${std1}  ${dest1.name}</span>
        <span>${etd1}</span>
      </div>
      <div class="oled-calling">
        <span class="oled-calling-label">Calling at:</span>
        <div class="oled-calling-scroll-container">
          <span class="oled-calling-scroll">${callingText1}</span>
        </div>
      </div>
    </div>
    <div class="oled-service">
      <div class="oled-service-line">
        <span>2nd  ${std2}  ${dest2.name}</span>
        <span>${etd2}</span>
      </div>
    </div>
  `;

  servicesEl.innerHTML = html;
}

// Show demo departures on load and refresh every 30 seconds to update times
showDemoDepartures();
setInterval(showDemoDepartures, 30000);

// ============= Payment Error Handling =============

// Check for payment-related URL parameters (from PayPal/Stripe redirects)
function checkPaymentStatus() {
  const urlParams = new URLSearchParams(window.location.search);
  const error = urlParams.get('error');
  const message = urlParams.get('message');
  const orderNumber = urlParams.get('order');
  const paymentCancelled = urlParams.get('payment_cancelled');

  if (paymentCancelled) {
    showPaymentMessage(
      '⚠️ Payment Cancelled',
      `Your PayPal payment for order ${paymentCancelled} was cancelled. You can try again from the checkout page.`,
      'warning'
    );
    return;
  }

  if (error === 'payment_failed') {
    showPaymentMessage(
      '❌ Payment Failed',
      message ? `Payment failed: ${message}` : 'Your payment could not be processed. Please try again or use a different payment method.',
      'error'
    );
  } else if (error === 'payment_processing') {
    showPaymentMessage(
      '⚠️ Payment Processing Issue',
      `There was an issue processing your payment${orderNumber ? ` for order ${orderNumber}` : ''}. ${message || 'Please contact support.'}`,
      'warning'
    );
  } else if (error === 'missing_parameters') {
    showPaymentMessage(
      '❌ Invalid Payment Link',
      'This payment link is invalid or incomplete. Please start a new checkout.',
      'error'
    );
  }
}

// Display payment message banner
function showPaymentMessage(title, message, type = 'info') {
  // Create message banner
  const banner = document.createElement('div');
  banner.className = `payment-message payment-message-${type}`;
  banner.innerHTML = `
    <div class="payment-message-content">
      <div class="payment-message-title">${title}</div>
      <div class="payment-message-text">${message}</div>
      <button onclick="this.parentElement.parentElement.remove()" class="payment-message-close">✕</button>
    </div>
  `;

  // Add styles if not already added
  if (!document.getElementById('payment-message-styles')) {
    const style = document.createElement('style');
    style.id = 'payment-message-styles';
    style.textContent = `
      .payment-message {
        position: fixed;
        top: 80px;
        left: 50%;
        transform: translateX(-50%);
        max-width: 600px;
        width: 90%;
        padding: 1.5rem;
        border-radius: 12px;
        box-shadow: 0 10px 40px rgba(0,0,0,0.2);
        z-index: 1000;
        animation: slideDown 0.4s ease;
      }

      @keyframes slideDown {
        from {
          opacity: 0;
          transform: translateX(-50%) translateY(-20px);
        }
        to {
          opacity: 1;
          transform: translateX(-50%) translateY(0);
        }
      }

      .payment-message-error {
        background: #fee;
        border-left: 4px solid #e53e3e;
      }

      .payment-message-warning {
        background: #ffc;
        border-left: 4px solid #dd6b20;
      }

      .payment-message-info {
        background: #e6f7ff;
        border-left: 4px solid #3182ce;
      }

      .payment-message-content {
        position: relative;
        padding-right: 2rem;
      }

      .payment-message-title {
        font-size: 1.1rem;
        font-weight: 600;
        margin-bottom: 0.5rem;
        color: #1a202c;
      }

      .payment-message-text {
        color: #4a5568;
        line-height: 1.6;
      }

      .payment-message-close {
        position: absolute;
        top: 0;
        right: 0;
        background: none;
        border: none;
        font-size: 1.5rem;
        cursor: pointer;
        color: #718096;
        padding: 0;
        width: 30px;
        height: 30px;
        line-height: 1;
      }

      .payment-message-close:hover {
        color: #1a202c;
      }
    `;
    document.head.appendChild(style);
  }

  // Insert banner at top of page
  document.body.insertBefore(banner, document.body.firstChild);

  // Auto-dismiss after 10 seconds
  setTimeout(() => {
    if (banner.parentElement) {
      banner.style.animation = 'slideDown 0.3s ease reverse';
      setTimeout(() => banner.remove(), 300);
    }
  }, 10000);
}

// Check for payment status on page load
checkPaymentStatus();
