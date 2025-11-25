// Track Order Client-Side Logic

const form = document.getElementById('orderTrackingForm');
const trackButton = document.getElementById('trackButton');
const buttonText = document.getElementById('buttonText');
const buttonSpinner = document.getElementById('buttonSpinner');
const errorMessage = document.getElementById('errorMessage');
const trackForm = document.getElementById('trackForm');
const orderDetails = document.getElementById('orderDetails');

// Handle form submission
form.addEventListener('submit', async (e) => {
  e.preventDefault();

  const email = document.getElementById('email').value.trim();
  const orderNumber = document.getElementById('orderNumber').value.trim().toUpperCase();

  // Hide previous error
  errorMessage.style.display = 'none';

  // Show loading state
  trackButton.disabled = true;
  buttonText.style.display = 'none';
  buttonSpinner.style.display = 'inline-block';

  try {
    const response = await fetch('/api/track-order', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ email, order_number: orderNumber })
    });

    const data = await response.json();

    if (!response.ok) {
      throw new Error(data.error || 'Failed to track order');
    }

    if (!data.success) {
      throw new Error(data.error || 'Order not found');
    }

    // Display order details
    displayOrderDetails(data.order);

    // Scroll to details
    orderDetails.scrollIntoView({ behavior: 'smooth', block: 'start' });

  } catch (error) {
    console.error('Track order error:', error);
    errorMessage.textContent = error.message;
    errorMessage.style.display = 'block';
  } finally {
    // Reset button state
    trackButton.disabled = false;
    buttonText.style.display = 'inline';
    buttonSpinner.style.display = 'none';
  }
});

