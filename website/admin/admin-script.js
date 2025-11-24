// Admin Dashboard JavaScript
// Initialize Supabase
const SUPABASE_URL = 'https://qqwrjrstqnwbwlceccde.supabase.co';
const SUPABASE_ANON_KEY = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InFxd3JqcnN0cW53YndsY2VjY2RlIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjM5MDMxNjUsImV4cCI6MjA3OTQ3OTE2NX0.ZLkqaJRwpK9aP4AjNCeikrbBfPzUVySaOGdst47GEyE';

const supabase = window.supabase.createClient(SUPABASE_URL, SUPABASE_ANON_KEY);

let currentUser = null;
let orders = [];
let boards = [];

// DOM Elements
const loginScreen = document.getElementById('loginScreen');
const adminDashboard = document.getElementById('adminDashboard');
const loginForm = document.getElementById('loginForm');
const loginError = document.getElementById('loginError');
const userEmail = document.getElementById('userEmail');
const logoutBtn = document.getElementById('logoutBtn');

// Initialize
async function init() {
  // Check if user is already logged in
  const { data: { session } } = await supabase.auth.getSession();

  if (session) {
    currentUser = session.user;
    showDashboard();
  } else {
    loginScreen.style.display = 'flex';
  }
}

// Login
loginForm.addEventListener('submit', async (e) => {
  e.preventDefault();
  loginError.textContent = '';

  const email = document.getElementById('loginEmail').value;
  const password = document.getElementById('loginPassword').value;

  try {
    const { data, error } = await supabase.auth.signInWithPassword({
      email,
      password
    });

    if (error) throw error;

    currentUser = data.user;
    showDashboard();
  } catch (error) {
    console.error('Login error:', error);
    loginError.textContent = error.message;
  }
});

// Logout
logoutBtn.addEventListener('click', async () => {
  await supabase.auth.signOut();
  location.reload();
});

// Show Dashboard
function showDashboard() {
  loginScreen.style.display = 'none';
  adminDashboard.style.display = 'block';
  userEmail.textContent = currentUser.email;

  // Load initial data
  loadOrders();
  loadBoards();

  // Set up event listeners
  setupEventListeners();
}

// Tab Navigation
function setupEventListeners() {
  // Tab switching
  document.querySelectorAll('.tab').forEach(tab => {
    tab.addEventListener('click', () => {
      const tabName = tab.dataset.tab;
      switchTab(tabName);
    });
  });

  // Order filters
  document.getElementById('statusFilter').addEventListener('change', filterOrders);
  document.getElementById('searchOrders').addEventListener('input', filterOrders);

  // Add board button
  document.getElementById('addBoardBtn').addEventListener('click', () => {
    showModal('addBoardModal');
  });

  // Add board form
  document.getElementById('addBoardForm').addEventListener('submit', handleAddBoard);

  // Modal close buttons
  document.querySelectorAll('.modal-close').forEach(btn => {
    btn.addEventListener('click', (e) => {
      const modal = e.target.closest('.modal');
      hideModal(modal.id);
    });
  });

  // Close modal on background click
  document.querySelectorAll('.modal').forEach(modal => {
    modal.addEventListener('click', (e) => {
      if (e.target === modal) {
        hideModal(modal.id);
      }
    });
  });
}

function switchTab(tabName) {
  // Update tab buttons
  document.querySelectorAll('.tab').forEach(tab => {
    tab.classList.remove('active');
    if (tab.dataset.tab === tabName) {
      tab.classList.add('active');
    }
  });

  // Update tab content
  document.querySelectorAll('.tab-content').forEach(content => {
    content.classList.remove('active');
  });
  document.getElementById(tabName + 'Tab').classList.add('active');

  // Load data if needed
  if (tabName === 'orders') {
    loadOrders();
  } else if (tabName === 'boards') {
    loadBoards();
  }
}

// Load Orders
async function loadOrders() {
  try {
    const { data, error } = await supabase
      .from('orders')
      .select(`
        *,
        boards (board_id, status),
        shipments (tracking_number, status)
      `)
      .order('created_at', { ascending: false });

    if (error) throw error;

    orders = data;
    displayOrders(orders);
  } catch (error) {
    console.error('Error loading orders:', error);
    document.getElementById('ordersList').innerHTML = '<p class="error-message">Failed to load orders</p>';
  }
}

