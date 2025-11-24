// ===== SHIPPING MANAGEMENT =====

let shipments = [];
let readyToShipOrders = [];

// Load Shipping Data
async function loadShipping() {
  try {
    // Load all shipments
    const { data: shipmentsData, error: shipmentsError } = await supabase
      .from('shipments')
      .select(`
        *,
        orders (
          id,
          order_number,
          customer_name,
          customer_email,
          shipping_address,
          shipping_city,
          shipping_postcode
        )
      `)
      .order('created_at', { ascending: false });

    if (shipmentsError) throw shipmentsError;
    shipments = shipmentsData || [];

    // Load orders ready to ship (paid orders with boards but no shipment)
    const { data: readyOrders, error: ordersError } = await supabase
      .from('orders')
      .select(`
        *,
        boards (board_id),
        shipments (id)
      `)
      .eq('payment_status', 'paid')
      .order('created_at', { ascending: false });

    if (ordersError) throw ordersError;

    // Filter orders that have boards assigned but no shipment
    readyToShipOrders = (readyOrders || []).filter(order => {
      const hasBoards = order.boards && order.boards.length > 0;
      const hasShipment = order.shipments && order.shipments.length > 0;
      return hasBoards && !hasShipment;
    });

    // Update stats
    updateShippingStats();
    displayShipments();
    displayReadyToShip();

  } catch (error) {
    console.error('Error loading shipping:', error);
  }
}

// Update Shipping Stats
function updateShippingStats() {
  const today = new Date();
  today.setHours(0, 0, 0, 0);

  const pending = readyToShipOrders.length;
  const labelsToday = shipments.filter(s => new Date(s.created_at) >= today).length;
  const inTransit = shipments.filter(s => s.status === 'in_transit' || s.status === 'collected').length;
  const deliveredToday = shipments.filter(s =>
    s.status === 'delivered' && s.delivered_at && new Date(s.delivered_at) >= today
  ).length;

  document.getElementById('pendingShipmentsCount').textContent = pending;
  document.getElementById('labelsTodayCount').textContent = labelsToday;
  document.getElementById('inTransitCount').textContent = inTransit;
  document.getElementById('deliveredTodayCount').textContent = deliveredToday;
}

// Display Shipments Table
function displayShipments() {
  const container = document.getElementById('shipmentsTable');
  const filter = document.getElementById('shipmentStatusFilter').value;

  let filtered = shipments;
  if (filter) {
    filtered = shipments.filter(s => s.status === filter);
  }

  if (filtered.length === 0) {
    container.innerHTML = '<p class="loading">No shipments found</p>';
    return;
  }

  const formatStatus = (status) => {
    return status ? status.replace(/_/g, ' ').replace(/\b\w/g, l => l.toUpperCase()) : 'Unknown';
  };

  container.innerHTML = `
    <div class="shipment-header">
      <div>Order</div>
      <div>Customer</div>
      <div>Tracking</div>
      <div>Service</div>
      <div>Status</div>
      <div>Actions</div>
    </div>
    ${filtered.map(shipment => `
      <div class="shipment-row">
        <div>
          <strong>${shipment.orders?.order_number || 'N/A'}</strong>
        </div>
        <div>
          ${shipment.orders?.customer_name || 'N/A'}<br>
          <small style="color: var(--text-light);">${shipment.orders?.shipping_city || ''}</small>
        </div>
        <div>
          ${shipment.tracking_number ? `
            <a href="https://www.royalmail.com/track-your-item#/tracking-results/${shipment.tracking_number}"
               target="_blank"
               class="tracking-link">
              ${shipment.tracking_number}
            </a>
          ` : 'No tracking'}
        </div>
        <div>
          <small>${shipment.service_name || shipment.service_code || 'N/A'}</small>
        </div>
        <div>
          <span class="status-badge status-${shipment.status}">
            ${formatStatus(shipment.status)}
          </span>
        </div>
        <div class="shipment-actions">
          ${shipment.label_url ? `
            <button class="btn-icon" onclick="window.open('${shipment.label_url}', '_blank')" title="Download Label">
              📄
            </button>
          ` : ''}
          <button class="btn-icon" onclick="refreshTrackingStatus('${shipment.id}')" title="Refresh Tracking">
            🔄
          </button>
        </div>
      </div>
    `).join('')}
  `;
}