// Display order details
function displayOrderDetails(order) {
  // Status badges
  const statusClasses = {
    pending: 'status-pending',
    paid: 'status-paid',
    processing: 'status-processing',
    shipped: 'status-shipped',
    delivered: 'status-delivered',
    cancelled: 'status-cancelled'
  };

  // Format dates
  const formatDate = (dateStr) => {
    if (!dateStr) return 'Not yet';
    return new Date(dateStr).toLocaleString('en-GB', {
      day: 'numeric',
      month: 'long',
      year: 'numeric',
      hour: '2-digit',
      minute: '2-digit'
    });
  };

  // Format last seen time
  const formatLastSeen = (lastSeen) => {
    if (!lastSeen) return 'Never';
    const diff = Date.now() - new Date(lastSeen).getTime();
    if (diff < 60000) return 'Just now';
    if (diff < 3600000) return `${Math.floor(diff / 60000)}m ago`;
    if (diff < 86400000) return `${Math.floor(diff / 3600000)}h ago`;
    return new Date(lastSeen).toLocaleDateString('en-GB');
  };

  // Check if board is online (seen in last 5 minutes)
  const isOnline = (lastSeen) => {
    if (!lastSeen) return false;
    return (Date.now() - new Date(lastSeen).getTime()) < 5 * 60 * 1000;
  };

  // Build HTML
  const html = `
    <div class="order-header">
      <h2>Order ${order.order_number}</h2>
      <span class="status-badge ${statusClasses[order.status] || 'status-pending'}">
        ${order.status}
      </span>
    </div>

    <div class="section">
      <h3>Order Information</h3>
      <div class="info-grid">
        <div class="info-item">
          <label>Order Date</label>
          <div class="value">${formatDate(order.created_at)}</div>
        </div>
        <div class="info-item">
          <label>Quantity</label>
          <div class="value">${order.quantity} board${order.quantity > 1 ? 's' : ''}</div>
        </div>
        <div class="info-item">
          <label>Total Price</label>
          <div class="value">£${parseFloat(order.total_price).toFixed(2)}</div>
        </div>
        <div class="info-item">
          <label>Payment Status</label>
          <div class="value">${order.payment_status === 'paid' ? '✓ Paid' : 'Pending'}</div>
        </div>
      </div>
    </div>

    <div class="section">
      <h3>Delivery Address</h3>
      <div class="info-item">
        <div class="value">
          ${order.shipping_address}<br>
          ${order.shipping_city}, ${order.shipping_postcode}<br>
          ${order.shipping_country || 'United Kingdom'}
        </div>
      </div>
    </div>

    ${order.boards && order.boards.length > 0 ? `
      <div class="section">
        <h3>Your StationBoard${order.boards.length > 1 ? 's' : ''}</h3>
        <div class="boards-grid">
          ${order.boards.map(board => {
            const online = isOnline(board.last_seen);
            return `
              <div class="board-card">
                <div>
                  <div class="board-id">${board.board_id}</div>
                  <div class="board-meta">
                    ${board.status === 'active' ? 'Activated and online' : board.status}
                  </div>
                </div>
                <div class="board-status">
                  <span class="online-indicator ${online ? 'online' : 'offline'}"
                        title="${online ? 'Online' : 'Offline'}">
                    ${online ? '●' : '○'}
                  </span>
                  <div>
                    <div class="board-meta">
                      ${online ? 'Online' : 'Offline'}
                    </div>
                    ${board.last_seen ? `
                      <div class="board-meta">
                        Last seen: ${formatLastSeen(board.last_seen)}
                      </div>
                    ` : ''}
                  </div>
                </div>
              </div>
            `;
          }).join('')}
        </div>
        ${order.boards.some(b => b.status !== 'active') ? `
          <p style="margin-top: 1rem; color: var(--text-light); font-size: 0.9rem;">
            💡 Your board will automatically come online when you power it on and connect to WiFi.
          </p>
        ` : ''}
      </div>
    ` : ''}

    ${order.shipment ? `
      <div class="section">
        <h3>Shipping Information</h3>
        <div class="info-grid">
          <div class="info-item">
            <label>Service</label>
            <div class="value">${order.shipment.service_name || order.shipment.service_code || 'Royal Mail'}</div>
          </div>
          <div class="info-item">
            <label>Shipped On</label>
            <div class="value">${formatDate(order.shipped_at)}</div>
          </div>
          ${order.shipment.tracking_number ? `
            <div class="info-item">
              <label>Tracking Number</label>
              <div class="value">
                <a href="https://www.royalmail.com/track-your-item#/tracking-results/${order.shipment.tracking_number}"
                   target="_blank"
                   class="tracking-link">
                  ${order.shipment.tracking_number} ↗
                </a>
              </div>
            </div>
          ` : ''}
          ${order.shipment.status ? `
            <div class="info-item">
              <label>Delivery Status</label>
              <div class="value">${order.shipment.status.replace(/_/g, ' ')}</div>
            </div>
          ` : ''}
        </div>
        ${order.shipment.tracking_number ? `
          <p style="margin-top: 1rem; color: var(--text-light); font-size: 0.9rem;">
            📦 Click the tracking number to see live delivery updates on Royal Mail's website
          </p>
        ` : ''}
      </div>
    ` : order.status === 'shipped' ? `
      <div class="section">
        <h3>Shipping Information</h3>
        <p style="color: var(--text-light);">
          Your order has been shipped! Tracking information will be available soon.
        </p>
      </div>
    ` : ''}

    ${order.order_history && order.order_history.length > 0 ? `
      <div class="section">
        <h3>Order Timeline</h3>
        <div class="timeline">
          ${order.order_history
            .sort((a, b) => new Date(b.created_at) - new Date(a.created_at))
            .map(event => `
              <div class="timeline-item">
                <div class="timeline-date">${formatDate(event.created_at)}</div>
                <div class="timeline-event">${event.action.replace(/_/g, ' ')}</div>
                ${event.description ? `
                  <div class="timeline-description">${event.description}</div>
                ` : ''}
              </div>
            `).join('')}
        </div>
      </div>
    ` : ''}

    <div class="section" style="text-align: center; margin-top: 3rem;">
      <p style="color: var(--text-light); margin-bottom: 1rem;">
        Questions about your order?
      </p>
      <a href="mailto:hello@stationboards.co.uk?subject=Order ${order.order_number}"
         class="tracking-link">
        Contact Support
      </a>
    </div>
  `;

  orderDetails.innerHTML = html;
  orderDetails.style.display = 'block';
}

// Auto-fill from URL params (if shared link)
window.addEventListener('DOMContentLoaded', () => {
  const params = new URLSearchParams(window.location.search);
  const orderNumber = params.get('order');
  const email = params.get('email');

  if (orderNumber) {
    document.getElementById('orderNumber').value = orderNumber;
  }

  if (email) {
    document.getElementById('email').value = email;
  }

  // Auto-submit if both params present
  if (orderNumber && email) {
    form.dispatchEvent(new Event('submit'));
  }
});