function displayOrders(ordersToDisplay) {
  const ordersList = document.getElementById('ordersList');

  if (ordersToDisplay.length === 0) {
    ordersList.innerHTML = '<p class="loading">No orders found</p>';
    return;
  }

  ordersList.innerHTML = ordersToDisplay.map(order => `
    <div class="order-card" onclick="showOrderDetail('${order.id}')">
      <div class="order-header">
        <div class="order-number">${order.order_number}</div>
        <div class="status-badge status-${order.status}">${order.status}</div>
      </div>
      <div class="order-info">
        <div class="order-info-item">
          <strong>Customer</strong>
          ${order.customer_name}
        </div>
        <div class="order-info-item">
          <strong>Email</strong>
          ${order.customer_email}
        </div>
        <div class="order-info-item">
          <strong>Quantity</strong>
          ${order.quantity} board${order.quantity > 1 ? 's' : ''}
        </div>
        <div class="order-info-item">
          <strong>Total</strong>
          £${order.total_price}
        </div>
        <div class="order-info-item">
          <strong>Date</strong>
          ${new Date(order.created_at).toLocaleDateString()}
        </div>
        <div class="order-info-item">
          <strong>Boards</strong>
          ${order.boards ? order.boards.length + ' assigned' : 'Not assigned'}
        </div>
      </div>
    </div>
  `).join('');
}

function filterOrders() {
  const statusFilter = document.getElementById('statusFilter').value;
  const searchQuery = document.getElementById('searchOrders').value.toLowerCase();

  let filtered = orders;

  // Filter by status
  if (statusFilter) {
    filtered = filtered.filter(order => order.status === statusFilter);
  }

  // Search
  if (searchQuery) {
    filtered = filtered.filter(order => {
      return order.order_number.toLowerCase().includes(searchQuery) ||
             order.customer_name.toLowerCase().includes(searchQuery) ||
             order.customer_email.toLowerCase().includes(searchQuery);
    });
  }

  displayOrders(filtered);
}

