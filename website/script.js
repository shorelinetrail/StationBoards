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

    // Get form data
    const formData = new FormData(orderForm);
    const data = {
      name: formData.get('name'),
      email: formData.get('email'),
      phone: formData.get('phone'),
      address: formData.get('address'),
      city: formData.get('city'),
      postcode: formData.get('postcode'),
      quantity: formData.get('quantity'),
      notes: formData.get('notes'),
      timestamp: new Date().toISOString(),
      total: totalEl.textContent
    };

    try {
      // Send order to backend (you'll need to implement this endpoint)
      const response = await fetch('/api/orders', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      });

      if (response.ok) {
        // Show success message
        orderForm.style.display = 'none';
        orderSuccess.style.display = 'block';

        // Send email notification (implement on backend)
        console.log('Order submitted:', data);
      } else {
        throw new Error('Order submission failed');
      }
    } catch (error) {
      console.error('Error submitting order:', error);

      // Fallback: open email client with order details
      const subject = encodeURIComponent('StationBoard Order Request');
      const body = encodeURIComponent(`
Name: ${data.name}
Email: ${data.email}
Phone: ${data.phone}

Shipping Address:
${data.address}
${data.city}, ${data.postcode}

Quantity: ${data.quantity}
Total: ${data.total}

Additional Notes:
${data.notes || 'None'}
      `);

      window.location.href = `mailto:orders@stationboards.co.uk?subject=${subject}&body=${body}`;

      // Show success message anyway
      orderForm.style.display = 'none';
      orderSuccess.style.display = 'block';
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
async function fetchLiveDepartures() {
  const stationCode = 'KGX'; // Kings Cross
  const liveServices = document.getElementById('liveServices');

  try {
    // Fetch from National Rail API
    const response = await fetch(`https://huxley2.azurewebsites.net/departures/${stationCode}/4`);

    if (!response.ok) {
      throw new Error('Failed to fetch departures');
    }

    const data = await response.json();
    const services = data.trainServices || [];

    if (services.length === 0) {
      liveServices.innerHTML = '<div class="loading-spinner"><p>No departures available</p></div>';
      return;
    }

    // Display services
    liveServices.innerHTML = services.slice(0, 4).map(service => {
      const time = service.std || '--:--';
      const destination = service.destination?.[0]?.locationName || 'Unknown';
      const platform = service.platform ? `Platform ${service.platform}` : 'Platform TBC';

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
        <div class="service-row">
          <span class="time">${time}</span>
          <span class="destination">${destination}</span>
          <span class="platform">${platform}</span>
          <span class="status ${statusClass}">${statusText}</span>
        </div>
      `;
    }).join('');

    console.log(`✓ Loaded ${services.length} live departures from ${stationCode}`);

  } catch (error) {
    console.error('Error fetching live departures:', error);
    liveServices.innerHTML = `
      <div class="loading-spinner">
        <p>Unable to load live data</p>
        <p style="font-size: 0.8rem; opacity: 0.7;">Showing example departures</p>
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
    background: #f8d7da;
    color: #721c24;
  }
`;
document.head.appendChild(cancelledStyle);

// Fetch live data on page load
document.addEventListener('DOMContentLoaded', () => {
  fetchLiveDepartures();

  // Refresh every 60 seconds
  setInterval(fetchLiveDepartures, 60000);
});

// Log page view (add analytics tracking here)
console.log('StationBoards website loaded');
console.log('Ready to take orders! 🚉');