// Display Ready to Ship Orders
function displayReadyToShip() {
  const container = document.getElementById('readyToShipTable');

  if (readyToShipOrders.length === 0) {
    container.innerHTML = '<p class="loading">No orders ready to ship</p>';
    return;
  }

  container.innerHTML = `
    <div class="shipment-header">
      <div>Order</div>
      <div>Customer & Address</div>
      <div>Boards</div>
      <div>Payment</div>
      <div>Actions</div>
    </div>
    ${readyToShipOrders.map(order => {
      const boardList = order.boards.map(b => b.board_id).join(', ');
      return `
      <div class="ready-to-ship-row">
        <div>
          <strong>${order.order_number}</strong><br>
          <small style="color: var(--text-light);">${new Date(order.created_at).toLocaleDateString()}</small>
        </div>
        <div>
          <strong>${order.customer_name}</strong><br>
          <small style="color: var(--text-light);">
            ${order.shipping_address}<br>
            ${order.shipping_city}, ${order.shipping_postcode}
          </small>
        </div>
        <div>
          ${order.quantity} board${order.quantity > 1 ? 's' : ''}<br>
          <small style="color: var(--text-light);" title="${boardList}">${order.boards.length} assigned</small>
        </div>
        <div>
          <span class="status-badge status-paid">Paid</span><br>
          <small>£${parseFloat(order.total_price).toFixed(2)}</small>
        </div>
        <div>
          <button class="btn-primary" onclick="createShippingLabel('${order.id}')">
            Create Shipping Label
          </button>
        </div>
      </div>
    `;
    }).join('')}
  `;
}

// Test Royal Mail API Connection
async function testRoyalMailConnection() {
  const statusDot = document.getElementById('rmStatusDot');
  const statusText = document.getElementById('rmStatusText');
  const testBtn = document.getElementById('testRoyalMailBtn');

  statusText.textContent = 'Testing connection...';
  statusDot.className = 'status-dot';
  testBtn.disabled = true;

  try {
    const response = await fetch('/api/test-royal-mail', {
      method: 'POST'
    });

    const data = await response.json();

    if (data.success) {
      statusDot.className = 'status-dot connected';
      statusText.textContent = '✓ Royal Mail API Connected';
    } else {
      throw new Error(data.error || 'Connection failed');
    }
  } catch (error) {
    statusDot.className = 'status-dot disconnected';
    statusText.textContent = '✗ API credentials not configured';
    console.log('Royal Mail API not configured:', error.message);
  } finally {
    testBtn.disabled = false;
  }
}

// Create Shipping Label (uses existing createShippingLabel from admin-script.js)
// This function opens the manual shipping modal
window.createShippingLabel = createShippingLabel;

// Refresh Tracking Status
async function refreshTrackingStatus(shipmentId) {
  try {
    const response = await fetch('/api/refresh-tracking', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ shipment_id: shipmentId })
    });

    const data = await response.json();

    if (data.success) {
      alert('Tracking status updated');
      loadShipping();
    } else {
      throw new Error(data.error);
    }
  } catch (error) {
    alert('Failed to refresh tracking: ' + error.message);
  }
}

// Create Manifest
async function createManifest() {
  if (!confirm('Create manifest for all unmanifested labels created today?')) {
    return;
  }

  const btn = document.getElementById('createManifestBtn');
  btn.disabled = true;
  btn.textContent = 'Creating...';

  try {
    const response = await fetch('/api/create-manifest', {
      method: 'POST'
    });

    const data = await response.json();

    if (data.success) {
      alert('Manifest created: ' + data.manifest_id + '\n' + data.shipment_count + ' shipments included');
      if (data.manifest_url) {
        window.open(data.manifest_url, '_blank');
      }
      loadShipping();
    } else {
      throw new Error(data.error);
    }
  } catch (error) {
    alert('Failed to create manifest: ' + error.message);
  } finally {
    btn.disabled = false;
    btn.textContent = '📋 Create Manifest';
  }
}

// Set up shipping event listeners
document.getElementById('shipmentStatusFilter')?.addEventListener('change', displayShipments);
document.getElementById('testRoyalMailBtn')?.addEventListener('click', testRoyalMailConnection);
document.getElementById('createManifestBtn')?.addEventListener('click', createManifest);

// Export functions
window.loadShipping = loadShipping;
window.refreshTrackingStatus = refreshTrackingStatus;
window.createManifest = createManifest;
window.testRoyalMailConnection = testRoyalMailConnection;