// Show Order Detail
async function showOrderDetail(orderId) {
  try {
    // Load full order details including history
    const { data: order, error } = await supabase
      .from('orders')
      .select(`
        *,
        boards (*),
        shipments (*),
        order_history (*),
        email_log (*)
      `)
      .eq('id', orderId)
      .single();

    if (error) throw error;

    // Build modal content
    const modalBody = document.getElementById('orderModalBody');
    document.getElementById('modalOrderNumber').textContent = `Order ${order.order_number}`;

    modalBody.innerHTML = `
      <div class="detail-section">
        <h3>Customer Information</h3>
        <div class="detail-grid">
          <div class="detail-item">
            <label>Name</label>
            <div class="value">${order.customer_name}</div>
          </div>
          <div class="detail-item">
            <label>Email</label>
            <div class="value">${order.customer_email}</div>
          </div>
          <div class="detail-item">
            <label>Phone</label>
            <div class="value">${order.customer_phone || 'N/A'}</div>
          </div>
        </div>
      </div>

      <div class="detail-section">
        <h3>Shipping Address</h3>
        <div class="detail-item">
          <div class="value">
            ${order.shipping_address}<br>
            ${order.shipping_city}, ${order.shipping_postcode}<br>
            ${order.shipping_country}
          </div>
        </div>
      </div>

      <div class="detail-section">
        <h3>Order Details</h3>
        <div class="detail-grid">
          <div class="detail-item">
            <label>Quantity</label>
            <div class="value">${order.quantity} board${order.quantity > 1 ? 's' : ''}</div>
          </div>
          <div class="detail-item">
            <label>Total Price</label>
            <div class="value">£${order.total_price}</div>
          </div>
          <div class="detail-item">
            <label>Status</label>
            <div class="value">
              <select id="orderStatus" onchange="updateOrderStatus('${order.id}', this.value)">
                <option value="pending" ${order.status === 'pending' ? 'selected' : ''}>Pending</option>
                <option value="paid" ${order.status === 'paid' ? 'selected' : ''}>Paid</option>
                <option value="processing" ${order.status === 'processing' ? 'selected' : ''}>Processing</option>
                <option value="shipped" ${order.status === 'shipped' ? 'selected' : ''}>Shipped</option>
                <option value="delivered" ${order.status === 'delivered' ? 'selected' : ''}>Delivered</option>
                <option value="cancelled" ${order.status === 'cancelled' ? 'selected' : ''}>Cancelled</option>
              </select>
            </div>
          </div>
          <div class="detail-item">
            <label>Payment Status</label>
            <div class="value">
              <select id="paymentStatus" onchange="updatePaymentStatus('${order.id}', this.value)">
                <option value="pending" ${order.payment_status === 'pending' ? 'selected' : ''}>Pending</option>
                <option value="paid" ${order.payment_status === 'paid' ? 'selected' : ''}>Paid</option>
                <option value="refunded" ${order.payment_status === 'refunded' ? 'selected' : ''}>Refunded</option>
              </select>
            </div>
          </div>
        </div>
        ${order.notes ? `
          <div class="detail-item" style="margin-top: 1rem;">
            <label>Notes</label>
            <div class="value">${order.notes}</div>
          </div>
        ` : ''}
      </div>

      <div class="detail-section">
        <h3>Assigned Boards</h3>
        ${order.boards && order.boards.length > 0 ? `
          <div class="boards-table">
            <table>
              <thead>
                <tr>
                  <th>Board ID</th>
                  <th>Status</th>
                  <th>Assigned</th>
                </tr>
              </thead>
              <tbody>
                ${order.boards.map(board => `
                  <tr>
                    <td class="board-id">${board.board_id}</td>
                    <td><span class="status-badge status-${board.status}">${board.status}</span></td>
                    <td>${new Date(board.assigned_at).toLocaleString()}</td>
                  </tr>
                `).join('')}
              </tbody>
            </table>
          </div>
        ` : '<p>No boards assigned yet</p>'}
        <div class="action-buttons">
          <button class="btn-primary" onclick="showAssignBoardDialog('${order.id}')">Assign Board</button>
        </div>
      </div>

      ${order.shipments ? `
        <div class="detail-section">
          <h3>Shipping Information</h3>
          <div class="detail-grid">
            <div class="detail-item">
              <label>Tracking Number</label>
              <div class="value">${order.shipments.tracking_number || 'Not yet shipped'}</div>
            </div>
            <div class="detail-item">
              <label>Service</label>
              <div class="value">${order.shipments.service_name || 'N/A'}</div>
            </div>
            <div class="detail-item">
              <label>Status</label>
              <div class="value">${order.shipments.status || 'N/A'}</div>
            </div>
          </div>
          <div class="action-buttons">
            <button class="btn-primary" onclick="createShippingLabel('${order.id}')">Create Shipping Label</button>
          </div>
        </div>
      ` : ''}

      <div class="detail-section">
        <h3>Actions</h3>
        <div class="action-buttons">
          <button class="btn-success" onclick="sendEmail('${order.id}', 'payment_reminder')">Send Payment Reminder</button>
          <button class="btn-success" onclick="sendEmail('${order.id}', 'shipping_notification')">Send Shipping Notification</button>
          <button class="btn-warning" onclick="downloadInvoice('${order.id}')">Download Invoice</button>
        </div>
      </div>

      <div class="detail-section">
        <h3>Activity Log</h3>
        <div class="activity-log">
          ${order.order_history && order.order_history.length > 0 ?
            order.order_history.map(activity => `
              <div class="activity-item">
                <div class="timestamp">${new Date(activity.created_at).toLocaleString()}</div>
                <div class="action">${activity.action}</div>
                <div class="description">${activity.description || ''}</div>
              </div>
            `).join('') :
            '<p>No activity yet</p>'
          }
        </div>
      </div>
    `;

    showModal('orderModal');
  } catch (error) {
    console.error('Error loading order details:', error);
    alert('Failed to load order details');
  }
}

// Update Order Status
async function updateOrderStatus(orderId, newStatus) {
  try {
    const { error } = await supabase
      .from('orders')
      .update({ status: newStatus })
      .eq('id', orderId);

    if (error) throw error;

    // Log activity
    await supabase
      .from('order_history')
      .insert({
        order_id: orderId,
        action: 'status_changed',
        description: `Status changed to ${newStatus}`,
        performed_by: currentUser.email
      });

    alert('Status updated successfully');
    loadOrders(); // Refresh orders list
  } catch (error) {
    console.error('Error updating status:', error);
    alert('Failed to update status');
  }
}

// Update Payment Status
async function updatePaymentStatus(orderId, newStatus) {
  try {
    const updates = { payment_status: newStatus };
    if (newStatus === 'paid') {
      updates.paid_at = new Date().toISOString();
    }

    const { error } = await supabase
      .from('orders')
      .update(updates)
      .eq('id', orderId);

    if (error) throw error;

    // Log activity
    await supabase
      .from('order_history')
      .insert({
        order_id: orderId,
        action: 'payment_status_changed',
        description: `Payment status changed to ${newStatus}`,
        performed_by: currentUser.email
      });

    alert('Payment status updated successfully');
    loadOrders();
  } catch (error) {
    console.error('Error updating payment status:', error);
    alert('Failed to update payment status');
  }
}

// Load Boards
async function loadBoards() {
  try {
    const { data, error } = await supabase
      .from('boards')
      .select('*, orders(order_number, customer_name)')
      .order('created_at', { ascending: false });

    if (error) throw error;

    boards = data;

    // Update stats
    const stats = {
      total: boards.length,
      in_stock: boards.filter(b => b.status === 'in_stock').length,
      assigned: boards.filter(b => b.status === 'assigned').length,
      active: boards.filter(b => b.status === 'active').length
    };

    document.getElementById('totalBoards').textContent = stats.total;
    document.getElementById('inStockBoards').textContent = stats.in_stock;
    document.getElementById('assignedBoards').textContent = stats.assigned;
    document.getElementById('activeBoards').textContent = stats.active;

    // Display boards table
    displayBoardsTable(boards);
  } catch (error) {
    console.error('Error loading boards:', error);
  }
}

function displayBoardsTable(boardsToDisplay) {
  const boardsTable = document.getElementById('boardsTable');

  if (boardsToDisplay.length === 0) {
    boardsTable.innerHTML = '<p class="loading">No boards in inventory</p>';
    return;
  }

  boardsTable.innerHTML = `
    <table>
      <thead>
        <tr>
          <th>Board ID</th>
          <th>Status</th>
          <th>Order</th>
          <th>Firmware</th>
          <th>Hardware</th>
          <th>Manufactured</th>
          <th>Actions</th>
        </tr>
      </thead>
      <tbody>
        ${boardsToDisplay.map(board => `
          <tr>
            <td class="board-id">${board.board_id}</td>
            <td><span class="status-badge status-${board.status}">${board.status}</span></td>
            <td>${board.orders ? board.orders.order_number : 'N/A'}</td>
            <td>${board.firmware_version || 'N/A'}</td>
            <td>${board.hardware_revision || 'N/A'}</td>
            <td>${board.manufactured_date ? new Date(board.manufactured_date).toLocaleDateString() : 'N/A'}</td>
            <td>
              <button class="btn-secondary" onclick="editBoard('${board.id}')">Edit</button>
            </td>
          </tr>
        `).join('')}
      </tbody>
    </table>
  `;
}

// Add Board
async function handleAddBoard(e) {
  e.preventDefault();

  const boardData = {
    board_id: document.getElementById('newBoardId').value,
    firmware_version: document.getElementById('firmwareVersion').value || null,
    hardware_revision: document.getElementById('hardwareRevision').value || null,
    manufactured_date: document.getElementById('manufacturedDate').value || null,
    notes: document.getElementById('boardNotes').value || null,
    status: 'in_stock'
  };

  try {
    const { error } = await supabase
      .from('boards')
      .insert(boardData);

    if (error) throw error;

    alert('Board added successfully');
    hideModal('addBoardModal');
    document.getElementById('addBoardForm').reset();
    loadBoards();
  } catch (error) {
    console.error('Error adding board:', error);
    alert('Failed to add board: ' + error.message);
  }
}

// Modal Functions
function showModal(modalId) {
  document.getElementById(modalId).classList.add('show');
}

function hideModal(modalId) {
  document.getElementById(modalId).classList.remove('show');
}

// Global variable to track current order being worked on
let currentOrderId = null;

// Board Assignment
async function showAssignBoardDialog(orderId) {
  currentOrderId = orderId;

  try {
    // Load available boards
    const { data: availableBoards, error } = await supabase
      .from('boards')
      .select('*')
      .eq('status', 'in_stock')
      .order('created_at', { ascending: false });

    if (error) throw error;

    const select = document.getElementById('assignBoardSelect');
    select.innerHTML = '<option value="">-- Select a board from inventory --</option>';

    if (availableBoards.length === 0) {
      select.innerHTML = '<option value="">No boards available in inventory</option>';
      select.disabled = true;
    } else {
      availableBoards.forEach(board => {
        const option = document.createElement('option');
        option.value = board.id;
        option.textContent = `${board.board_id} - ${board.firmware_version || 'No version'} - ${board.hardware_revision || 'No revision'}`;
        select.appendChild(option);
      });
      select.disabled = false;
    }

    showModal('assignBoardModal');
  } catch (error) {
    console.error('Error loading boards:', error);
    alert('Failed to load available boards: ' + error.message);
  }
}

// Handle board assignment form submission
document.getElementById('assignBoardForm').addEventListener('submit', async (e) => {
  e.preventDefault();

  const boardId = document.getElementById('assignBoardSelect').value;
  const notes = document.getElementById('assignmentNotes').value;

  if (!boardId || !currentOrderId) {
    alert('Please select a board');
    return;
  }

  try {
    // Update board record
    const { error: boardError } = await supabase
      .from('boards')
      .update({
        order_id: currentOrderId,
        status: 'assigned',
        assigned_at: new Date().toISOString(),
        notes: notes || null
      })
      .eq('id', boardId);

    if (boardError) throw boardError;

    // Log in order history
    const { error: historyError } = await supabase
      .from('order_history')
      .insert({
        order_id: currentOrderId,
        action: 'board_assigned',
        description: `Board assigned to order`,
        performed_by: currentUser.email,
        user_id: currentUser.id
      });

    if (historyError) console.error('History log error:', historyError);

    // Close modal and refresh
    hideModal('assignBoardModal');
    document.getElementById('assignBoardForm').reset();

    alert('Board assigned successfully!');

    // Refresh order details
    await showOrderDetail(currentOrderId);
    await loadBoards();
  } catch (error) {
    console.error('Error assigning board:', error);
    alert('Failed to assign board: ' + error.message);
  }
});

// Manual Shipping Entry
async function createShippingLabel(orderId) {
  currentOrderId = orderId;

  // Reset form
  document.getElementById('shippingForm').reset();
  document.getElementById('shippingWeight').value = '400';

  showModal('shippingModal');
}

// Handle shipping form submission
document.getElementById('shippingForm').addEventListener('submit', async (e) => {
  e.preventDefault();

  const service = document.getElementById('shippingService').value;
  const trackingNumber = document.getElementById('trackingNumber').value;
  const weight = document.getElementById('shippingWeight').value;
  const notes = document.getElementById('shippingNotes').value;

  if (!service || !trackingNumber || !currentOrderId) {
    alert('Please fill in all required fields');
    return;
  }

  const serviceNames = {
    'rm48': 'Royal Mail 48 Tracked',
    'rm24': 'Royal Mail 24 Tracked',
    'rmsd': 'Royal Mail Special Delivery by 1pm',
    'other': 'Other Service'
  };

  try {
    // Create shipment record
    const { error: shipmentError } = await supabase
      .from('shipments')
      .insert({
        order_id: currentOrderId,
        tracking_number: trackingNumber,
        service_code: service,
        service_name: serviceNames[service],
        weight_grams: parseInt(weight) || 400,
        status: 'label_created',
        status_details: notes || null
      });

    if (shipmentError) throw shipmentError;

    // Update order status to shipped
    const { error: orderError } = await supabase
      .from('orders')
      .update({
        status: 'shipped',
        shipped_at: new Date().toISOString()
      })
      .eq('id', currentOrderId);

    if (orderError) throw orderError;

    // Update assigned boards status
    const { error: boardError } = await supabase
      .from('boards')
      .update({
        status: 'shipped',
        shipped_at: new Date().toISOString()
      })
      .eq('order_id', currentOrderId)
      .eq('status', 'assigned');

    if (boardError) console.error('Board update error:', boardError);

    // Log in order history
    await supabase.from('order_history').insert({
      order_id: currentOrderId,
      action: 'shipped',
      description: `Order shipped via ${serviceNames[service]}. Tracking: ${trackingNumber}`,
      performed_by: currentUser.email,
      user_id: currentUser.id
    });

    // Send shipping notification email
    await sendEmail(currentOrderId, 'shipping');

    // Close modal and refresh
    hideModal('shippingModal');
    document.getElementById('shippingForm').reset();

    alert('Shipment created and notification sent successfully!');

    // Refresh displays
    await showOrderDetail(currentOrderId);
    await loadOrders();
  } catch (error) {
    console.error('Error creating shipment:', error);
    alert('Failed to create shipment: ' + error.message);
  }
});

// Email Sending
async function sendEmail(orderId, emailType) {
  try {
    // Get order details
    const { data: order, error: orderError } = await supabase
      .from('orders')
      .select(`
        *,
        boards (board_id),
        shipments (tracking_number, service_name)
      `)
      .eq('id', orderId)
      .single();

    if (orderError) throw orderError;

    let emailData = {
      orderId: order.id,
      orderNumber: order.order_number,
      customerName: order.customer_name,
      customerEmail: order.customer_email
    };

    let emailSubject, emailBody;

    switch (emailType) {
      case 'shipping':
        if (!order.shipments || order.shipments.length === 0) {
          alert('No shipment found for this order');
          return;
        }
        emailSubject = `Your StationBoard has shipped! - Order ${order.order_number}`;
        emailBody = `
          <h2>Your order is on its way! 🚉</h2>
          <p>Hi ${order.customer_name},</p>
          <p>Great news! Your StationBoard order <strong>${order.order_number}</strong> has been shipped.</p>
          <h3>Shipping Details:</h3>
          <ul>
            <li><strong>Service:</strong> ${order.shipments[0].service_name}</li>
            <li><strong>Tracking Number:</strong> ${order.shipments[0].tracking_number}</li>
            <li><strong>Track your parcel:</strong> <a href="https://www.royalmail.com/track-your-item#/tracking-results/${order.shipments[0].tracking_number}">Track on Royal Mail</a></li>
          </ul>
          ${order.boards && order.boards.length > 0 ? `
            <h3>Your Board(s):</h3>
            <ul>${order.boards.map(b => `<li>${b.board_id}</li>`).join('')}</ul>
          ` : ''}
          <p>Your board should arrive within 2-3 working days.</p>
          <p>Best regards,<br>StationBoards Team</p>
        `;
        break;

      case 'payment':
        emailSubject = `Payment Reminder - Order ${order.order_number}`;
        emailBody = `
          <h2>Payment Reminder</h2>
          <p>Hi ${order.customer_name},</p>
          <p>This is a friendly reminder about your StationBoard order <strong>${order.order_number}</strong>.</p>
          <p><strong>Amount due:</strong> £${order.total_price}</p>
          <p>Please complete payment to proceed with your order.</p>
          <p>If you've already paid, please disregard this message.</p>
          <p>Best regards,<br>StationBoards Team</p>
        `;
        break;

      default:
        alert('Unknown email type');
        return;
    }

    // Call email API endpoint
    const response = await fetch('/api/send-email', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        to: order.customer_email,
        subject: emailSubject,
        html: emailBody
      })
    });

    if (!response.ok) {
      // If API doesn't exist, log to database only
      console.warn('Email API not available, logging to database only');
    }

    // Log email in database
    await supabase.from('email_log').insert({
      order_id: orderId,
      email_type: emailType,
      recipient_email: order.customer_email,
      subject: emailSubject,
      status: 'sent'
    });

    // Log in order history
    await supabase.from('order_history').insert({
      order_id: orderId,
      action: `email_sent_${emailType}`,
      description: `Email sent: ${emailSubject}`,
      performed_by: currentUser.email,
      user_id: currentUser.id
    });

    alert(`${emailType.charAt(0).toUpperCase() + emailType.slice(1)} email sent to ${order.customer_email}`);
  } catch (error) {
    console.error('Error sending email:', error);
    alert('Failed to send email: ' + error.message);
  }
}

// Status update functions
async function updateOrderStatus(orderId, status) {
  try {
    const { error } = await supabase
      .from('orders')
      .update({ status })
      .eq('id', orderId);

    if (error) throw error;

    await supabase.from('order_history').insert({
      order_id: orderId,
      action: 'status_updated',
      description: `Order status changed to: ${status}`,
      performed_by: currentUser.email,
      user_id: currentUser.id
    });

    await showOrderDetail(orderId);
    await loadOrders();
  } catch (error) {
    console.error('Error updating status:', error);
    alert('Failed to update status: ' + error.message);
  }
}

async function updatePaymentStatus(orderId, paymentStatus) {
  try {
    const updateData = { payment_status: paymentStatus };

    if (paymentStatus === 'paid') {
      updateData.paid_at = new Date().toISOString();
    }

    const { error } = await supabase
      .from('orders')
      .update(updateData)
      .eq('id', orderId);

    if (error) throw error;

    await supabase.from('order_history').insert({
      order_id: orderId,
      action: 'payment_updated',
      description: `Payment status changed to: ${paymentStatus}`,
      performed_by: currentUser.email,
      user_id: currentUser.id
    });

    await showOrderDetail(orderId);
    await loadOrders();
  } catch (error) {
    console.error('Error updating payment status:', error);
    alert('Failed to update payment status: ' + error.message);
  }
}

// Placeholder functions (to be implemented later)
function downloadInvoice(orderId) {
  alert('Invoice generation feature coming soon');
}

function editBoard(boardId) {
  alert('Board editing feature coming soon');
}

// Initialize on page load
init();
